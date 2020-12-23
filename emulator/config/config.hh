#ifndef EMULATOR_CONFIG_HH
#define EMULATOR_CONFIG_HH

#include <string>
#include <vector>

enum HardwareType { NotDefined, Emulated, Real };

struct ConfigLine {
    std::string filename;
    uint16_t    memory_location;
};

using ConfigFile = std::vector<ConfigLine>;

class Config {
public:
    Config(int argc, char* argv[]);

    HardwareType      hardware_type() const { return hardware_type_; }
    ConfigFile const& config_file()   const { return config_file_; }

private:
    static void print_usage(std::string const& argv0);
    static ConfigFile load_config_file(const char* filename);

    HardwareType hardware_type_ = NotDefined;
    ConfigFile   config_file_;
    std::string  serial_port_;
};

#endif
