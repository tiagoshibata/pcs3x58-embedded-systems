#include <cstdint>

#include <librealsense/rs.hpp>

class HandTracking {
public:
    HandTracking();
    ~HandTracking();
    bool serialize(std::int8_t *buffer);

private:
    std::int8_t last_reading;
    rs::device *dev;
};
