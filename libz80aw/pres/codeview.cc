#include "codeview.hh"
#include "z80pres.hh"

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
        auto it = std::find(di_->filenames().begin(), di_->filenames().end(), file_selected_.value());
        if (it == di_->filenames().end())
            throw std::runtime_error("Bug: filename not found for location");
        sl = SourceLocation { std::distance(di_->filenames().begin(), it), 1 };
    }
    
    // create lines
    size_t i = 1;
    while (true) {
        SourceLocation sl_line { sl.file, i };
        auto oline = di_->sourceline(sl_line);
        if (!oline.has_value())
            break;
        auto oaddr = di_->rlocation(sl_line);
        // TODO - check breakpoints
        bool is_breakpoint = false;
        lines_.emplace_back(oline.value(), oaddr, oaddr.value_or(-1) == z80_state_.pc, is_breakpoint, di_->bytes(sl_line));
        ++i;
    }
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
