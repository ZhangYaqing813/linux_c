#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd < 0)
    {
        printf("Error opening /proc/self/status\n");
    }
    char buf[2048] ;
    // char * buf[2048],这是一个指针数组，每个元素都是一个指针。
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    if (n < 0)
    {
        close(fd);
        printf("Error reading /proc/self/status\n");
    }
    buf[n] = '\0';
    printf("Status: %s\n", buf);

    printf("Hello, World!\n");
    close(fd);
    return 0;
}
