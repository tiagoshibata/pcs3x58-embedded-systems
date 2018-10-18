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
    uint8_t end_of_packet;
} __attribute__((packed)) gamepad_report_t;

void send_gamepad_report(int fd, gamepad_report_t *report) {
    char *report_bytes = (char *)report;
    for (unsigned i = 0; i < sizeof *report; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        if (write(fd, (void *)(report_bytes + i), 1) != 1) {
            perror("write");
            return;
        }
    }
}

int main() {
    int fd = serial_open("/dev/ttySAC0");
    gamepad_report_t report;
    report.signature = 0x55;
    report.end_of_packet = 0x5a;
    Serialize9Dof s;
    for (;;) {
        bool has_data = false;
        has_data |= s.serialize(&report.x1);
        if (!has_data)
            continue;
        printf("Sending...\n");
        printf("%x %d %d %d %d\n", report.signature, report.x1, report.y1, report.x2, report.buttons);
        send_gamepad_report(fd, &report);
    }
    return 0;
}
