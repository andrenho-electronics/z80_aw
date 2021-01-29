#include "myui.hh"

#include <iostream>
#include <optional>
using namespace std::chrono_literals;

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

static const int PageUp = 0x10a;
static const int PageDown = 0x10b;
static const int F2 = 0x123;
static const int F7 = 0x128;
static const int F8 = 0x129;
static const int F9 = 0x12a;
static const int F12 = 0x12d;

MyUI::MyUI(Window const& window, Options const& options)
    : UI(window), show_demo_window(options.show_demo_window()), config(this->context)
{
    // execution will not start until "MyUI::start_exectution" is run
}

//
// INFORMATION
//

bool MyUI::stopped() const
{
    return !presentation.has_value() ||  p().mode() == Z80State::Stopped;
}

//
// DRAW
//

void MyUI::draw()
{
    if (presentation.has_value() && !stopped()) {
        p().check_events();
        do_keypress();
    }
    
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    if (!presentation.has_value()) {
        draw_start();
    } else {
        if (show_advanced_window)
            draw_advanced();
        if (show_choose_file)
            draw_choose_file();
        if (show_choose_symbol)
            draw_choose_symbol();
        draw_memory();
        draw_cpu();
        draw_terminal();
        draw_code();
        if (show_keypress_modal)
            draw_keypress_modal();
    }
    if (error_message.has_value())
        draw_error_modal();
    
    if (std::chrono::system_clock::now() > last_blink + 500ms) {
        blink = !blink;
        last_blink = std::chrono::system_clock::now();
    }
}

void MyUI::draw_start()
{
    ImGui::Begin("Welcome to Z80AW debugger", nullptr);
    
    ImGui::Text("Select project file"); ImGui::SameLine();
    ImGui::InputText("##b", config.project_file, sizeof config.project_file);
    
    ImGui::Separator();
    
    ImGui::Text("Execution type");
    ImGui::SameLine();
    if (ImGui::RadioButton("Emulator", config.emulator_mode))
        config.emulator_mode = true;
    ImGui::SameLine();
    if (ImGui::RadioButton("Real hardware", !config.emulator_mode))
        config.emulator_mode = false;
    
    if (!config.emulator_mode) {
        ImGui::Text("Serial port"); ImGui::SameLine();
        ImGui::InputText("##a", config.serial_port, sizeof config.serial_port);
    } else {
        ImGui::Text("Emulator Path"); ImGui::SameLine();
        ImGui::InputText("##e", config.emulator_path, sizeof config.serial_port);
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("Start execution (F12)") || ImGui::IsKeyPressed(F12, false)) {
        start_execution();
    }
    
    ImGui::End();
}

void MyUI::draw_code()
{
    bool window_open = true;
    ImGui::Begin("Code debugger", &window_open);
    if (!window_open) {
        reset_project();
        ImGui::End();
        return;
    }
    
    if (p().mode() == Z80State::Stopped) {
        if (ImGui::Button("Step (F7)") || ImGui::IsKeyPressed(F7, false)) {
            try {
                p().step();
                scroll_to_pc = true;
            } catch (std::runtime_error& e) {
                error("Error during step", e.what());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Next (F8)") || ImGui::IsKeyPressed(F8, false)) {
            try {
                p().next();
                scroll_to_pc = true;
            } catch (std::runtime_error& e) {
                error("Error during next step", e.what());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Run (F9)") || ImGui::IsKeyPressed(F9, false)) {
            try { p().continue_(); } catch (std::runtime_error& e) { error("Error during continue", e.what()); }
        }
    } else {
        if (ImGui::Button("Stop (Ctrl+C)") || (io.KeyCtrl && ImGui::IsKeyPressed('c', false))) {
            try {
                p().stop();
                scroll_to_pc = true;
            } catch (std::runtime_error& e) {
                error("Error stopping execution", e.what());
            }
        }
    }
    
    draw_code_view();
    
    if (stopped()) {
        ImGui::Text("Click on the address to set a breakpoint.");
    
        if (ImGui::Button("Reset CPU")) {
            try {
                p().reset();
                scroll_to_pc = true;
            } catch (std::runtime_error& e) {
                error("Error resetting CPU", e.what());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Recompile project (Ctrl+R)") || (io.KeyCtrl && ImGui::IsKeyPressed('r', false))) {
            try {
                p().recompile_project();
                if (config.emulator_mode)
                    p().upload_compiled();
                update_symbol_list();
                p().reset();
                scroll_to_pc = true;
            } catch (std::runtime_error& e) {
                error("Error recompiling project", e.what());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Go to file... (F)") || ImGui::IsKeyPressed('f', false))
            show_choose_file = true;
        ImGui::SameLine();
        if (ImGui::Button("Go to symbol... (S)") || ImGui::IsKeyPressed('s', false))
            show_choose_symbol = true;
        ImGui::SameLine();
    if (ImGui::Button("Advanced..."))
        show_advanced_window = true;
    }
    
    ImGui::End();
}

void MyUI::reset_project()
{
    presentation.reset();
    show_advanced_window = false;
    show_choose_file = false;
    show_choose_symbol = false;
    show_keypress_modal = false;
}

void MyUI::draw_code_view()
{
    CodeView& c = p().codeview();
    
    // filename
    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
    if (stopped())
        ImGui::Button(c.file_selected().has_value() ? c.file_selected()->c_str() : "No file selected");
    else
        ImGui::Button("Code in execution...");
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    
    // table
    static int tbl_flags = ImGuiTableFlags_BordersOuterH
                           | ImGuiTableFlags_BordersOuterV
                           | ImGuiTableFlags_BordersInnerV
                           | ImGuiTableFlags_BordersOuter
                           | ImGuiTableFlags_RowBg
                           | ImGuiTableFlags_ScrollY
                           | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    static ImU32 pc_row_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f));
    static ImU32 bkp_cell_color = ImGui::GetColorU32(ImVec4(0.8f, 0.2f, 0.2f, 0.65f));
    
    ImVec2 size = ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y - 42);
    if (ImGui::BeginTable("##code", 3, tbl_flags, size)) {
    
        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Bytes", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Code", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
    
        if (stopped()) {
            size_t nline = 1;
            for (auto const& line: c.lines()) {
                ImGui::TableNextRow();
                
                if (line.address.has_value()) {
                    if (*line.address == p().pc()) {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, pc_row_color);
                        if (scroll_to_pc) {
                            ImGui::SetScrollHereY();
                            scroll_to_pc = false;
                        }
                    }
                    
                    if (line.is_breakpoint)
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bkp_cell_color, 0);
                    
                    if (nline == show_this_line_on_next_frame.value_or(-1)) {
                        ImGui::SetScrollHereY();
                        show_this_line_on_next_frame.reset();
                    }
        
                    ImGui::TableSetColumnIndex(0);
                    char buf[5];
                    sprintf(buf, "%04X", *line.address);
                    if (ImGui::Selectable(buf)) {
                        if (line.is_breakpoint)
                            c.remove_breakpoint(nline);
                        else
                            c.add_breakpoint(nline);
                    }
                }
                
                ImGui::TableSetColumnIndex(1);
                char buf[30] = { 0 };
                int pos = 0;
                for (auto b: line.bytes)
                    pos += sprintf(&buf[pos], "%02X ", b);
                if (!line.bytes.empty())
                    ImGui::Text("%s", buf);
                
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", line.code.c_str());
                
                ++nline;
            }
        }
        
        ImGui::EndTable();
    }
}

void MyUI::draw_memory()
{
    MemoryView& m = p().memoryview();
    
    float h = 370;
    if (!p().is_uploaded() && stopped())
        h += 24;
    ImGui::SetNextWindowSize(ImVec2(560, h));
    if (ImGui::Begin("Memory", nullptr, ImGuiWindowFlags_NoResize)) {
        uint8_t page = m.page_number();
    
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Page: (PgUp)");
        ImGui::SameLine();
        
        if (ImGui::Button("<") || ImGui::IsKeyPressed(PageUp))
            m.go_to_page(page - 1);
        ImGui::SameLine();
    
        char buf[3];
        snprintf(buf, 3, "%02X", page);
        ImGui::PushItemWidth(24.0);
        ImGui::InputText("##page", buf, sizeof buf, ImGuiInputTextFlags_CallbackEdit,
                         [](ImGuiInputTextCallbackData* data) {
                             auto* m = reinterpret_cast<MemoryView*>(data->UserData);
                             unsigned long new_page = strtoul(data->Buf, nullptr, 16);
                             if (new_page != ULONG_MAX)
                                 m->go_to_page(new_page);
                             return 0;
                         }, &m);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button(">") || ImGui::IsKeyPressed(PageDown))
            m.go_to_page(page + 1);
        ImGui::SameLine();
        ImGui::Text("(PgDown)");
    
        draw_memory_table(m);
        
        char stack[512] = "Stack: ";
        int n = strlen(stack);
        for (uint8_t byte: m.stack())
            n += sprintf(&stack[n], "%02X ", byte);
        ImGui::Text("%s", stack);
    
        if (!p().is_uploaded() && stopped()) {
            ImGui::AlignTextToFramePadding();
            if (blink)
                ImGui::Text("The compiled binary does not match with the memory!");
            else
                ImGui::Text("                                                   ");
            ImGui::SameLine();
            if (ImGui::Button("Upload to memory"))
                upload_binary();
        }
    }
    ImGui::End();
}

void MyUI::draw_memory_table(MemoryView& m) const
{
    static int tbl_flags = ImGuiTableFlags_BordersOuter
                         | ImGuiTableFlags_NoBordersInBody
                         | ImGuiTableFlags_RowBg
                         | ImGuiTableFlags_ScrollY;
    static ImU32 pc_bg_color = ImGui::GetColorU32(ImVec4(0.2f, 0.6f, 0.2f, 0.65f));
    static ImU32 sp_bg_color = ImGui::GetColorU32(ImVec4(0.6f, 0.2f, 0.2f, 0.65f));
    uint16_t page = ((uint16_t) m.page_number()) << 8;
    
    ImVec2 size = ImVec2(-FLT_MIN, 293);
    if (ImGui::BeginTable("##ram", 18, tbl_flags, size)) {
        
        // headers
        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
        for (int i = 0; i < 0x10; ++i) {
            char buf[3];
            sprintf(buf, "_%X", i);
            ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed);
        }
        ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        if (stopped()) {
            for (int line = 0; line < 0x10; ++line) {
                ImGui::TableNextRow();
                
                // address
                uint16_t addr = page + (line * 0x10);
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%04X : ", addr);
                
                // data
                std::string ascii;
                for (int i = 0; i < 0x10; ++i) {
                    ImGui::TableSetColumnIndex(i + 1);
                    uint8_t byte =  m.data().at((line * 0x10) + i);
                    bool needs_pop = false;
                    if (addr + i == p().pc()) {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, pc_bg_color);
                    } else if (p().registers().has_value() && addr + i == p().registers().value().SP) {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, sp_bg_color);
                    }
                    if (byte == 0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(128, 128, 128)));
                        needs_pop = true;
                    }
                    ImGui::Text("%02X", byte);
                    if (needs_pop)
                        ImGui::PopStyleColor();
                    ascii += (byte >= 32 && byte < 127) ? (char) byte : '.';
                }
                
                // ascii
                ImGui::TableSetColumnIndex(17);
                ImGui::Text("%s", ascii.c_str());
            }
        }
        
        ImGui::EndTable();
    }
}

void MyUI::draw_cpu()
{
    ImGui::SetNextWindowSize({ 265, 375 });
    if (ImGui::Begin("CPU", nullptr, ImGuiWindowFlags_NoResize)) {
        if (stopped()) {
            ImGui::Text("Registers:");
    
            auto pair = [](size_t& i, size_t sz, std::string const& name, int value) {
                ImGui::TableSetColumnIndex(i++);
                ImGui::Text("%s:", name.c_str());
                ImGui::TableSetColumnIndex(i++);
                char buf[5];
                if (value == -1)
                    sprintf(buf, "%s", sz == 2 ? "??" : "????");
                else if (sz == 2)
                    sprintf(buf, "%02X", (uint8_t) value);
                else if (sz == 4)
                    sprintf(buf, "%04X", (uint16_t) value);
                ImGui::Button(buf);
            };
    
            static int tbl_flags = ImGuiTableFlags_NoBordersInBody;
            if (ImGui::BeginTable("##cpu", 8, tbl_flags)) {
                for (int i = 0; i < 8; ++i)
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
                
                ImGui::TableNextRow();
                size_t i = 0;
                pair(i, 4, "AF", p().registers().has_value() ? p().registers().value().AF : -1);
                pair(i, 2, "A", p().registers().has_value() ? p().registers().value().AF >> 8: -1);
                pair(i, 2, "F", p().registers().has_value() ? p().registers().value().AF & 0xff: -1);
                pair(i, 4, "AF'", p().registers().has_value() ? p().registers().value().AFx : -1);
                ImGui::TableNextRow();
                i = 0;
                pair(i, 4, "BC", p().registers().has_value() ? p().registers().value().BC : -1);
                pair(i, 2, "B", p().registers().has_value() ? p().registers().value().BC >> 8: -1);
                pair(i, 2, "C", p().registers().has_value() ? p().registers().value().BC & 0xff: -1);
                pair(i, 4, "BC'", p().registers().has_value() ? p().registers().value().BCx : -1);
                ImGui::TableNextRow();
                i = 0;
                pair(i, 4, "DE", p().registers().has_value() ? p().registers().value().DE : -1);
                pair(i, 2, "D", p().registers().has_value() ? p().registers().value().DE >> 8: -1);
                pair(i, 2, "E", p().registers().has_value() ? p().registers().value().DE & 0xff: -1);
                pair(i, 4, "DE'", p().registers().has_value() ? p().registers().value().DEx : -1);
                ImGui::TableNextRow();
                i = 0;
                pair(i, 4, "HL", p().registers().has_value() ? p().registers().value().HL : -1);
                pair(i, 2, "H", p().registers().has_value() ? p().registers().value().HL >> 8: -1);
                pair(i, 2, "L", p().registers().has_value() ? p().registers().value().HL & 0xff: -1);
                pair(i, 4, "HL'", p().registers().has_value() ? p().registers().value().HLx : -1);
                ImGui::TableNextRow();
                i = 0;
                pair(i, 4, "IX", p().registers().has_value() ? p().registers().value().IX : -1);
                pair(i, 2, "I", p().registers().has_value() ? p().registers().value().I : -1);
                pair(i, 2, "R", p().registers().has_value() ? p().registers().value().R : -1);
                pair(i, 4, "IY", p().registers().has_value() ? p().registers().value().IY : -1);
                ImGui::TableNextRow();
                i = 0;
                pair(i, 4, "PC", p().pc());
                i = 6;
                pair(i, 4, "SP", p().registers().has_value() ? p().registers().value().SP : -1);
                ImGui::EndTable();
            }
            bool v = p().registers().has_value() && p().registers().value().HALT;
            ImGui::Checkbox("HALT", &v);
            
            if (p().registers().has_value()) {
                uint8_t f = p().registers().value().AF & 0xff;
                ImGui::Separator();
                ImGui::Text("Flags:");
                bool s = (f >> 7) & 1; ImGui::Checkbox("Sign", &s);
                bool z = (f >> 6) & 1; ImGui::Checkbox("Zero", &z);
                bool h = (f >> 4) & 1; ImGui::Checkbox("Half carry", &h);
                bool pv = (f >> 2) & 1; ImGui::Checkbox("Parity / Overflow (V)", &pv);
                bool n = (f >> 1) & 1; ImGui::Checkbox("Subtract (N)", &n);
                bool c = (f >> 0) & 1; ImGui::Checkbox("Carry", &c);
            }
    
        } else {
            ImGui::Text("CPU in execution...");
        }
    }
    ImGui::End();
}

void MyUI::draw_terminal()
{
    TerminalView const& t = p().terminalview();
    
    float cursor_x = 0.0;
    std::optional<float> cursor_y;
    ImGui::SetNextWindowSize({ 580, 480 });
    if (ImGui::Begin("Terminal", nullptr, ImGuiWindowFlags_NoResize)) {
    
        // terminal
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(100, 255, 100)));
        for (size_t i = 0; i < t.lines(); ++i) {
            if (i == t.cursor_y()) {
                cursor_x = ImGui::GetCursorPosX();
                cursor_y = ImGui::GetCursorPosY();
            }
            ImGui::Text("%s", t.text().at(i).c_str());
        }
        ImGui::PopStyleColor();
        
        // buttons
        if (stopped()) {
            if (ImGui::Button("Keypress... (F2)") || ImGui::IsKeyPressed(F2))
                show_keypress_modal = true;
        }
        
        // cursor
        if (cursor_y.has_value()) {
            // draw cursor
            ImGui::SetCursorPos({ cursor_x + (t.cursor_x() * 7), cursor_y.value() + 2 });
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(230, 60, 60)));
            ImGui::Button(" ", { 7, 11 });
            ImGui::PopStyleColor();
    
            // redraw charcter in black
            ImGui::SetCursorPos({ cursor_x + (t.cursor_x() * 7), cursor_y.value() });
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 255-60, 255-60)));
            // ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(0, 0, 0)));
            ImGui::Text("%c", t.text().at(t.cursor_y()).at(t.cursor_x()));
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void MyUI::draw_keypress_modal()
{
    if (ImGui::BeginPopupModal("Keypress", &show_keypress_modal, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Press a key to send to the Z80 computer.");
        ImGui::Separator();
        if (ImGui::Button("Cancel"))
            show_keypress_modal = false;
        ImGui::EndPopup();
    
        if (do_keypress())
            show_keypress_modal = false;
    }
    ImGui::OpenPopup("Keypress");
}

void MyUI::draw_advanced()
{
    bool logging = p().logging_to_stdout();
    bool empty = p().assert_empty_buffer();
    
    ImGui::Begin("Advanced", &show_advanced_window);
    
    if (stopped()) {
        try {
            if (ImGui::Button("Power Z80 off"))
                p().powerdown();
        } catch (std::runtime_error& e) {
            error("Error powering CPU down", e.what());
        }
        ImGui::SameLine();
        try {
            if (ImGui::Button("Send NMI"))
                p().nmi();
        } catch (std::runtime_error& e) {
            error("Error sending NMI", e.what());
        }
    }
    
    ImGui::Separator();
    
    try {
        if (ImGui::Checkbox("Data logging to stdout", &logging)) {
            p().set_logging_to_stdout(logging);
            config.log_to_stdout = logging;
        }
        if (ImGui::Checkbox("Assert empty buffer after finishing communication (makes execution slower)", &empty))
            p().set_assert_empty_buffer(empty);
        int item_current = static_cast<int>(p().register_fetch_mode());
        if (ImGui::Combo("Register fetch mode", &item_current, "Disabled\0NMI (fires a NMI interrupt on Z80, needs to be implemented in the OS)\0Emulator (get data directly from the emulator, not available on the real Z80)\0\0"))
            p().set_register_fetch_mode(static_cast<RegisterFetchMode>(item_current));
    } catch (std::runtime_error& e) {
        error("Error setting options", e.what());
    }
    
    ImGui::End();
}

void MyUI::draw_choose_file()
{
    if (ImGui::Begin("Choose file", &show_choose_file)) {
        ImGui::Text("Choose a file to go to:");
    
        static int tbl_flags = ImGuiTableFlags_BordersOuterH
                               | ImGuiTableFlags_BordersOuterV
                               | ImGuiTableFlags_BordersInnerV
                               | ImGuiTableFlags_BordersOuter
                               | ImGuiTableFlags_RowBg
                               | ImGuiTableFlags_ScrollY
                               | ImGuiTableFlags_Resizable
                               | ImGuiTableFlags_Reorderable
                               | ImGuiTableFlags_Hideable
                               | ImGuiTableFlags_Sortable
                               | ImGuiTableFlags_SortTristate;
    
        if (ImGui::BeginTable("##file", 1, tbl_flags)) {
            if (stopped()) {
                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                
                for (auto const& fname: file_list) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    if (ImGui::Selectable(fname.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick) && ImGui::IsMouseDoubleClicked(0)) {
                        p().codeview().set_file(fname);
                        ImGui::SetWindowFocus("Code debugger");
                    }
                }
            }
            
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void MyUI::draw_choose_symbol()
{
    if (ImGui::Begin("Choose symbol", &show_choose_file)) {
        ImGui::Text("Choose a symbol to go to:");
        
        static int tbl_flags = ImGuiTableFlags_BordersOuterH
                               | ImGuiTableFlags_BordersOuterV
                               | ImGuiTableFlags_BordersInnerV
                               | ImGuiTableFlags_BordersOuter
                               | ImGuiTableFlags_RowBg
                               | ImGuiTableFlags_ScrollY
                               | ImGuiTableFlags_Resizable
                               | ImGuiTableFlags_Reorderable
                               | ImGuiTableFlags_Hideable
                               | ImGuiTableFlags_Sortable
                               | ImGuiTableFlags_SortTristate;
        
        if (ImGui::BeginTable("##file", 2, tbl_flags)) {
            if (stopped()) {
                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                ImGui::TableSetupColumn("Symbol", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                
                for (auto const& symbol: symbol_list) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    int sel_flags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SpanAllColumns;
                    if (ImGui::Selectable(symbol.symbol.c_str(), false, sel_flags) && ImGui::IsMouseDoubleClicked(0)) {
                        auto oi = p().codeview().goto_symbol(symbol.symbol);
                        if (oi.has_value())
                            show_this_line_on_next_frame = *oi;
                        ImGui::SetWindowFocus("Code debugger");
                    }
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", symbol.file_line.c_str());
                }
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}


//
// ERROR MANAGEMENT
//

void MyUI::error(std::string const& title, std::string const& message)
{
    error_message = { title, message };
}

void MyUI::draw_error_modal()
{
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    if (ImGui::BeginPopupModal((*error_message).title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text((*error_message).message.c_str());
        ImGui::Separator();
        if (ImGui::Button("Ok"))
            error_message.reset();
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor();
    
    ImGui::OpenPopup((*error_message).title.c_str());
}

//
// ACTIONS
//

void MyUI::start_execution()
{
    std::string step;
    try {
        step = "connecting to emulator or serial port";
        if (config.emulator_mode) {
            presentation.emplace(config.emulator_path, true);
        } else {
            presentation.emplace(config.serial_port, false);
        }
        p().set_logging_to_stdout(config.log_to_stdout);
        
        step = "compiling project";
        p().compile_project(z80aw::DebugInformation::Vasm, config.project_file);
        if (config.emulator_mode) {
            p().set_register_fetch_mode(RegisterFetchMode::Emulator);
            p().upload_compiled();
        } else {
            p().update_upload_status();
        }
        
        step = "getting file/symbol list";
        update_symbol_list();
    
        step = "resetting CPU";
        p().reset();
        p().update();
        scroll_to_pc = true;
    } catch (std::runtime_error& e) {
        error("Error " + step, e.what());
        presentation.reset();
    }
}

void MyUI::update_symbol_list()
{
    file_list = p().file_list();
    symbol_list.clear();
    for (auto const& s: p().symbol_list()) {
        symbol_list.push_back({ s.symbol, s.file + ":" + std::to_string(s.line) });
    }
}

void MyUI::upload_binary()
{
    p().upload_compiled([](void*, float perc) {
        if (ImGui::BeginPopupModal("Uploading binary", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Uploading binary to memory: %d%%", static_cast<int>(perc * 100));
            ImGui::EndPopup();
        }
        ImGui::OpenPopup("Uploading binary");
    });
}

bool MyUI::do_keypress()
{
    bool press = false;
    for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) {
        if (ImGui::IsKeyPressed(i)) {
            for (uint8_t k: translate_keypress(i, io.KeyCtrl, io.KeyShift, io.KeyAlt, io.KeySuper))
                p().keypress(k);
            press = true;
        }
    }
    return press;
}


//
// translations
//

std::vector<uint8_t> MyUI::translate_keypress(int key, bool ctrl, bool shift, bool alt, bool super) const
{
    // std::cout << "Keypress: " << key << std::endl;
    
    if (ctrl || alt || super)   // not supported yet
        return {};
    if (key >= 'A' && key <= 'Z') {
        return { shift ? (uint8_t) key : (uint8_t) (key + 32) };
    } else if (key < 127) {
        return { (uint8_t) key };
    } else switch(key) {
        case GLFW_KEY_ENTER: return { '\r', '\n' };
        case GLFW_KEY_BACKSPACE: return { '\b' };
        default: return {};
    }
}

