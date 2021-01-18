#include "myui.hh"

#include "imgui/imgui.h"

void MyUI::draw()
{
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
}

