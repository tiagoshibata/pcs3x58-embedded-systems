#include <cstdint>

#include <librealsense/rs.hpp>

class HandTracking {
public:
    HandTracking(bool debug);
    ~HandTracking();
    bool serialize(std::int8_t *buffer);

private:
    rs::context ctx;
    std::int8_t last_reading;
    rs::device *dev;
};
