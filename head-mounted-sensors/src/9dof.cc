#include <cmath>
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

namespace {
#define ROTATION_LIMIT  (PI / 4)
    int8_t constrain(float angle) {
        angle = fmodf(angle, 2 * PI);
        if (angle > PI)
            angle -= 2 * PI;
        else if (angle < -PI)
            angle += 2 * PI;
        if (angle > ROTATION_LIMIT)
            return INT8_MAX;
        if (angle < -ROTATION_LIMIT)
            return INT8_MIN;
        return 127 * angle / ROTATION_LIMIT;
    }
}

bool Serialize9Dof::serialize(int8_t *buffer) {
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
    buffer[0] = constrain(imuData.fusionPose.data(0) + PI);
    buffer[1] = constrain(imuData.fusionPose.data(1));
    // printf("%f %f %f\n", imuData.fusionPose.x(), imuData.fusionPose.y(), imuData.fusionPose.z());
    return true;
}

Serialize9Dof::~Serialize9Dof() {
    if (imu)
        delete imu;
    delete settings;
}
