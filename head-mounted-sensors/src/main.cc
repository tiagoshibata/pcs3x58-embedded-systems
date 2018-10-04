#include <iostream>
#include <unistd.h>

#include "serial.h"

int main() {
    int fd = serial_open("/dev/ttySAC0");
    if (write(fd, "hello", 6) != 6) {
        perror("write");
    }
    return 0;
}
