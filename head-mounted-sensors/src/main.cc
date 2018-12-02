#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include <cstdlib>
#include <unistd.h>

#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>

#include "9dof.hh"
#include "hand_tracking.hh"
#include "serial.h"

typedef struct {
    uint8_t signature;
    int8_t x1;
    int8_t y1;
    int8_t x2;
    uint8_t buttons;
} __attribute__((packed)) gamepad_report_t;

namespace {
gamepad_report_t report = {};
std::atomic<bool> running(true);

void delay_write(int fd, int8_t value) {
    std::this_thread::sleep_for(std::chrono::milliseconds(12));
    if (write(fd, (void *)(&value), 1) != 1) {
        perror("write");
    }
}

void send_gamepad_report(int fd) {
    gamepad_report_t report_copy;
    int8_t *report_bytes = (int8_t *)&report_copy;
    while (running) {
        report_copy = report;
        int8_t sum = 0;
        for (unsigned i = 0; i < sizeof(report); i++) {
            sum ^= report_bytes[i];
            delay_write(fd, report_bytes[i]);
        }
        delay_write(fd, sum);
        delay_write(fd, ~sum);
    }
    close(fd);
}

void usage(const char *name) {
    std::cerr << "Usage: " << name << " [-t time] [-v]\n";
}
}


int main(int argc, char **argv) {
    int fd = serial_open("/dev/ttySAC0");
    bool verbose = false;
    int time = -1;
    int opt;
    while ((opt = getopt(argc, argv, "vt:")) != -1) {
        switch (opt) {
        case 'v':
            verbose = true;
            break;
        case 't':
            time = std::atoi(optarg);
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }
    if (optind < argc) {
        usage(argv[0]);
        return -1;
    }
    report.signature = 0x55;
    Serialize9Dof s;
    HandTracking hand_tracking(verbose);
    std::thread sender(send_gamepad_report, fd);

    while (time) {
        if (time > 0)
            time--;

        std::this_thread::sleep_for(std::chrono::milliseconds(4));

        bool has_data = false;
        has_data |= s.serialize(&report.y1);
        // has_data |= realsense_has_data;

        if (!has_data)
            continue;
        if (verbose)
            printf("%x %d %d %d %d\n", report.signature, report.x1, report.y1, report.x2, report.buttons);
    }
    running = false;
    sender.join();
    return 0;
}
