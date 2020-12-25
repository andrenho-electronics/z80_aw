#include "source.hh"
#include "../compiler/compiler.hh"

Source::Source()
{
    subwindow_ = newwin(1, 1, 0, 0);
}

Source::~Source()
{
    delwin(subwindow_);
}

void Source::resize(int line, int col, int lines, int cols)
{
    Window::resize(line, col, lines, cols);
    wresize(subwindow_, this->lines_ - 2, this->cols_ - 2);
    mvwin(subwindow_, 1, 1);
}

void Source::update() const
{
    wbkgd(subwindow_, background_color_);
    werase(subwindow_);

    for (int i = 0; i < lines_; ++i) {
        std::string line;
        try {
            line = compiled_code.source.at(source_location_.file).at(i + scroll_);
        } catch (std::out_of_range&) {
            break;  // TODO - clear panel
        }
        int col = 0;
        attron(COLOR_MEMORY);
        for (int j = 0; j < this->cols_ - 2; ++j) {
            if (j < line.length()) {
                if (line[j] == '\t') {
                    int next = ((col / 8) + 1) * 8;
                    while (col < next)
                        mvwaddch(subwindow_, i, col++, ' ');
                } else {
                    mvwaddch(subwindow_, i, col, line[j]);
                    ++col;
                }
            }
        }
        if (source_location_.line == i + scroll_ + 1) {
            mvwchgat(subwindow_, i, 0, this->cols_ - 1, 0, 2, nullptr);
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
    } else if (cursor_line_ + rel >= lines_ - 2) {
        if (scroll_ < lines_ - 2)
            ++scroll_;
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

void Source::choose_file()
{
    int line = 4;
    int col = COLS / 2 - 15;
    int lines = LINES - 4;
    int cols = 30;
    WINDOW* wchoose = newwin(line, col, lines, cols);
    box(wchoose, 0, 0);
    std::string name = "Choose source file";
    mvwprintw(window_, 0, cols_ / 2 - name.length() / 2, name.c_str());
    wbkgd(window_, COLOR_DIALOG);

    wrefresh(window_);
    int ch = getch();

    delwin(wchoose);
}

