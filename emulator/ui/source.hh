#ifndef EMULATOR_SOURCE_HH
#define EMULATOR_SOURCE_HH


#include "memory.hh"
#include "../compiler/compiler.hh"

class Source : public Window {
public:
    Source();
    ~Source();
    void resize(int line, int col, int lines, int cols) override;

    std::string name() const override;

    void update() const override;
    void pc_updated();

    void move_cursor(int rel);

    void swap_breakpoint() const;

    int choose_file();

private:
    void print_source_line(int line_number, std::string const &line_str, std::optional<uint16_t> const& addr) const;
    void format_source_line(int line_number, std::optional<uint16_t> const& addr) const;

    WINDOW* subwindow_;
    size_t scroll_ = 0;
    int cursor_line_ = 0;
    SourceLocation source_location_ {};
};

#endif //EMULATOR_SOURCE_HH
