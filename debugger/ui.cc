#include "ui.hh"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

UI::UI(Window const& window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.ptr(), true);
    ImGui_ImplOpenGL3_Init("#version 150");
}

UI::~UI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    draw();
    ImGui::Render();
}

void UI::render_draw()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
