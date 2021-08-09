#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define MAXN 505
#define PORT 8080
#define SA struct sockaddr

void rev(char* str) {
    if (str) {
        char* end = str + strlen(str) - 1;
        char c;
        while (str < end) {
            c = *str; *str = *end; *end = c;
            str++;
            end--;
        }
    }
}

void go(int sockfd) {
    char buf[MAXN];
    int n;
    while (1) {
        bzero(buf, sizeof buf);
        read(sockfd, buf, sizeof buf);
        if (strcmp("quit", buf) == 0) {
            printf("Terminate server!\n");
            break;
        }
        rev(buf);
        write(sockfd, buf, sizeof buf);
    }
}

int main() {
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket successfully created..\n");
    }
    bzero(&servaddr, sizeof servaddr);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else {
        printf("Socket successfully binded..\n");
    }

    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening..\n");
    }

    len = sizeof(cli);

    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    } else {
        printf("server acccept the client...\n");
    }

    go(connfd);
    close(sockfd);
}