#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


typedef int  (*callback)(const char *key, const char *value, void *data);

int count_cb(const char *key, const char *value, void *data)
{
    int * count  = (int *)data;
    (*count)++ ;
    return 0;
}

int printOut(const char *key, const char *value, void *data)
{
    //count_cb(key, value, data);
    printf("key = %s, value = %s\n", key, value);
    return 0;
}

int parsConfig(char *text, callback callback, void *data)
{
    if (text == NULL)
    {
        return -1;
    }

    char * line = NULL;
    char * saveptr = NULL;

    line = strtok_r(text, "\n", &saveptr);
    while (line != NULL)
    {
        char * key,*value;
        char * seck = strchr(line, ':');
        if (seck)
        {
            *seck = '\0';
            key = line;
            value = seck + 1;
            // 去除空白字符
            while (*value ==  ' ' || *value ==  '\t')
            {
                value++;
            }

            char * end = key + strlen(key) - 1 ;
            while(end > key &&  (*end == ' ' || *end == '\t'))
             {
                 *end = '\0';
                 end --;
             }
            int ret = callback(key, value, data);
            if (ret < 0)
            {
                *seck = ':';
                return -1;
            }
        }
        *seck = ':';
        line = strtok_r(NULL,"\n", &saveptr);
    }
    return 0;

}

int main(void)
{
    int fd =  open("/proc/self/status",O_RDONLY);
    if (fd < 0)
    {
        printf("Error opening /proc/self/status\n");
        close(fd);
        return -1;
    }

    char buf[2048];
    ssize_t n = read(fd,buf,sizeof(buf));
    if (n < 0|| n == 0)
    {
        printf("Error reading /proc/self/status\n");
        close(fd);
        return -1;
    }
    buf[n] = '\0';
    int count = 0;
    parsConfig(buf,count_cb,&count);
    printf("count = %d\n",count);

    return 0;
}
