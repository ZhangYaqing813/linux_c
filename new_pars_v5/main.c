#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


typedef int  (*callback ) (const char * key,const char * value,void *data);


typedef struct sysInfo
{
    char Name[32];
    char State[16];
    int Pid;
    int VmRSS;
};


int printer(const char *key,const char * value,void *data){
    //void *data;
    printf("key = %s ,value = %s \n",key,value);


}



int countTime(const char *key,const char *value ,void *data){

    int * count = (int *)data;
    (*count) ++;
    return 0 ;

}


int getSysInfo(const char *key, const char *value,void *data){
    // 这一行有点不理解，为什么 sinf 是指针类型？是因为*data 吗
    printf("this is getsysinfo \n");
    struct sysInfo *sinf = (struct sysInfo *) data;

    if (strcmp(key,"Name")==0){
        printf("this get sysinfo key = %s, value = %s\n",key,value);

       strncpy(sinf->Name,value,sizeof(sinf->Name)-1);
       sinf->Name[sizeof(sinf->Name)-1] = '\0';
        
    } else if (strcmp(key,"State")== 0)
    {
        /* code */
        strncpy(sinf->State,value,sizeof(sinf->State) -1);
        sinf->State[sizeof(sinf->State)-1] = '\0';
    } else if (strcmp(key,"Pid") == 0)
    {
        /* code */
        
        sinf->Pid = atoi(value);
        
    }else if (strcmp(key,"VmRSS") == 0)
    {
        /* code */
        sinf->VmRSS = atoi(value);
    }
    
    

    return 0;

}


int parsConfig(char *text,callback cb,void *data){
    printf("current is  parConfig \n");
    if(text == NULL){
        return -1;
    }

    char * saveptr;
    char *line = strtok_r(text,"\n",&saveptr);

    while(line != NULL){
        char *key;
        char *value;
        //先切分行，然后进行赋值操作
        char * seck = strchr(line,':');
        if(seck){
            *seck = '\0';
            key = line;
            value = seck +1;

            while (*value == ' '|| *value == '\t'){
                value ++;

            }

            char *end = key + strlen(key) -1;

            if (end > key && *end == ' '|| *end == '\t'){
                *end = '\0';
                end -- ;
            }

            int ret = cb(key,value,data);
            if (ret < 0 ){
                *seck = '\0';
                return -1;
            }


        }
        //值要用*p,
        *seck = ':';

        line = strtok_r(NULL,"\n",&saveptr);



    }

    return 0;

}



int main (){

    int fd = open("/proc/self/status",O_RDONLY);
    if (fd < 0){
        printf("open faild");
        return -1;
    }

    char buf[2048];
    ssize_t n = read(fd,buf,sizeof(buf));
    if (n < 0){
        printf("read error");
        return -1;
    }

    close(fd);
    /*回调函数parsconfig
    //parsConfig(buf,printer,NULL);
    

    // 回到计数函数
    int count = 0;
    int ret = parsConfig(buf,countTime,&count);
    printf("调用了 %d次\n",count);
    //printf("%s",buf);

    */
    struct sysInfo myInfo ;

    int ret = parsConfig(buf,getSysInfo,&myInfo);

    printf("name = %s\n",myInfo.Name);
    return 0;
}