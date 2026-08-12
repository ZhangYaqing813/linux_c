#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{

    int fd = open("/proc/self/status",O_RDONLY);
    if (fd < 0 )
    {
        printf ("Error opening /proc/self/status\n");
        close (fd);
    }
    char buf[2048];
    ssize_t n =  read(fd,buf,sizeof(buf)-1);
    buf[n] = '\0';

    if  (n == 0)
    {
        printf ("Read EOF\n");
        close (fd);
    }
    close (fd);

    char * srt = NULL;
    char * lines = NULL;

    lines = strtok_r (buf, "\n", &srt);
    //printf("%s\n",srt );
    while (lines != NULL) {
        char *key;
        char *valuel;
        char *tmp;
        key= strtok_r(lines,":",&tmp);

        valuel = strtok_r(NULL,":",&tmp);
       // printf ("%s=%s\n",key,valuel);
       // printf ("lenth = %d\n",strlen(valuel));
        for (int i = 0;i<strlen(valuel);i++) {
            //printf("i = %d\n",i);
            //printf ("i= %d,   value[i] =%c\n",i,valuel[i]);
            if (valuel[i] == ' ' || valuel[i] == '\t') {
                valuel[i] = valuel[i+1];

            }

        }
        printf ("key=%s, value=%s\n", key, valuel);
        lines =  strtok_r (NULL, "\n", &srt);

    }
    return 0;
}
