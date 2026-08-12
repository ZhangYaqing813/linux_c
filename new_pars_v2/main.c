#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


int main(int)
{
    int fd = open("/proc/self/status",O_RDONLY);
    if (fd < 0 ){
        printf("open file error");
        close(fd);
    }
    char buf[2048];
    ssize_t n = read(fd,buf,sizeof(buf)-1);
    if (n == 0 || n < 0) {
        printf("read buf faild ");
        close(fd);

    }
    close(fd);
    buf[n]= '\0';
    char * savePtr ;
    char * line ;

    line = strtok_r(buf,"\n",&savePtr);

    while (line != NULL)
    {
        char * key = NULL;
        char * value = NULL;
        char * seck = strchr(line,':');
        if (seck){
            seck[0] = '\0';
            key = line;
            value = seck + 1;
            while (*value == ' '|| *value == '\t')
            {
                value ++;
            }
            // key 的值是从尾部开始向前进行字符串的空值处理
            //key 指向的是开头的地址，+上长度-1 便指向了最后一个字符。
            char * end = key + strlen(key)-1;
            while (end > key && (*end == ' '|| *end == '\t'))
            {
                *end = '\0';
                end -- ;
            }
            printf("%s =: %s\n",key,value);
        }
        //这里需要吧seck 的值重置为'\0’ 吗？
        *seck = ':';
        line = strtok_r(NULL,"\n",&savePtr);
    }
    return 0;
}
