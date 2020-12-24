#include "source.hh"
#include "../compiler/compiler.hh"

Source::Source()
{
    subwindow_ = derwin(window_, 1, 1, 0, 0);
}

Source::~Source()
{
    delwin(window_);
}

void Source::resize(int line, int col, int lines, int cols)
{
    Window::resize(line, col, lines, cols);
    wresize(subwindow_, this->lines_ - 1, this->cols_ - 1);
}

void Source::update() const
{
    wbkgd(subwindow_, background_color_);

    SourceLocation source_location;
    try {
        source_location = compiled_code.locations.at(hardware->PC());
    } catch (std::out_of_range& e) {
        mvwprintw(subwindow_, 1, 1, "PC %04X does not point to any location in source code.");
    }

    if (scroll_ + this->lines_ - 2 < source_location.line)
        scroll_ = std::max(source_location.line - this->lines_ + 8, (size_t) 0);

    for (int i = 0; i < lines_; ++i) {
        std::string line;
        try {
            line = compiled_code.source.at(source_location.file).at(i + scroll_);
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
        if (source_location.line == i + scroll_ + 1) {
            mvwchgat(subwindow_, i + 1, 1, this->cols_ - 1, 0, 2, nullptr);
        }
    }
    wrefresh(subwindow_);
}

