#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

char req[5] = {0x01, 0x03, 0x00, 0x15, 0xE9};
char res[18] = {0};

int openPort(int comPort);
int blockingSend(char* REQ, int fd);
int blockingRec(char* RES, int fd);
int main() 
{
    int fd = openPort(5);

    return 0;
}

int openPort(int comPort)
{
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
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);//set baud
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD); //dont change owner of port | Enable recieveing 
    options.c_cflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw input
    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

int blockingSend(char* REQ, int len, int fd)
{
    int n;
    n = write(fd, REQ, len);
    if (n < 0)
    {
        printf("write() of blockingSend failed.\n");
        return 1;
    }
    char RES[1] = {0};
    n = read(fd, RES, 1);
    if (n != 1 || RES[0] != 0x06)
    {
        fprintf("blocking send:read did not get ack, instead got %X\n", RES[0]);
        return 1;
    }    
    return 0;
}
int blockingRec(char* RES, int len, int fd)
{
    int n;
    n = read(fd, RES, len);
    if (n != len)
    {
        printf("blocking rec: read did not get RES\n");
    }
    unsigned char CS;
    for (int i = 0; i < len - 1; ++i) CS ^= RES[i];
    if (CS != RES[len-1]) 
    {
        printf("blocking rec: read error checksum should be %X, but is %X\n", RES[len-1], CS);
        n = write(fd, 0x15, 1);//NAK
        return 1;
    }
    n = write(fd, 0x06, 1);
    if (n < 0)
    { 
        printf("blocking rec: could not ack");
        return 1;
    }
    return 0;
}