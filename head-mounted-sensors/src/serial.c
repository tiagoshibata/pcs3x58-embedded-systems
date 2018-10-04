#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

int serial_open(const char *port) {
    int fd = open(port, O_WRONLY | O_NOCTTY);
    if (fd == -1) {
        perror("open");
        exit(-1);
    }
    struct termios tty;
    if (tcgetattr(fd, &tty)) {
        perror("tcgetattr");
        exit(-1);
    }

    cfsetospeed(&tty, B115200);

    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;

    tty.c_cflag &= ~(CSTOPB | CSIZE);
    tty.c_cflag |= CLOCAL | CREAD | PARENB | CS8;

    if (tcsetattr(fd, TCSANOW, &tty)) {
        perror("tcsetattr");
        exit(-1);
    }
    return fd;
}
