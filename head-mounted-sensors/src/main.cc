#include <iostream>
#include <unistd.h>

#include "9dof.hh"
#include "serial.h"

typedef struct {
    int8_t signature;
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
        s.serialize((char *)&report.x1);
        if (write(fd, report, sizeof(report)) != sizeof(report)) {
            perror("write");
        }
    }
    return 0;
}
