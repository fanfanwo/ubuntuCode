//udp客户端的实现
#include <stdio.h> //printf
#include <stdlib.h> //exit
#include <sys/types.h>
#include <sys/socket.h> //socket
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h> //htons inet_addr
#include <unistd.h> //close
#include <string.h>
#include <pthread.h>

typedef struct{
    char recv_ip[16];
    short recv_port;
    char text[128];
}MSG;

int sockfd; //文件描述符
struct sockaddr_in serveraddr; //服务器网络信息结构体
socklen_t addrlen = sizeof(serveraddr);

void *recv_fun(void *arg)
{
    MSG msg;

    while(1)
    {
        if(recvfrom(sockfd, &msg, sizeof(MSG), 0, (struct sockaddr *)&serveraddr, &addrlen) < 0)
        {
            perror("fail to recvfrom");
            exit(1);
        }

        printf("[%s - %d]: %s\n", msg.recv_ip, msg.recv_port, msg.text);
    }
}

int main(int argc, char const *argv[])
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    //第一步：创建套接字
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("fail to socket");
        exit(1);
    }

    //客户端自己指定自己的ip地址和端口号
    struct sockaddr_in clientaddr;
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr = inet_addr(argv[3]); //客户端的ip地址
    clientaddr.sin_port = htons(atoi(argv[4])); //客户端的端口号
    if(bind(sockfd, (struct sockaddr *)&clientaddr, addrlen) < 0)
    {
        perror("fail to bind");
        exit(1);
    }

    //第二步：填充服务器网络信息结构体
    //inet_addr：将点分十进制字符串ip地址转化为整形数据
    //htons：将主机字节序转化为网络字节序
    //atoi：将数字型字符串转化为整形数据
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

    //创建一个子线程接收服务器发送的数据
    pthread_t thread;
    if(pthread_create(&thread, NULL, recv_fun, NULL) != 0)
    {
        perror("fail to pthread_create");
        exit(1);
    }
    pthread_detach(thread);

    //主控线程负责发送数据
    char buf[128] = "";
    char s[32] = "";
    MSG msg;
    printf("*** 请先指定要发送数据的用户：sayto ip port ***\n");
    while(1)
    {
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf) - 1] = '\0';

        //如果输入的是sayto开头的字符串，说明要指定跟谁聊天
        //sayto 192.168.1.123 9999
        if(strncmp(buf, "sayto", 5) == 0)
        {
            sscanf(buf, "sayto %s %s", msg.recv_ip, s);
            msg.recv_port = atoi(s);
            printf("%s --> %d\n", msg.recv_ip, msg.recv_port);
        }
        else
        {
            //如果输入的不是sayto开头的字符串，说明要给之前指定好的用户进行通信
            strcpy(msg.text, buf);

            if(sendto(sockfd, &msg, sizeof(MSG), 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
            {
                perror("fail to sendto");
                exit(1);
            }
        }
    }

    return 0;
}