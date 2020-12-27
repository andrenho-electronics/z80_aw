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

    for (int line_number = 0; line_number < lines_; ++line_number) {
        std::string line;
        try {
            std::optional<uint16_t> addr;
            line = compiled_code.source.at(source_location_.file).at(line_number + scroll_);
            auto const& iaddr = compiled_code.rlocations.find({source_location_.file, line_number + scroll_ + 1 });
            if (iaddr != compiled_code.rlocations.end())
                addr = iaddr->second;
            print_source_line(line_number, line, addr);
            format_source_line(line_number, addr);
        } catch (std::out_of_range&) {
            break;
        }
    }

    move(cursor_line_ + 1, 1);
    wrefresh(subwindow_);
}

void Source::print_source_line(int line_number, std::string const &line_str, std::optional<uint16_t> const& addr) const
{
    int col = 6;
    for (int j = 0; j < cols_ - 2; ++j) {
        if (addr) {
            wattr_on(subwindow_, COLOR_ADDRESS, nullptr);
            mvwprintw(subwindow_, line_number, 0, "%04X: ", addr.value());
            wattr_off(subwindow_, COLOR_ADDRESS, nullptr);
        }
        if (j < line_str.length()) {
            if (line_str[j] == '\t') {
                int next = (((col - 6) / 8) + 1) * 8 + 6;
                while (col < next)
                    mvwaddch(subwindow_, line_number, col++, ' ');
            } else {
                mvwaddch(subwindow_, line_number, col, line_str[j]);
                ++col;
            }
        }
    }
}

void Source::format_source_line(int line_number, std::optional<uint16_t> const& addr) const
{
    if (source_location_.line == line_number + scroll_ + 1) {  // line == PC
        mvwchgat(subwindow_, line_number, 6, cols_ - 1, 0, 2, nullptr);
        return;
    } else {   // is breakpoint
        if (addr) {
            if (hardware->is_breakpoint(addr.value())) {
                mvwchgat(subwindow_, line_number, 6, cols_ - 1, 0, 7, nullptr);
                return;
            }
        }
    }

    // mvwchgat(subwindow_, line_number, 0, cols_ - 1, 0, 4, nullptr);
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
    size_t previous_file = source_location_.file;
    try {
        source_location_ = compiled_code.locations.at(hardware->PC());
        if (source_location_.file != previous_file) {
            scroll_ = 0;
            redraw();
        }
    } catch (std::out_of_range& e) {
        mvwprintw(subwindow_, 1, 1, "PC %04X does not point to any location in source code.");
        return;
    }

    if (scroll_ + this->lines_ - 2 < source_location_.line)
        scroll_ = std::max(source_location_.line - this->lines_ + 8, (size_t) 0);
}

int Source::choose_file()
{
    int h = compiled_code.filename.size() + 2;

    int line = LINES / 2 - h / 2;
    int col = COLS / 2 - 15;
    int lines = h;
    int cols = 30;
    WINDOW* wchoose = newwin(lines, cols, line, col);
    std::string name = "Choose source file";
    wbkgd(wchoose, COLOR_DIALOG);

    int selected = 0;

    int ch;
    do {
        werase(wchoose);

        box(wchoose, 0, 0);
        mvwprintw(wchoose, 0, cols / 2 - name.length() / 2, name.c_str());

        int i = 0;
        for (auto const& filename: compiled_code.filename) {
            mvwprintw(wchoose, i + 1, 1, "%s", filename.c_str());
            if (selected == i)
                mvwchgat(wchoose, i + 1, 1, cols - 2, 0, 0, nullptr);
            ++i;
        }
        wrefresh(wchoose);

        ch = getch();
        if (ch == KEY_UP || ch == 60419) {
            if (selected > 0)
                --selected;
        } else if (ch == KEY_DOWN || ch == 60418) {
            if (selected < compiled_code.filename.size())
                ++selected;
        } else if (ch == 27) {
            delwin(wchoose);
            return -1;
        }
    } while (ch != KEY_ENTER && ch != 10);

    source_location_.file = selected;

    if (compiled_code.locations.at(hardware->PC()).file == selected) {
        int ln = compiled_code.locations.at(hardware->PC()).line;
        source_location_.line = ln;
        scroll_ = ln - 4;
    } else {
        source_location_.line = 0;
        scroll_ = 0;
    }
    redraw();
    update();

    delwin(wchoose);
    return selected;
}

void Source::swap_breakpoint() const
{
    for (auto const& [addr, sl]: compiled_code.locations) {
        if (sl.file == source_location_.file && sl.line == scroll_ + cursor_line_ + 1) {
            if (hardware->is_breakpoint(addr)) {
                hardware->remove_breakpoint(addr);
                update();
            } else {
                hardware->add_breakpoint(addr);
                update();
            }
            return;
        }
    }
}

std::string Source::name() const
{
    try {
        return "Source [" + compiled_code.filename.at(source_location_.file) + "]";
    } catch (std::out_of_range&) {
        return "Source";
    }
}

void Source::set_running() const
{
    werase(subwindow_);
    mvwprintw(subwindow_, 0, 0, "Running...");
    wrefresh(subwindow_);
}

void Source::reset()
{
    scroll_ = 0;
    cursor_line_ = 0;
    source_location_ = {};
    pc_updated();
    update();
}

