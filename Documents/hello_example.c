#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <minix/ioctl.h>
#include <sys/ioc_hello.h>

int main(int argc, char** argv) {
    int fd, a;

    char x[64] = "sproboje wpisac mniej i zobaczymy co sie stanie";

    if ((fd = open("/dev/hello", O_RDONLY)) < 0)
        exit(1);


    ioctl(fd, HIOCSETMSG, x);

    return 0;
}