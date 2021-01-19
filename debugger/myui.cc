#include "myui.hh"

#include "imgui/imgui.h"

void MyUI::draw()
{
    draw_demo();
    draw_start();
}

void MyUI::draw_demo()
{
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
}

void MyUI::draw_start()
{
    if (!pres.has_value()) {
        ImGui::Begin("Welcome to Z80AW debugger");
    
        ImGui::Text("Execution type");
        ImGui::SameLine();
        if (ImGui::RadioButton("Emulator", emulator_mode))
            emulator_mode = true;
        ImGui::SameLine();
        if (ImGui::RadioButton("Real hardware", !emulator_mode))
            emulator_mode = false;
        
        ImGui::Text("Select project file"); ImGui::SameLine();
        ImGui::InputText("##b", project_file, sizeof project_file);
        
        if (!emulator_mode) {
            ImGui::Text("Serial port"); ImGui::SameLine();
            ImGui::InputText("##a", serial_port, sizeof serial_port);
        } else {
            ImGui::Text("Emulator Path"); ImGui::SameLine();
            ImGui::InputText("##e", emulator_path, sizeof serial_port);
        }
        
        if (ImGui::Button("Start execution")) {
            if (emulator_mode)
                pres.emplace(Z80Presentation::initialize_with_emulator(emulator_path));
            else
                pres.emplace(serial_port);
        }
        
        ImGui::End();
    }
}

