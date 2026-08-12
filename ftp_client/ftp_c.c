#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define BUF_SIZE 4096
#define RESPONSE_SIZE 1024

// 全局控制连接套接字
int ctrl_sock = -1;

// 发送 FTP 命令并读取响应（仅读取第一行）
// 返回响应码（如 220、331 等）
int send_command(const char *cmd, char *response, size_t resp_len) {
    if (send(ctrl_sock, cmd, strlen(cmd), 0) == -1) {
        perror("send");
        return -1;
    }
    // 读取响应行
    int n = recv(ctrl_sock, response, resp_len - 1, 0);
    if (n <= 0) {
        perror("recv");
        return -1;
    }
    response[n] = '\0';
    // 响应码是前三个字符
    int code = atoi(response);
    // 调试输出（可选）
    // printf("[DEBUG] %s", response);
    return code;
}

// 读取多行响应直到遇到以数字开头且后跟空格的行（表示最后一行）
// 简化版本：直接读取一行即可满足大多数命令
// 但对于 PASV 返回的 227 只有一行，没问题。
int send_command_simple(const char *cmd, char *response, size_t resp_len) {
    return send_command(cmd, response, resp_len);
}

// 登录 FTP 服务器
int ftp_login(const char *user, const char *pass) {
    char resp[RESPONSE_SIZE];
    int code;

    // 等待服务器欢迎消息
    if ((code = send_command("", resp, sizeof(resp))) == -1) return -1;
    if (code != 220) {
        fprintf(stderr, "Unexpected welcome code: %d\n", code);
        return -1;
    }

    // 发送 USER
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "USER %s\r\n", user);
    if ((code = send_command(cmd, resp, sizeof(resp))) == -1) return -1;
    if (code != 331 && code != 230) {
        fprintf(stderr, "USER failed with code: %d\n", code);
        return -1;
    }

    // 如果已经登录成功（230），跳过密码
    if (code == 331) {
        snprintf(cmd, sizeof(cmd), "PASS %s\r\n", pass);
        if ((code = send_command(cmd, resp, sizeof(resp))) == -1) return -1;
        if (code != 230) {
            fprintf(stderr, "PASS failed with code: %d\n", code);
            return -1;
        }
    }

    printf("Logged in successfully.\n");
    return 0;
}

// 设置二进制传输模式
int ftp_type_binary() {
    char resp[RESPONSE_SIZE];
    int code = send_command("TYPE I\r\n", resp, sizeof(resp));
    if (code != 200) {
        fprintf(stderr, "TYPE I failed with code: %d\n", code);
        return -1;
    }
    return 0;
}

// 解析 PASV 响应 "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)"
// 返回数据服务器地址和端口
int parse_pasv_response(const char *resp, char *data_host, int *data_port) {
    unsigned int h1, h2, h3, h4, p1, p2;
    // 扫描格式：227 ... (h1,h2,h3,h4,p1,p2)
    if (sscanf(resp, "%*d %*[^(](%u,%u,%u,%u,%u,%u)", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        fprintf(stderr, "Failed to parse PASV response: %s\n", resp);
        return -1;
    }
    sprintf(data_host, "%u.%u.%u.%u", h1, h2, h3, h4);
    *data_port = p1 * 256 + p2;
    return 0;
}

// 进入被动模式，返回新建立的数据连接套接字
int ftp_pasv(int *data_sock, char *data_host, int *data_port) {
    char resp[RESPONSE_SIZE];
    int code = send_command("PASV\r\n", resp, sizeof(resp));
    if (code != 227) {
        fprintf(stderr, "PASV failed with code: %d\n", code);
        return -1;
    }
    if (parse_pasv_response(resp, data_host, data_port) != 0) return -1;

    // 创建数据连接 socket
    *data_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*data_sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(*data_port);
    if (inet_pton(AF_INET, data_host, &data_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(*data_sock);
        return -1;
    }

    if (connect(*data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) == -1) {
        perror("connect data socket");
        close(*data_sock);
        return -1;
    }

    return 0;
}

// 下载文件
int ftp_download(const char *remote_file, const char *local_file) {
    char data_host[64];
    int data_port, data_sock;

    // 1. 进入被动模式，建立数据连接
    if (ftp_pasv(&data_sock, data_host, &data_port) != 0) {
        return -1;
    }

    // 2. 发送 RETR 命令
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "RETR %s\r\n", remote_file);
    char resp[RESPONSE_SIZE];
    int code = send_command(cmd, resp, sizeof(resp));
    if (code != 150 && code != 125) {
        fprintf(stderr, "RETR failed with code: %d\n", code);
        close(data_sock);
        return -1;
    }

    // 3. 从数据连接接收数据并写入本地文件
    FILE *fp = fopen(local_file, "wb");
    if (fp == NULL) {
        perror("fopen");
        close(data_sock);
        return -1;
    }

    char buf[BUF_SIZE];
    int n;
    while ((n = recv(data_sock, buf, sizeof(buf), 0)) > 0) {
        fwrite(buf, 1, n, fp);
    }
    if (n < 0) {
        perror("recv data");
        fclose(fp);
        close(data_sock);
        return -1;
    }

    fclose(fp);
    close(data_sock);

    // 4. 读取数据连接关闭后的完成响应（226 或 250）
    if ((code = send_command("", resp, sizeof(resp))) == -1) return -1;
    if (code != 226 && code != 250) {
        fprintf(stderr, "Transfer completion code expected 226/250, got %d\n", code);
        return -1;
    }

    printf("File downloaded successfully: %s\n", local_file);
    return 0;
}

// 关闭控制连接
void ftp_quit() {
    char resp[RESPONSE_SIZE];
    send_command("QUIT\r\n", resp, sizeof(resp));
    if (ctrl_sock != -1) close(ctrl_sock);
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <server> <port> <user> <pass> <remote_file> [local_file]\n", argv[0]);
        fprintf(stderr, "Example: %s ftp.example.com 21 anonymous pass@ /pub/file.txt ./file.txt\n", argv[0]);
        return 1;
    }

    const char *server = argv[1];
    int port = atoi(argv[2]);
    const char *user = argv[3];
    const char *pass = argv[4];
    const char *remote_file = argv[5];
    const char *local_file = (argc == 7) ? argv[6] : strrchr(remote_file, '/') ? strrchr(remote_file, '/') + 1 : remote_file;

    // 解析服务器地址
    struct hostent *he = gethostbyname(server);
    if (he == NULL) {
        herror("gethostbyname");
        return 1;
    }

    // 创建控制连接
    ctrl_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ctrl_sock == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(ctrl_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(ctrl_sock);
        return 1;
    }

    printf("Connected to %s:%d\n", server, port);

    // 登录
    if (ftp_login(user, pass) != 0) {
        ftp_quit();
        return 1;
    }

    // 设置二进制模式
    if (ftp_type_binary() != 0) {
        ftp_quit();
        return 1;
    }

    // 下载文件
    if (ftp_download(remote_file, local_file) != 0) {
        ftp_quit();
        return 1;
    }

    ftp_quit();
    printf("Done.\n");
    return 0;
}