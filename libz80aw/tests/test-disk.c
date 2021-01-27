#include <stdio.h>
#include <unistd.h>

#include "../comm/z80aw.h"

#define ASSERT(msg, expr)  \
    printf("%s... ", msg); \
    if (expr) { printf("\e[0;32m✔\e[0m\n"); } else { printf("\e[0;31mX\e[0m\n"); exit(1); }

bool log_to_stdout = false;

int main(int argc, char* argv[])
{
    //
    // compile SDCARD disk
    //
    DebugInformation* di = compile_vasm_disk("z80src/sdcard/sdcard.toml");
    ASSERT("DebugInformation is not null", di);
    ASSERT("Compiler output is successful", debug_output(di, NULL, 0));
    if (log_to_stdout) {
        printf("Compiler output:\n\e[0;33m");
        debug_print(di);
        printf("\e[0m\n");
    }
    
    // generate image
    unlink("/tmp/sdcard.img");
    ASSERT("Generate image", debug_generate_image(di, "/tmp/sdcard.img") == 0);
    if (log_to_stdout) {
        printf("Image files:\n\e[0;33m");
        system("mdir -i /tmp/sdcard.img ::");
        printf("\e[0m\n");
    }
    
    unlink("/tmp/sdcard.img");
    debug_free(di);
    
    //
    // test in emulator
    //
    
}

