#include <iostream>
#include <time.h>

#include <RTIMULib.h>

#include "9dof.hh"

constexpr auto PI = 3.14159265358979323846;

Serialize9Dof::Serialize9Dof() {
    settings = new RTIMUSettings("/etc", "RTIMULib");
    imu = RTIMU::createIMU(settings);

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
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    //  poll at the rate recommended by the IMU
    double delta_ms = (current_time.tv_sec - last_read.tv_sec) * 1000 + 1e-6 * (current_time.tv_nsec - last_read.tv_nsec);
    if (delta_ms < imu->IMUGetPollInterval())
        return false;
    last_read = current_time;
    if (!imu->IMURead())
        return false;
    RTIMU_DATA imuData = imu->getIMUData();
    if (!imuData.fusionPoseValid)
        return false;
    buffer[0] = imuData.fusionPose.x() / PI * 127;
    buffer[1] = imuData.fusionPose.y() / PI * 127;
    buffer[2] = imuData.fusionPose.z() / PI * 127;
    printf("%f %f %f\n", imuData.fusionPose.x(), imuData.fusionPose.y(), imuData.fusionPose.z());
    return true;
}

Serialize9Dof::~Serialize9Dof() {
    if (imu)
        delete imu;
    delete settings;
}
