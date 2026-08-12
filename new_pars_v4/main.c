#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


typedef int (*callback) (const char *key, const char * value, void *data);


typedef  struct sysInfo {
    char Name;
    char State;
    int  Pid;
    int  VmRSS; 
};
// printOut 用来输出的
int printOut(const char *key, const char *value,void *data){
    //void *data;

    if (*key == ' ' || *value == ' ' ){
        printf("key or value is null !");
        return -1;
    }
    printf("key = %s,value = %s\n ",key,value);
    return 0;
}

// count 函数是用来统计调用次数的
int countF(const char * key,const char *value ,void *data){
    //做void data 的强制类型转换，
    int *count = (int *)data;
    (*count) ++ ;
    return 0;
}


//
int getSysInfo(const char *key , const char *value,void *data){
    printf("getSysinfo ==========");
    struct  sysInfo sinfo;

    if (key == "Name" ){
        sinfo.Name = *value;
    
    }else if (key == "State") {
        sinfo.State = *value;

    }else if (key == "Pid")
    {
        sinfo.Pid = *value ;

    } else if (key == "VmRSS")
    {
        /* code */
        sinfo.VmRSS = *value;
    }
    

    printf("sinfo is %s \n",sinfo);

    return 0;
}




// parsConfig 使用来解析配置文件的
int parsConfig(char *text,callback callback,void *data ){
    printf("parsConfig ==========");
    if(text == NULL){
        return -1;
    }
    //定义一个变量保存分割后的内容地址
    char *saveptr;
    char * line = strtok_r(text,"\n",&saveptr);

    while (line != NULL){
        char * key ;
        char *value ;
        char *seck = strchr(line,':');
        if (seck !=NULL){
            *seck = '\0';
            key = line ;
            value = seck +1;
            
            while(*value == ' ' || *value == '\t'){
                value ++;
            }
            //将end 指向key 的最后一个字符
            // 如果key 的值后面没有空格或空白字符，*end 就变成指向key 的最后一个字符，
            // 类似key[strlen(key) - 1]
            char * end = key + strlen(key) - 1 ;
            
            while(end > key && *end == ' '|| *end == '\t' ){
                *end = '\0';
                end -- ;
            }
            int ret = callback(key,value,data);
            if (ret < 0){
                *seck = ':';
                return -1 ;
            }


        }
        *seck = ':';
        line = strtok_r(NULL,"\n",&saveptr);

    }

    return 0;
}



int  main (){

    int fd = open("/proc/self/status", O_RDONLY);

    if(fd < 0) {
        printf("open err " );
        return -1 ;
    }
    char buf[2048];
    ssize_t n = read(fd,buf,sizeof(buf));
    if (n < 0){
        printf("read err ");
        return -1;

    }
    buf[n] = '\0';
    //printf("read %s \n",buf);
    close(fd);
    printf("++++++");
    int count =0;

    int ret = parsConfig(buf,countF,&count);
    if (ret < 0){
        return -1;
    }
    printf("count is %d\n",count);
    return 0;
}