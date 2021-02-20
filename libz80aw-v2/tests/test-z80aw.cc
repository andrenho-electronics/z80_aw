#include "realhardware.hh"

int main()
{
    RealHardware r("/dev/ttyUSB0");
    r.set_log_bytes(true);
    r.set_log_messages(true);
    r.free_mem();
}
