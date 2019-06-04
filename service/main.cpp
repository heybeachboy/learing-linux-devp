/**
  *主要用于展示网络IO子进程处理模型
  *
  */
#include <netinet/in.h>     // for sockaddr_in
#include <sys/socket.h>     // for socket functions
#include <sys/errno.h>      // for errno
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX_LIMIT 16384
#define BACKLOG 33
/**
 ×rot13交换加密
 * @brief encode_rot13
 * @param c
 * @return
 */
char encode_rot13(char c){
    if((c >= 'a' && c <= 'm') ||(c >= 'A' && c <= 'M')){
        return c+13;
    }
    if((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z')){
        return c-13;
    }
    return c;
}
/**
 *子进程IO操作处理具体业务逻辑
 * @brief child_process
 * @param fd
 */
void child_process(int fd){
    char outbuf[MAX_LIMIT + 1];  // extra byte for '\0'
    size_t outbuf_used = 0;
    ssize_t code;

    while(1)
    {
        char ch;
        code = recv(fd, &ch, 1, 0);

        if(code <=0 ){
            perror("read info error");
            break;
        }


        if(outbuf_used < sizeof(outbuf))
        {
            outbuf[outbuf_used++] = encode_rot13(ch);
        }

        if(ch == '\n')
        {
            send(fd, outbuf, outbuf_used, 0);
            outbuf_used = 0;
            continue;
        }
    }
}
/**
 *@初始化和绑定socket
 * @brief init_network_socket
 */
void init_network_socket(void)
{
    int fd;
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(40713);

    fd = socket(AF_INET, SOCK_STREAM, 0);

    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if(bind(fd, (struct sockaddr *)(&sin), sizeof(sin)) < 0){
        perror("bind socket failed");
        return;
    }

    if(listen(fd, BACKLOG) < 0){
        perror("listen socket failed");
        return;
    }

    while(true)
    {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);
        int client_fd = accept(fd, (struct sockaddr *)(&ss), &slen);
        if(fd < 0){
            perror("accept client exception");
            continue;
        }
        if(fork() == 0){
            child_process(client_fd);
        }

    }
}

int main(int argc, char ** argv)
{
    init_network_socket();
    return 0;
}
