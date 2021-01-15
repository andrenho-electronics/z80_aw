#include "codeview.hh"
#include "z80pres.hh"

void CodeView::set_debug_information(z80aw::DebugInformation const& di)
{
    di_ = &di;
}

void CodeView::update()
{
    lines_.clear();
    
    // find PC location, and return empty if PC does not appear on code
    std::optional<SourceLocation> osl = di_->location(z80_state_.pc);
    if (!osl.has_value()) {
        file_selected_ = {};
        return;
    }
    
    // find selected file
    SourceLocation sl = osl.value();
    file_selected_ = di_->filenames().at(sl.file);
    
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
        lines_.emplace_back(oline.value(), oaddr, oaddr.value_or(-1) == z80_state_.pc, is_breakpoint);
        ++i;
    }
    
    // load memory
}
