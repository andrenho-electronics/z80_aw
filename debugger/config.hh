#ifndef DEBUGGER_CONFIG_HH
#define DEBUGGER_CONFIG_HH

struct Config {
    Config();
    
    char emulator_path[1024] = "../libz80aw";
    char project_file[2048] { 0 };
    char serial_port[256] = "/dev/ttyUSB0";
    bool emulator_mode = true;
    
    void save();
};

#endif
