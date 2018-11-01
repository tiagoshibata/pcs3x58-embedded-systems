#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "9dof.hh"
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

    void delay_write(int fd, int8_t value) {
        std::this_thread::sleep_for(std::chrono::milliseconds(24));
        if (write(fd, (void *)(&value), 1) != 1) {
            perror("write");
        }
    }

    void send_gamepad_report(int fd) {
        gamepad_report_t report_copy;
        int8_t *report_bytes = (int8_t *)&report_copy;
        for (;;) {
            report_copy = report;
            int8_t sum = 0;
            for (unsigned i = 0; i < sizeof(report); i++) {
                sum ^= report_bytes[i];
                delay_write(fd, report_bytes[i]);
            }
            delay_write(fd, sum);
            delay_write(fd, ~sum);
        }
    }
}


int main(int argc, char **argv) {
    int fd = serial_open("/dev/ttySAC0");
    int verbose = argc > 1 && std::string(argv[1]) == "-v";
    report.signature = 0x55;
    Serialize9Dof s;
    std::thread sender(send_gamepad_report, fd);
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
        bool has_data = false;
        has_data |= s.serialize(&report.x1);
        // has_data |= realsense_has_data;
        if (!has_data)
            continue;
        if (verbose)
            printf("%x %d %d %d %d\n", report.signature, report.x1, report.y1, report.x2, report.buttons);
    }
    return 0;
}
