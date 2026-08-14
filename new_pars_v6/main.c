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


typedef struct sysinfo   {
    char    Name[32];
    char    State[32];
    int     Pid;
    int     VmRSS;
} ;


typedef int (*callback)(const char  *key, const char *value, void *data);


int count(const char *key, const char *value,void *data) {
    int *count = (int *)data ;
    (*count)++;
    return 0;

}

int printer(const char  *key, const char  *value, void *data) {
    printf("key = %s, value = %s\n", key, value);
}

int getsysinfo(const char  *key, const char  *value, void *data) {
    struct sysinfo *info = (struct sysinfo *)data;

        if (strcmp(key, "Name") == 0) {
            strncpy(info->Name, value,sizeof(info->Name)-1);
            info->Name[sizeof(info->Name)-1] = '\0';
        } else if (strcmp(key, "State") == 0) {
            strncpy(info->State,value,sizeof(info->State)-1);
            info->State[sizeof(info->State)-1] = '\0';
        }   else if (strcmp(key, "Pid") == 0) {
            info->Pid = atoi(value);
        } else if (strcmp(key, "VmRSS") == 0) {
            info->VmRSS = atoi(value);
        }

    return 0;

}

int splicing() {

}

int parConfig(char *text, callback cb, void *data) {
    printf("parConfig\n");
    if (text == NULL) {
        return -1;
    }

    char *saveptr = NULL;
    char *line = strtok_r(text, "\n", &saveptr);
    while (line != NULL) {
        char *key ;
        char *value ;
        char *seck = strchr(line, ':') ;
        if (seck != NULL) {
            *seck = '\0';
            key = line;
            value = seck + 1 ;
            while (*value == ' '|| *value == '\t'){
                value ++;

            }
            char *end = key + strlen(key)-1 ;
            while (end > key& *end == ' ' || *end == '\t' ) {
                *end = '\0' ;
                *end-- ;
            }
            *seck = '\0';
            int ret=  cb(key, value, data);
            if (ret < 0) {

                return -1;
            }
        }
        *seck = '\0' ;
        line = strtok_r(NULL, "\n", &saveptr);
    }

    return 0;
}


int main(void) {

    int fd = open("/proc/self/status", O_RDONLY);
    if (fd < 0) {
        printf("Error opening /proc/self/status\n");
        return -1;
    }

    char buf[2048];
    ssize_t n = read(fd, buf, sizeof(buf)-1);


    if (n < 0 || n == 0 ) {
        perror("Error reading status file");

        return -1;
    }
    buf[n] = '\0';
    close(fd);
    struct sysinfo p;

    parConfig(buf, getsysinfo, &p);
    //printf("process_name  : %s \nprocess_state :  %s\nprocess_id : %d \n ",p.Name, p.State, p.Pid);

    int sockfd ,connfd;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(18080);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        close(sockfd);
    }
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(sockfd, 5);

    for (;;) {
        connfd = accept(sockfd, NULL, NULL);
        if (connfd < 0) {
            perror("accept failed");
            continue;
        }
        char msg[1024];

        int ln = snprintf(msg, sizeof(msg), "Process Name: %s\nState: %s\nPID: %d\nVmRSS: %d kB\n",
                       p.Name, p.State, p.Pid, p.VmRSS);
        if (ln > sizeof(msg)-1 || ln < 0) {
            strncpy(msg, "Connection timed out", sizeof(msg)-1);
            ln = strlen (msg);
        }
        ssize_t  wn = write(connfd, msg,ln);
        if (wn < 0) {
            perror("write failed");
        }
        char bf[1024];
        ssize_t rn = read(connfd, bf, sizeof(bf));

        if (rn < 0) {
            perror("read failed");
        }
        printf("%s\n",bf);


    }
    return 0;
}
