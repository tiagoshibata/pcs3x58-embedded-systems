#include <iostream>
#include <unistd.h>

#include "9dof.hh"
#include "serial.h"

int main() {
    int fd = serial_open("/dev/ttySAC0");
    if (write(fd, "hello", 6) != 6) {
        perror("write");
    }
    Serialize9Dof s;
    return 0;
}
