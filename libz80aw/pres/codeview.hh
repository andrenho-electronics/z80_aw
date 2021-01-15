#ifndef LIBZ80PRES_CODEVIEW_HH
#define LIBZ80PRES_CODEVIEW_HH

#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "view.hh"

namespace z80aw { class DebugInformation; }

struct CodeViewLine {
    CodeViewLine(std::string const& code, std::optional<uint16_t> const& address, bool is_pc, bool is_breakpoint, std::vector<uint8_t> const& bytes)
        : code(code), address(address), is_pc(is_pc), is_breakpoint(is_breakpoint), bytes(bytes) {}
    
    std::string             code;
    std::optional<uint16_t> address;
    bool                    is_pc;
    bool                    is_breakpoint;
    std::vector<uint8_t>    bytes;
};

class CodeView : public View {
public:
    explicit CodeView(Z80State z80_state) : View(z80_state) {}
    
    void update();
    void set_debug_information(z80aw::DebugInformation const& di);
    
    std::vector<CodeViewLine> const& lines() const { return lines_; }
    std::optional<std::string> const& file_selected() const { return file_selected_; }

private:
    z80aw::DebugInformation const* di_ = nullptr;
    std::optional<std::string>     file_selected_;
    std::vector<CodeViewLine>      lines_;
};

#endif
