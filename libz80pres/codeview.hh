#ifndef LIBZ80PRES_CODEVIEW_HH
#define LIBZ80PRES_CODEVIEW_HH

#include <optional>
#include <string>
#include <vector>

namespace z80aw { class DebugInformation; }

struct CodeViewLine {
    std::string             code;
    std::vector<uint8_t>    bytes;
    std::optional<uint16_t> address;
    bool                    is_breakpoint;
    bool                    is_pc;
};

class CodeView {
public:
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
