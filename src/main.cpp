#include "core/Core.h"
#include <vector>

int main() {
    Core core = Core(1024);

    std::vector<uint32_t> programa = {
        0x00500513,
        0x00000593,
        0x00A585B3,
        0xFFF50513,
        0x00050463,
        0xFF5FF06F,
        0x00000000
    };

    core.load_program(programa);
    core.run();

    return 0;
}
