#include "source.hh"
#include "../compiler/compiler.hh"

Source::Source()
{
    subwindow_ = derwin(window_, 2, 2, 0, 0);
}

Source::~Source()
{
    delwin(subwindow_);
}

void Source::resize(int line, int col, int lines, int cols)
{
    Window::resize(line, col, lines, cols);
    wresize(subwindow_, this->lines_ - 1, this->cols_ - 1);
}

void Source::update() const
{
    wbkgd(subwindow_, background_color_);

    for (int i = 0; i < lines_; ++i) {
        std::string line;
        try {
            line = compiled_code.source.at(source_location_.file).at(i + scroll_);
        } catch (std::out_of_range&) {
            break;  // TODO - clear panel
        }
        int col = 1;
        for (int j = 0; j < this->cols_ - 2; ++j) {
            if (j < line.length()) {
                if (line[j] == '\t') {
                    int next = (((col - 1) / 8) + 1) * 8;
                    while (col <= next)
                        mvwaddch(subwindow_, i+1, col++, ' ');
                } else {
                    mvwaddch(subwindow_, i + 1, col, line[j]);
                    ++col;
                }
            }
        }
        if (source_location_.line == i + scroll_ + 1) {
            mvwchgat(subwindow_, i + 1, 1, this->cols_ - 1, 0, 2, nullptr);
        }
    }

    move(cursor_line_ + 1, 1);

    wrefresh(subwindow_);
}

void Source::move_cursor(int rel)
{
    if (cursor_line_ + rel < 0) {
        if (scroll_ > 0) {
            --scroll_;
        }
    } else {
        cursor_line_ += rel;
    }
    update();
}

void Source::pc_updated()
{
    try {
        source_location_ = compiled_code.locations.at(hardware->PC());
    } catch (std::out_of_range& e) {
        mvwprintw(subwindow_, 1, 1, "PC %04X does not point to any location in source code.");
        return;
    }

    if (scroll_ + this->lines_ - 2 < source_location_.line)
        scroll_ = std::max(source_location_.line - this->lines_ + 8, (size_t) 0);
}

