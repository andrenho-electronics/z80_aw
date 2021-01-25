#ifndef DEBUGGER_MYUI_HH
#define DEBUGGER_MYUI_HH

#include <chrono>
#include <optional>

#include "ui.hh"
#include "options.hh"
#include "../libz80aw/pres/z80pres.hh"
#include "config.hh"

struct ErrorMessage {
    std::string title;
    std::string message;
};

class MyUI : public UI {
public:
    explicit MyUI(Window const& window, Options const& options);
    
    void draw() override;
    bool stopped() const;
    
private:
    struct SymbolItem {
        std::string symbol;
        std::string file_line;
    };
    
    std::optional<Z80Presentation> presentation {};
    
    bool show_demo_window;
    bool show_advanced_window = false;
    bool show_choose_file = false;
    bool show_choose_symbol = false;
    bool show_keypress_modal = false;
    std::vector<std::string> file_list;
    std::vector<SymbolItem> symbol_list;
    bool scroll_to_pc = true;
    bool blink = true;
    std::chrono::time_point<std::chrono::system_clock> last_blink = std::chrono::system_clock::now();
    std::optional<size_t> show_this_line_on_next_frame {};
    
    // start window
    Config config;
    std::optional<ErrorMessage> error_message {};
    
    Z80Presentation& p() { return presentation.value(); }
    Z80Presentation const& p() const { return presentation.value(); }
    
    // draw
    void draw_start();
    void draw_code();
    void draw_memory();
    void draw_memory_table(MemoryView& m) const;
    void draw_cpu();
    void draw_terminal();
    void draw_keypress_modal();
    void draw_advanced();
    void draw_code_view();
    void draw_choose_file();
    void draw_choose_symbol();
    
    // errors
    void error(std::string const& title, std::string const& message);
    void draw_error_modal();
    
    // actions
    void start_execution();
    void update_symbol_list();
    void upload_binary();
    bool do_keypress();
   
    // translations
    std::vector<uint8_t> translate_keypress(int key, bool ctrl, bool shift, bool alt, bool super) const;
    
    void reset_project();
};

#endif
