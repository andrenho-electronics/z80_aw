#include "config.hh"

#include <fstream>
#include <iostream>

#include <getopt.h>

Config::Config(int argc, char **argv)
{
    while (true) {
        static struct option long_options[] = {
                { "help",     no_argument,       nullptr, 'h' },
                { "emulated", required_argument, nullptr, 'e' },
                { "real",     required_argument, nullptr, 'r' },
                { nullptr,    0,                 nullptr, 0 },
        };

        int idx;
        int c = getopt_long(argc, argv, "he:r:", long_options, &idx);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                print_usage(argv[0]);
                exit(1);
            case 'h':
                print_usage(argv[0]);
                exit(0);
            case 'e':
                hardware_type_ = Emulated;
                config_file_ = load_config_file(optarg);
                break;
            case 'r':
                std::cerr << "Sorry, support for real hardware is not yet implemented.\n";
                exit(1);
            case '?':
                break;
            default:
                abort();
        }
    }

    if (hardware_type_ == NotDefined) {
        print_usage(argv[0]);
        exit(0);
    }
}

void Config::print_usage(std::string const& argv0)
{
    std::cout << "Emulator for the Z80AW machine.\n";
    std::cout << "Usage: " << argv0 << " [-e CONFIGFILE] | [-r PORT]\n";
    std::cout << "  Choose either `-e` for running an emulation, or `-r` to connect to the real hardware.\n";
    std::cout << "  For emulation, a config file in the following format must be passed:\n";
    std::cout << "     ASSEMBLY_FILE_NAME:0x7000\n";
    std::cout << "     ...\n";
    std::cout << "  For the real hardware, a serial port (such as /dev/ttyUSB0) must be given.\n";
}

ConfigFile Config::load_config_file(char const *filename)
{
    ConfigFile cf;

    std::ifstream f(filename);
    if (f.fail()) {
        std::cerr << "File " << filename << " does not exist or could not be opened.\n";
        exit(1);
    }

    std::string line;
    int nline = 1;
    while (std::getline(f, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            std::cerr << "Config file is in incorrect format: missing colon in line " << nline << ".\n";
            exit(1);
        }
        std::string source_file = line.substr(0, colon_pos);
        std::string address_s = line.substr(colon_pos + 1);
        unsigned long address = strtoul(address_s.c_str(), nullptr, 0);
        if (address == ULONG_MAX) {
            std::cerr << "Invalid address in line " << nline << ".\n";
            exit(1);
        }
        cf.push_back({ source_file, static_cast<uint16_t>(address) });
        ++nline;
    }

    return cf;
}
