#include <iostream>
#include "myui.hh"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

static const int F7 = 0x128;
static const int F8 = 0x129;
static const int F9 = 0x12a;
static const int F12 = 0x12d;

MyUI::MyUI(Window const& window, Options const& options)
    : UI(window), show_demo_window(options.show_demo_window()), config(this->context)
{
}

//
// DRAW
//

void MyUI::draw()
{
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    if (!presentation.has_value()) {
        draw_start();
    } else {
        draw_code();
        draw_memory();
        draw_cpu();
        if (show_advanced_window)
            draw_advanced();
    }
    if (error_message.has_value())
        draw_error_modal();
}

void MyUI::draw_start()
{
    ImGui::Begin("Welcome to Z80AW debugger", nullptr);

    ImGui::Text("Execution type");
    ImGui::SameLine();
    if (ImGui::RadioButton("Emulator", config.emulator_mode))
        config.emulator_mode = true;
    ImGui::SameLine();
    if (ImGui::RadioButton("Real hardware", !config.emulator_mode))
        config.emulator_mode = false;
    
    ImGui::Separator();
    
    ImGui::Text("Select project file"); ImGui::SameLine();
    ImGui::InputText("##b", config.project_file, sizeof config.project_file);
    
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
    ImGui::Begin("Code debugger");
    
    if (p().mode() == Z80State::Stopped) {
        if (ImGui::Button("Step (F7)") || ImGui::IsKeyPressed(F7, false)) {
            try { p().step(); } catch (std::runtime_error& e) { error("Error during step", e.what()); }
        }
        ImGui::SameLine();
        if (ImGui::Button("Next (F8)") || ImGui::IsKeyPressed(F8, false)) {
            try { p().next(); } catch (std::runtime_error& e) { error("Error during next step", e.what()); }
        }
        ImGui::SameLine();
        if (ImGui::Button("Run (F9)") || ImGui::IsKeyPressed(F9, false)) {
            try { p().continue_(); } catch (std::runtime_error& e) { error("Error during continue", e.what()); }
        }
    } else {
        if (ImGui::Button("Stop (Ctrl+C)") || (io.KeyCtrl && ImGui::IsKeyPressed('c', false))) {
            try { p().stop(); } catch (std::runtime_error& e) { error("Error stopping execution", e.what()); }
        }
    }
    
    ImGui::Separator();
    
    ImGui::Separator();
    
    if (ImGui::Button("Reset CPU")) {
        try { p().reset(); } catch (std::runtime_error& e) { error("Error resetting CPU", e.what()); }
    }
    ImGui::Button("Go to file...");
    ImGui::SameLine();
    ImGui::Button("Go to symbol...");
    ImGui::SameLine();
    
    if (ImGui::Button("Advanced..."))
        show_advanced_window = true;
    
    ImGui::End();
}

void MyUI::draw_memory()
{

}

void MyUI::draw_cpu()
{

}

void MyUI::draw_advanced()
{
    bool logging = p().logging_to_stdout();
    bool empty = p().assert_empty_buffer();
    RegisterFetchMode mode = p().register_fetch_mode();
    
    ImGui::Begin("Advanced", &show_advanced_window);
    
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
    
    ImGui::Separator();
    
    try {
        if (ImGui::Checkbox("Data logging to stdout", &logging))
            p().set_logging_to_stdout(logging);
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
            p().set_register_fetch_mode(RegisterFetchMode::Emulator);
        } else {
            presentation.emplace(config.serial_port, false);
        }
        
        step = "compiling project";
        p().compile_project(z80aw::DebugInformation::Vasm, config.project_file);
        
        step = "resetting CPU";
        p().reset();
    } catch (std::runtime_error& e) {
        error("Error " + step, e.what());
        presentation.reset();
    }
}

