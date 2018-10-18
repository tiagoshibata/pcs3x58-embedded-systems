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

int main() {
    int fd = serial_open("/dev/ttySAC0");
    gamepad_report_t report;
    report.signature = 0xaa;
    Serialize9Dof s;
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        bool has_data = false;
        has_data |= s.serialize(&report.x1);
        if (!has_data)
            continue;
        printf("Sending...\n");
        printf("%x %d %d %d %d\n", report.signature, report.x1, report.y1, report.x2, report.buttons);
        if (write(fd, (void *)&report, sizeof(report)) != sizeof(report)) {
            perror("write");
        }
    }
    return 0;
}
