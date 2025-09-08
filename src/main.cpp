#include "core/Core.h"
#include <vector>

int main() {
    Core core = Core(1024);

    std::vector<uint32_t> programa = {
        0x00018537,
        0x6A050513
    };

    core.load_program(programa);
    core.run();

    return 0;
}
