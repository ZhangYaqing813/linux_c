#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
//定义回调函数
typedef int (*callback) (const char *key, const char *value,  void *data);

//想要获取进程状态的字段
struct proc_info {
    char    name[64];
    int     pid;
    int     vmrss;
    char    state[16];
    // ... 可以继续添加
};


int parse_file(char *text, callback cb, const void *data) {
    if (!text || !cb) {
        printf("parse_file: invalid arguments\n");
        return -1;
    }
    // 这个是用来保存指针地址
    char *savePtr;
    //先把文本按照"\n" 换行进行拆分，
    char *line = strtok_r(text, "\n", &savePtr);
    //逐行进行进行遍历，不为空就继续
    while (line != NULL) {
        //行处理，按照 ： 进行拆分，
        printf("line = %s\n", line);
        char *colon = strchr(line,':');
        if (colon) {
            printf("看一下colon[0] 的值是什么：%s\n",colon);
            colon[0] = '\0';
            printf("=======colon 的值是什么：%s\n",colon);
            printf("处理后的line = %s\n",line);
            char *key = line;

            printf("colon+1 = %s\n",colon+1);
            printf("\n");
            char *value = colon + 1;
            // 如果是空就跳过
            while (value == ' ' || value == '\t') {
                value++;
            }

/*
            *写法	输出内容	安全
            printf("%p", &key)	key 这个指针变量自己的内存地址	✅
            printf("%c", *key)	key 指向的字符串的第一个字符	✅
            printf("%s", key)	key 指向的整个字符串	✅
            printf(key)	和 printf("%s", key) 一样，但遇到 % 会炸	❌
 */


            char *end = key + strlen(key) - 1;//
            //输出一下看看
            printf("end 内容： %s\n",end );
            while (end > key && (*end == ' ' || *end == '\t')) {
                *end = '\0';
                end --;
            }


            int ret = cb(key, value, data);
            if (ret != 0 ) {
                *colon = '\0';
                return -1;
            }
            *colon = '\0';
        }
        line = strtok_r(NULL, "\n", &savePtr);
    }

    return 0;
}


int print_kv(const char *key, const char *value,void *data) {

    (void)data;
    printf("print_kv: %s: %s\n", key, value);
    return 0;

}



int fill_proc_info( const  char *key, const char *value, void *data) {
    //强制类型转换
    struct proc_info *pi = (struct proc_info *)data;

    if (strcmp(key, "Name") == 0) {
        strcpy(pi->name, value);
        pi->name[sizeof(pi->name) - 1] = '\0';
    } else if (strcmp (key,"Pid" ) == 0 ) {
        pi->pid = atoi(value);
    } else if (strcmp (key ,"VmRSS") == 0 ) {
        pi->vmrss = atoi(value);
    } else if (strcmp (key,"State") == 0) {
        strcpy(pi->state,value);
        pi->state[sizeof(pi->state) - 1] = '\0';
    }


    return 0;
}



int main() {

    int fd = open("/proc/self/status", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    char buffer[4096];
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1 );
    if (n <0) {
        perror("read");
        close(fd);
        return 1;
    }
    buffer[n] = '\0';
    close(fd);
/*
    printf("============一种回调演示============\n");
    parse_file(buffer, print_kv, NULL);
    printf("\n");
*/
    printf("============第二种回调演示============\n");

    struct proc_info myproc = {0};
    parse_file(buffer, fill_proc_info, &myproc);
    printf("=== 方式2：提取到结构体 ===\n");
    printf("进程名: %s\n", myproc.name);
    printf("PID:    %d\n", myproc.pid);
    printf("状态:   %s\n", myproc.state);
    printf("VmRSS:  %d kB\n", myproc.vmrss);
    return 0 ;

}