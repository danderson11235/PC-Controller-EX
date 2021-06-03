#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

unsigned char req[5] = {0x01, 0x03, 0x00, 0x15, 0xE9};
unsigned char res[18] = {0};

int openPort(int comPort);
int blockingSend(char* REQ, int len, int fd);
int blockingRec(unsigned char* RES, int len, int fd);
int main() 
{
    int fd = openPort(5);
    if (fd < 0) return 1;
    if (blockingSend(req, 5, fd) != 0) {
        printf("blocking Send returned 1\n");
        return 1;
    }
    if (blockingRec(res, 18, fd) != 0) {
        printf("blocking rec returned 1\n");
        return 1;
    }
    printf("Returned message is:\n");
    for (int i = 0; i < 18; i++) printf("0x%02x ", res[i]);
    printf("\n");
    
    close(fd);
    return 0;
}

int openPort(int comPort)
{
    char port[] = "/dev/ttyS0";
    port[9] += comPort;
    int fd;
    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror("openPort: Unable to open port");
        return -1;
    }
    fcntl(fd, F_SETFL, 0);
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);//set baud
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD); //dont change owner of port | Enable recieveing 
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw input
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cc[VTIME] = 15;
    options.c_cc[VMIN] = 1;
    tcsetattr(fd, TCSAFLUSH, &options);
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
        printf("blocking send:read did not get ack, instead got %X\n", RES[0]);
        return 1;
    }    
    return 0;
}
int blockingRec( unsigned char* RES, int len, int fd)
{
    int n;
    unsigned char ACK[1] = {0x06};
    n = read(fd, RES, len);
    if (n != len || RES[0] != 0x01)
    {
        printf("blocking rec: read did not get RES\n");
        return 1;
    }
    unsigned char CS = 0xFF;
    for (int i = 1; i < len - 1; ++i) CS ^= RES[i];
    if (CS != RES[len-1]) 
    {
        printf("blocking rec: read error checksum should be %X, but is %X\n", RES[len-1], CS);
        n = write(fd, 0x15, 1);//NAK
        return 1;
    }
    n = write(fd, ACK, 1);
    if (n < 0)
    { 
        perror("blocking rec: could not ack");
        return 1;
    }
    return 0;
}