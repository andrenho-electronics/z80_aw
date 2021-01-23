#include "config.hh"

#include <cstring>

Config::Config(ImGuiContext* context)
    : context(context)
{
    ini_handler.TypeName = "UserData";
    ini_handler.TypeHash = ImHashStr("UserData");
    ini_handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler* h, const char* name) -> void* {
        if (strcmp(name, "Config") == 0)
            return h->UserData;
        return nullptr;
    };
    ini_handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) {
        auto* config = reinterpret_cast<Config*>(entry);
        char str[2048];
        if (sscanf(line, "emulator_mode=%16s", str) == 1)   config->emulator_mode = (strcmp(str, "true") == 0);
        if (sscanf(line, "emulator_path=%1024s", str) == 1) strcpy(config->emulator_path, str);
        if (sscanf(line, "project_file=%2048s", str) == 1)  strcpy(config->project_file, str);
        if (sscanf(line, "serial_port=%256s", str) == 1)    strcpy(config->serial_port, str);
        if (sscanf(line, "log_to_stdout=%16s", str) == 1)   config->log_to_stdout = (strcmp(str, "true") == 0);
    };
    ini_handler.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* h, ImGuiTextBuffer* buf) {
        auto* config = reinterpret_cast<Config*>(h->UserData);
        buf->appendf("[UserData][Config]\n");
        buf->appendf("emulator_mode=%s\n", config->emulator_mode ? "true" : "false");
        buf->appendf("emulator_path=%s\n", config->emulator_path);
        buf->appendf("project_file=%s\n", config->project_file);
        buf->appendf("serial_port=%s\n", config->serial_port);
        buf->appendf("log_to_stdout=%s\n", config->log_to_stdout ? "true" : "false");
    };
    ini_handler.UserData = this;
    context->SettingsHandlers.push_back(ini_handler);
}

