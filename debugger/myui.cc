#include <iostream>
#include "myui.hh"

#include "imgui/imgui.h"

static const int F7 = 0x128;
static const int F8 = 0x129;
static const int F9 = 0x12a;
static const int F12 = 0x12d;

MyUI::MyUI(Window const& window, Options const& options)
    : UI(window), show_demo_window(options.show_demo_window())
{
}

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
    }
    if (error_message.has_value())
        draw_error_modal();
}

void MyUI::draw_start()
{
    ImGui::Begin("Welcome to Z80AW debugger", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Execution type");
    ImGui::SameLine();
    if (ImGui::RadioButton("Emulator", emulator_mode))
        emulator_mode = true;
    ImGui::SameLine();
    if (ImGui::RadioButton("Real hardware", !emulator_mode))
        emulator_mode = false;
    
    ImGui::Separator();
    
    ImGui::Text("Select project file"); ImGui::SameLine();
    ImGui::InputText("##b", project_file, sizeof project_file);
    
    if (!emulator_mode) {
        ImGui::Text("Serial port"); ImGui::SameLine();
        ImGui::InputText("##a", serial_port, sizeof serial_port);
    } else {
        ImGui::Text("Emulator Path"); ImGui::SameLine();
        ImGui::InputText("##e", emulator_path, sizeof serial_port);
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("Start execution (F12)") || ImGui::IsKeyPressed(F12, false)) {
        std::string step;
        try {
            step = "connecting to emulator or serial port";
            if (emulator_mode)
                presentation.emplace(emulator_path, true);
            else
                presentation.emplace(serial_port, false);
            
            step = "compiling project";
            p().compile_project(z80aw::DebugInformation::Vasm, project_file);
            
            step = "resetting CPU";
            p().reset();
        } catch (std::runtime_error& e) {
            error("Error " + step, e.what());
            presentation.reset();
        }
    }
    
    ImGui::End();
}

void MyUI::draw_code()
{
    ImGui::Begin("Code debugger");
    
    if (p().mode() == Z80State::Stopped) {
        if (ImGui::Button("Step (F7)") || ImGui::IsKeyPressed(F7, false)) {
            p().step();
        }
        ImGui::SameLine();
        if (ImGui::Button("Next (F8)") || ImGui::IsKeyPressed(F8, false)) {
            p().next();
        }
        ImGui::SameLine();
        if (ImGui::Button("Run (F9)") || ImGui::IsKeyPressed(F9, false)) {
            p().continue_();
        }
    } else {
        if (ImGui::Button("Stop (Ctrl+C)") || (io.KeyCtrl && ImGui::IsKeyPressed('c', false))) {
            p().stop();
        }
    }
    ImGui::SameLine();
    ImGui::Button("Advanced...");
    
    ImGui::Button("Go to file...");
    ImGui::SameLine();
    ImGui::Button("Go to symbol...");
    ImGui::SameLine();
    ImGui::Button("Options...");
    
    ImGui::End();
}

void MyUI::draw_memory()
{

}

void MyUI::draw_cpu()
{

}

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

