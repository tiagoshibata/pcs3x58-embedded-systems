#include <RTIMULib.h>

class Serialize9Dof {
public:
    Serialize9Dof();
    ~Serialize9Dof();
    bool serialize(char *buffer);

private:
    RTIMUSettings *settings;
}
