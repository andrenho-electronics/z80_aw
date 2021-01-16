#include "codeview.hh"
#include "z80pres.hh"

#include "z80aw.hh"

void CodeView::set_debug_information(z80aw::DebugInformation const& di)
{
    di_ = &di;
}

void CodeView::update(bool update_file_selected)
{
    lines_.clear();
    
    // find PC location, and return empty if PC does not appear on code
    SourceLocation sl;
    if (update_file_selected) {
        std::optional<SourceLocation> osl = di_->location(z80_state_.pc);
        if (!osl.has_value()) {
            file_selected_ = {};
            return;
        }
        sl = osl.value();
        file_selected_ = di_->filenames().at(sl.file);
    } else {
        if (!file_selected_.has_value())
            return;
        sl = SourceLocation { find_file_idx(file_selected_.value()), 1 };
    }
    
    // retrieve breakpoints
    std::vector<uint16_t> bkps = z80aw::query_breakpoints();
    
    // create lines
    size_t i = 1;
    while (true) {
        SourceLocation sl_line { sl.file, i };
        auto oline = di_->sourceline(sl_line);
        if (!oline.has_value())
            break;
        auto oaddr = di_->rlocation(sl_line);
        bool is_breakpoint = oaddr.has_value() ? (std::find(bkps.begin(), bkps.end(), oaddr.value())) != bkps.end() : false;
        lines_.emplace_back(oline.value(), oaddr, oaddr.value_or(-1) == z80_state_.pc, is_breakpoint, di_->bytes(sl_line));
        ++i;
    }
}

ssize_t CodeView::find_file_idx(std::string const& filename) const
{
    auto it = std::find(di_->filenames().begin(), di_->filenames().end(), filename);
    if (it == di_->filenames().end())
        return -1;
    return std::distance(di_->filenames().begin(), it);
}

void CodeView::set_file(std::string const& filename)
{
    file_selected_ = filename;
    update(false);
}

std::vector<std::string> CodeView::files(Order order) const
{
    std::vector<std::string> f = di_->filenames();
    if (order == Order::Alphabetical)
        std::sort(f.begin(), f.end());
    return f;
}

std::vector<Symbol> CodeView::symbols(Order order) const
{
    std::vector<Symbol> ss;
    auto syms = di_->symbols();
    std::transform(syms.begin(), syms.end(), std::back_inserter(ss), [](auto const& s) -> Symbol { return { s.symbol, s.addr }; });
    if (order == Order::Alphabetical)
        std::sort(ss.begin(), ss.end(), [](Symbol const& a, Symbol const& b) { return a.symbol < b.symbol; });
    return ss;
}

std::optional<size_t> CodeView::goto_symbol(std::string const& symbol)
{
    // find symbol
    auto syms = symbols(Order::Source);
    auto it = std::find_if(syms.begin(), syms.end(), [&symbol](Symbol const& s) { return s.symbol == symbol; });
    if (it == syms.end())
        return {};
    
    // find source location
    auto osl = di_->location(it->addr);
    if (!osl.has_value())
        return {};
    
    set_file(di_->filenames().at(osl.value().file));
    return osl.value().file;
}

void CodeView::add_breakpoint(size_t line)
{
    if (!file_selected_.has_value())
        return;
    auto oaddr = di_->rlocation({ find_file_idx(file_selected_.value()), line });
    if (oaddr.has_value()) {
        z80aw::add_breakpoint(oaddr.value());
        update(false);
    }
}

void CodeView::remove_breakpoint(size_t line)
{
    if (!file_selected_.has_value())
        return;
    auto oaddr = di_->rlocation({ find_file_idx(file_selected_.value()), line });
    if (oaddr.has_value()) {
        z80aw::remove_breakpoint(oaddr.value());
        update(false);
    }
}
