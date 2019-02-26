#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int s, rc;
	uint16_t port = 0;

	if ( argc > 1 ) {
		port = atoi( argv[1] );
	}

	s = socket(AF_INET, SOCK_STREAM, 0);
	if ( s < 0 ) {
		perror("socket");
		exit(1);
	}

	memset( &addr, 0, sizeof(addr) );

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if ( bind( s, (struct sockaddr *)&addr, addrlen ) ) {
		perror("bind");
		exit(1);
	}

	if ( getsockname( s, (struct sockaddr *)&addr, &addrlen ) ) {
		perror("getsockname");
		exit(1);
	}

	printf( "Asked for port %" PRIu16 ", got port %" PRIu16 "\n", port, ntohs(addr.sin_port) );

	close( s );
}
