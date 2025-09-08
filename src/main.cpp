#include "core/Core.h"
#include <vector>

int main() {
    Core core = Core(1024);

    std::vector<uint32_t> programa = {
        0x02A00293,
        0x20000313,
        0x00532023,
        0x00000293,
        0x00032283
    };

    core.load_program(programa);
    core.run();

    return 0;
}
