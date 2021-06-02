#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

char ack[1] = {0x06};
char REQ[5] = {0x01, 0x03, 0x00, 0x15, 0xE9};
struct termios options;

int openPort(int comPort);

int main() {
    int fd = openPort(5);
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);//set baud
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD); //dont change owner of port | Enable recieveing 
    options.c_cflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw input
    tcsetattr(fd, TCSANOW, &options);

    int n;
    n = write(fd, REQ, 5);
    if (n < 0)
    {
        fputs("write() of versionMsg failed.\n", stderr);
        return 1;
    }
    char RES[18];
    n = read(fd, RES, 1);
    if (n != 1)
    {
        printf("read did not get ack");
        return 1;
    }
    n = read(fd, RES, 18);
    if (n != 18)
    {
        printf("read did not get RES");
    }
    n = write(fd, ack, 1);
    return 0;
}

int openPort(int comPort){
    char port[] = "/dev/ttyS0";
    port[9] += comPort;
    int fd;
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        perror("openPort: Unable to open port");
    }
    else
    {
        fcntl(fd, F_SETFL, 0);
    }
    return fd;
}