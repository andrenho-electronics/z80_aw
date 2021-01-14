#include "codeview.hh"

void CodeView::update()
{
    lines_.clear();
}

void CodeView::set_debug_information(z80aw::DebugInformation const& di)
{
    di_ = &di;
    update();
}
