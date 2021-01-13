#include "../z80pres.hh"

int main()
{
    z80pres::initialize_emulator("../emulator");
    
    z80pres::finalize();
}
