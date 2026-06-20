#include "../../include/utils/time.hpp"

uint32_t getTimestampNow() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now);
    uint32_t timestamp = static_cast<uint32_t>(duration.count());
    return timestamp;
}
