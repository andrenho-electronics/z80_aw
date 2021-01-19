#ifndef DEBUGGER_CONFIG_HH
#define DEBUGGER_CONFIG_HH

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

struct Config {
    explicit Config(ImGuiContext* context);
    
    char emulator_path[1024] = "../libz80aw";
    char project_file[2048] { 0 };
    char serial_port[256] = "/dev/ttyUSB0";
    bool emulator_mode = true;

private:
    ImGuiContext* context;
    ImGuiSettingsHandler ini_handler;
};

#endif
