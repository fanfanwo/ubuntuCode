//udp服务器的实现
#include <stdio.h> //printf
#include <stdlib.h> //exit
#include <sys/types.h>
#include <sys/socket.h> //socket
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h> //htons inet_addr
#include <unistd.h> //close
#include <string.h>

typedef struct{
    char recv_ip[16];
    short recv_port;
    char text[128];
}MSG;

int main(int argc, char const *argv[])
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int sockfd; //文件描述符
    struct sockaddr_in serveraddr; //服务器网络信息结构体
    socklen_t addrlen = sizeof(serveraddr);

    //第一步：创建套接字
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("fail to socket");
        exit(1);
    }

    //第二步：填充服务器网络信息结构体
    //inet_addr：将点分十进制字符串ip地址转化为整形数据
    //htons：将主机字节序转化为网络字节序
    //atoi：将数字型字符串转化为整形数据
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

    //第三步：将套接字与服务器网络信息结构体绑定
    if(bind(sockfd, (struct sockaddr *)&serveraddr, addrlen) < 0)
    {
        perror("fail to bind");
        exit(1);
    }

    MSG msg;
    while(1)
    {
        //第四步：进行通信
        struct sockaddr_in clientaddr;
        if(recvfrom(sockfd, &msg, sizeof(MSG), 0, (struct sockaddr *)&clientaddr, &addrlen) < 0)
        {
            perror("fail to recvfrom");
            exit(1);
        }

        printf("send:%s - %d, recv:%s - %d, text:%s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), msg.recv_ip, msg.recv_port, msg.text);

        //根据服务器接收到的数据指定用户并给他发送数据
        struct sockaddr_in recvaddr;
        recvaddr.sin_family = AF_INET;
        recvaddr.sin_addr.s_addr = inet_addr(msg.recv_ip);
        recvaddr.sin_port = htons(msg.recv_port);

        strcpy(msg.recv_ip, inet_ntoa(clientaddr.sin_addr));
        msg.recv_port = ntohs(clientaddr.sin_port);

        if(sendto(sockfd, &msg, sizeof(MSG), 0, (struct sockaddr *)&recvaddr, addrlen) < 0)
        {
            perror("fail to sendto");
        }
    }

    return 0;
}
