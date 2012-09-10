#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int openConnection(char* hostname, int port) {
    int sd = socket(PF_INET, SOCK_STREAM, 0); 
    if (sd < 0) {
        perror("socket");
        return -1; 
    }

    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);

    struct hostent *hptr;
    hptr = gethostbyname(hostname);
    if (hptr == NULL) {
        perror("Error resolving hostname");
        return -1;
    }
    memcpy( &addr.sin_addr, hptr->h_addr, hptr->h_length);

    if (connect(sd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Failed to establish connection");
        return -1;
    }

    return sd;
}

int main(int argc, char **argv) {
    int port, amount, i;
    char *hostname;
    int *socketArray;
    
    if ( argc != 4 ) {
        fprintf(stderr, "%s: invalid parameters\n", argv[0]);
        return 1;
    }

    amount = atoi(argv[1]);
    port = atoi(argv[3]);

    socketArray = malloc( amount * sizeof(int) );

    for ( i = 0; i < amount; i++ ) {
      printf( "\rOpening connection %d", i );
      socketArray[i] = openConnection(argv[2], port);
      if (socketArray[i] < 0)
        break;
    }

    printf( "\rAll %d connections are open", amount );
    getchar();

    for ( ; i ; )
      if ( close(socketArray[--i]) < 0 )
        perror("close");

    return 0;
}
