#include "core/Core.h"
#include <vector>

int main() {
    Core core = Core(1024);

    std::vector<uint32_t> programa = {
        0xFB000313,
        0x00435393,
        0x40435413, 
    };

    core.load_program(programa);
    core.run();

    return 0;
}
