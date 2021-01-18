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
    if (show_start_window) {
        ImGui::Begin("Welcome to Z80AW debugger");
    
        ImGui::Text("Execution type");
        ImGui::SameLine();
        ImGui::RadioButton("Emulator", emulator_mode);
        ImGui::SameLine();
        ImGui::RadioButton("Real hardware", !emulator_mode);
        
        ImGui::Text("Select project file"); ImGui::SameLine();
        ImGui::InputText("##b", project_file, sizeof project_file);
        
        ImGui::Text("Serial port"); ImGui::SameLine();
        ImGui::InputText("##a", serial_port, sizeof serial_port);
        
        ImGui::Button("Start execution");
        
        ImGui::End();
    }
}

