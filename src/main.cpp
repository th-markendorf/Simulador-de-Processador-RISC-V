
#include "core/Core.h"
#include <vector>

int main() {

    Core core = Core(1024);

    std::vector<uint32_t> programa = {
        0x01400293, 
        0x00F00313,
        0x006283B3,
        0x40638433
    };

    core.load_program(programa);
    core.run();

    return 0;
}
