#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXN 505
#define PORT 8080
#define SA struct sockaddr

void go(int sockfd) {
    char buf[MAXN];
    int i;
    while (1) {
        bzero(buf, sizeof buf);
        printf("Enter string: ");
        scanf("%s", buf);
        write(sockfd, buf, sizeof buf);
        if (strcmp(buf, "quit") == 0) {
            printf("Client quit!\n");
            break;
        }
        bzero(buf, sizeof buf);
        read(sockfd, buf, sizeof buf);
        printf("From server: %s\n", buf);
    }
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket successfully created..\n");
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        printf("connected to the server..\n");
    }

    go(sockfd);

    close(sockfd);
}