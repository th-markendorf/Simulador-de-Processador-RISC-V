#include "core/Core.h"
#include <vector>

int main() {
    Core core = Core(1024);

    std::vector<uint32_t> programa = {
        0x00A00293, // addi x5, x0, 10
        0x00C00313, // addi x6, x0, 12

        // Instruções Tipo-R
        0x0062F3B3, // and x7, x5, x6  ; x7 = 10 & 12 = 8
        0x0062E433, // or  x8, x5, x6  ; x8 = 10 | 12 = 14
        0x0062C4B3, // xor x9, x5, x6  ; x9 = 10 ^ 12 = 6

        // Instruções Tipo-I
        0x00F2F593, // andi x11, x5, 15 ; x11 = 10 & 15 = 10
        0x00F2E613, // ori  x12, x5, 15 ; x12 = 10 | 15 = 15
        0x00F2C693  // xori x13
    };

    core.load_program(programa);
    core.run();

    return 0;
}
