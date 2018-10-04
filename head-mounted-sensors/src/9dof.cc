#include <iostream>

#include <RTIMULib.h>

#include "9dof.hh"

Serialize9Dof::Serialize9Dof() {
    settings = new RTIMUSettings("/etc", "RTIMULib");
    RTIMU *imu = RTIMU::createIMU(settings);

    if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
        std::cerr << "No IMU found\n";
        exit(1);
    }
    imu->IMUInit();
    imu->setSlerpPower(0.02);
    imu->setGyroEnable(true);
    imu->setAccelEnable(true);
    imu->setCompassEnable(true);
}

bool Serialize9Dof::serialize(char *buffer) {
    //  poll at the rate recommended by the IMU
    // usleep(imu->IMUGetPollInterval() * 1000);
    if (!imu->IMURead())
        return false;
    RTIMU_DATA imuData = imu->getIMUData();
    // printf("%s\r", RTMath::displayDegrees("", imuData.fusionPose));
    return true;
}

Serialize9Dof::~Serialize9Dof() {
    delete settings;
}
