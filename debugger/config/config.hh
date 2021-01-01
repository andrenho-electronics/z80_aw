#ifndef EMULATOR_CONFIG_HH
#define EMULATOR_CONFIG_HH

#include <string>
#include <vector>
#include <optional>

enum HardwareType { NotDefined, Emulated, Real };

struct ConfigLine {
    std::string filename;
    uint16_t    memory_location;
};

using ConfigFile = std::vector<ConfigLine>;

class Config {
public:
    Config(int argc, char* argv[]);

    [[nodiscard]] HardwareType                      hardware_type() const { return hardware_type_; }
    [[nodiscard]] ConfigFile const&                 config_file()   const { return config_file_; }
    [[nodiscard]] std::string const&                serial_port()   const { return serial_port_; }
    [[nodiscard]] std::optional<std::string> const& log_file()      const { return log_file_; }
    [[nodiscard]] std::string const&                source_path()   const { return source_path_; }

private:
    static void print_usage(std::string const& argv0);
    static std::string find_source_path(std::string const& filename);
    static ConfigFile load_config_file(std::string const& filename);

    HardwareType hardware_type_ = NotDefined;
    ConfigFile   config_file_;
    std::string  source_path_;
    std::string  serial_port_;
    std::optional<std::string> log_file_;
};

#endif
