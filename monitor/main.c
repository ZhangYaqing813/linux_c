#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>

//procinfo is processes info
typedef struct procinfo {
    int Pid;
    char Name[32];
    char State[16];
    int VmRSS;
} procinfo ;

typedef struct meminfo {
    int MemTotal;
    int MemFree;
    int MemAvailable;
} meminfo ;

typedef int (*callback) (const char *key, const char *value, void *data);

static int printer(const char *key, const char *value, void *data) {
    printf("key = %s, value = %s\n", key, value);
    return 0;

}
//getpinfo get process info
int getpinfo (const char *key, const char *value, void *data) {

    struct procinfo *info = (struct procinfo *) data;
    if (strcmp(key, "Name") == 0) {//字符串比较用strcmp()
        strncpy(info->Name,value,sizeof(info->Name)-1);// 字符串赋值用strncpy()
        info->Name[sizeof(info->Name)-1] = '\0'; // 增加截断
    } else if (strcmp(key, "State") == 0) {
        strncpy(info->State,value,sizeof(info->State)-1);
        info->State[sizeof(info->State)-1] = '\0';

    } else if (strcmp(key, "VmRSS") == 0) {
        info->VmRSS = atoi(value);  // int 类型用atoi
    } else if (strcmp(key, "Pid") == 0) {
        info->Pid = atoi(value);
    }

    return 0;

}

int getmem(const char *key, const char *value, void *data) {
    meminfo *info = (meminfo *) data;
    if (strcmp(key, "MemTotal") == 0) {
        info->MemTotal = atoi(value);
    } else if (strcmp(key, "MemFree") == 0) {
        info->MemFree = atoi(value);
    } else if (strcmp(key, "MemAvailable") == 0) {
        info->MemAvailable = atoi(value);
    }
    return 0;
}

int parsConfig(char *text,callback cb,const char * deadline,void *data) {

    printf("parsconfig begin \n");
    if (text == NULL) {
        return -1;
    }

    char * savestr = NULL;
    char *line = strtok_r(text, "\n", &savestr);
    while (line != NULL  ) {
        char * key;
        char * value;

        char * seck = strchr(line, *deadline);
        if (seck != NULL) {
            *seck = '\0';
            key = line;
            value = seck+ 1;
            while (*value == ' ' || * value == '\t') {
                value++;
            }
           // printf("key and value ");
            char *end = key +strlen(key)-1;
            while (*end == ' ' || *end == '\t') {
                *end = '\0';
                /*
                 **end-- 会先解引用 end（读一下它指向的字符），
                 *然后指针 end 向后移。但你已经把 *end 写成了 '\0'，
                 *再解引用就只是读了那个 '\0'，没有任何作用。
                 *更糟的是，因为 *end 已经被置零，循环条件会在下一轮检查时访问一个可能越界的位置。
                 */
                // *end --;
                end -- ;
            }
            *seck = '\0';

            int ret = cb(key, value, data);
            if (ret <0 ) {
                return -1;
            }
            //printf("callback after ");
        }
        /*
         *你第一次切断 *seck = '\0' 是为了把 key 和 value 分开，这没问题。但之后应该恢复成原来的分隔符（这里是 ':'），而不是继续置零
         */
        // *seck = '\0';
        *seck  = ':';
        line = strtok_r(NULL, "\n", &savestr);
    }
    return 0;

}


int myopen(const char *filename,int flag  ,void *data, size_t bufsize ) {
    printf("myopen begin \n");
    char * buf = (char *)data;

    int fd = open(filename, flag);
    if (fd < 0) {
        printf("open file %s error \n",filename);
        return -1;
    }
   // printf("fd = %d\n",fd);
    //ssize_t 和 size_t 的区别就是有符号和无符号的问题，linux 有符号中最高位表示符号- ，如果系统收到的是带符号的数字，则无法判断函数执行情况
    ssize_t n = read(fd,buf,bufsize - 1);
    if (n < 0) {
        printf("read file %s error \n",filename);
        return -1;
    }
    close(fd);
    buf[n] = '\0';
    return 0;
}








int main(void) {

    char buf[2048];
    char  deadline = ':';
    int ret = myopen("/proc/self/status",O_RDONLY,buf,sizeof(buf));
    if (ret < 0) {
        printf("myopen error \n");
    }
    //printf(buf);
    //parsConfig(buf,printer,&deadline,NULL);
    struct procinfo p;
    parsConfig(buf,getpinfo,&deadline,&p);

    int sockfd ,connfd;
    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(18080);

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd < 0) {
        perror("socket error");
        close(sockfd);
        return -1;
    }
    int bn = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    if (bn < 0) {
        perror("bind error");
        close(sockfd);
        return -1;

    }
    int tn =  listen(sockfd,5);
    if (tn < 0) {
        perror("listen error");
        close(sockfd);
        return -1;
    }


    for (;;) {
        connfd = accept(sockfd,NULL,NULL);
        if (connfd < 0) {
            perror("accept error");
            continue;

        }
        char msg[1024];
        int ln = snprintf(msg,sizeof(msg),"Process Name: %s\nState: %s\nPID: %d\nVmRSS: %d kB\n ",
                       p.Name, p.State, p.Pid, p.VmRSS);
        // 分开处理两种不同的情况，ln <= 0 snprintf 执行失败
        if (ln < 0 ) {
            strncpy(msg,"Connection timed out",sizeof(msg)-1);
            ln = strlen(msg);
        }
        // ln 过大时保护缓冲区
        if (ln >= sizeof(msg)) {
            ln = sizeof(msg)-1;
        }


        ssize_t wn = write(connfd,msg,ln);
        if (wn < 0) {
            perror("write error");
        }
        close(connfd);
        /*
        char bf[1024];

        ssize_t rn = read (connfd,bf,sizeof(bf));
        if (rn < 0) {
            perror("read error");
        }
        printf("%s\n",bf);
        */
    }

    return 0;
}
