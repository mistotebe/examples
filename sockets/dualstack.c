#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv)
{
    struct sockaddr_in6 sin;
    int sock, conn, option = 0;

    memset(&sin, 0, sizeof(sin));
    sin.sin6_family = AF_INET6;
    sin.sin6_addr = in6addr_any;
    sin.sin6_port = htons(8080);


    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 1;
    }

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option))) {
        perror("setsockopt");
        return 1;
    }

    if (bind(sock, (struct sockaddr *)&sin, sizeof(sin))) {
        perror("bind");
        return 1;
    }

    if (listen(sock, 16)) {
        perror("listen");
        return 1;
    }

    while ((conn = accept(sock, NULL, NULL)) >= 0) {
        printf("A connection!\n");
        close(conn);
    }

    close(sock);

    return 0;
}
