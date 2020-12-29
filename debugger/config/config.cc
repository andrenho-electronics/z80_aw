#include "config.hh"

#include <fstream>
#include <iostream>

#include <getopt.h>
#include <climits>

Config::Config(int argc, char **argv)
{
    int idx;
    while (true) {
        static struct option long_options[] = {
                { "help",     no_argument,       nullptr, 'h' },
                { "emulated", no_argument,       nullptr, 'e' },
                { "real",     required_argument, nullptr, 'r' },
                { "log",      required_argument, nullptr, 'l' },
                { nullptr,    0,       nullptr, 0 },
        };

        int c = getopt_long(argc, argv, "her:l:", long_options, &idx);
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
                break;
            case 'r':
                hardware_type_ = Real;
                serial_port_ = optarg;
                break;
            case 'l':
                log_file_ = optarg;
                break;
            case '?':
                break;
            default:
                abort();
        }
    }

    if (hardware_type_ == NotDefined) {
        print_usage(argv[0]);
        exit(1);
    }

    if (optind != argc - 1) {
        print_usage(argv[0]);
        exit(1);
    }

    config_file_ = load_config_file(argv[optind]);
}

void Config::print_usage(std::string const& argv0)
{
    std::cout << "Emulator for the Z80AW machine.\n";
    std::cout << "Usage: " << argv0 << " [-e] | [-r PORT] [-l LOGFILE] CONFIGFILE\n";
    std::cout << "  Choose either `-e` for running an emulation, or `-r` to connect to the real hardware.\n";
    std::cout << "  A config file name in the following format must be given:\n";
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
        if (line[0] == '#')
            continue;
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
