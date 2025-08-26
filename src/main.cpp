
#include "core/Core.h"
#include <vector>

int main() {

    Core core = Core(1024);

    std::vector<uint32_t> programa = {
        0x00500513,
        0xFFF50593
    };

    core.load_program(programa);
    core.run();

    return 0;
}
