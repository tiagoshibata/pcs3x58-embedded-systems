#include <time.h>

#include <RTIMULib.h>

class Serialize9Dof {
public:
    Serialize9Dof();
    ~Serialize9Dof();
    bool serialize(int8_t *buffer);

private:
    RTIMUSettings *settings;
    RTIMU *imu;
    struct timespec last_read = {};
};
