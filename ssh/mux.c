#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <arpa/inet.h>

#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <stdio.h>

#define MUX_MSG_HELLO       0x00000001
#define MUX_C_NEW_STDIO_FWD 0x10000008
#define MUX_S_PERMISSION_DENIED 0x80000002
#define MUX_S_FAILURE       0x80000003
#define MUX_S_SESSION_OPENED    0x80000006

int main(int argc, char **argv)
{
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    int myfds[2] = {0}; /* Contains the file descriptors to pass. */
    char buf[CMSG_SPACE(sizeof myfds)];  /* ancillary data buffer */
    int *fdptr;
    int rc, mux_socket;
    struct iovec iov = {.iov_len = 1};
    struct sockaddr_un addr;
    char buffer[BUFSIZ];
    uint32_t *p, num;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, 100, "%s", argv[1]);

    mux_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( connect(mux_socket, (struct sockaddr *)&addr, sizeof(addr)) ) {
        perror("connect");
        return 1;
    }

    rc = recv(mux_socket, &buffer, BUFSIZ, 0);
    if (rc < 0) {
        perror("recv");
        return 1;
    }

    p = (uint32_t *)buffer;
    num = ntohl(*p);
    printf("Message has %d bytes\n", num);

    if (num < 8) {
        printf("Message too short\n");
        return 1;
    }

    num = ntohl(*++p);
    if (num != MUX_MSG_HELLO) {
        printf("Message not a Hello\n");
        return 1;
    }
    num = ntohl(*++p);
    if (num != 4) {
        printf("Protocol version not 4\n");
        return 1;
    }

    // our hello message is the same, cheat a little :)
    if (rc != send(mux_socket, &buffer, rc, 0)) {
        perror("send");
        return 1;
    }

    // then request the connection
    p = (uint32_t *)buffer;
    p++; // reserve for msglen
    *p++ = htonl(MUX_C_NEW_STDIO_FWD); // msgtype
    *p++ = htonl(1); // request_id
    *p++ = htonl(0); // reserved
    // string host
    num = sprintf((char *)(p+1), "%s", "localhost");
    *p++ = htonl(num); // strlen
    p = (uint32_t *)((char *)p + num);

    /* despite what the documentation says, port is a uint32
    // string port
    num = sprintf((char *)(p+1), "%s", "5555");
    *p++ = htonl(num); // strlen
    p = (uint32_t *)((char *)p + num);
    */
    *p++ = htonl(5555); // port

    num = ((char *)p) - buffer;
    p = (uint32_t *)buffer;
    *p = htonl(num - sizeof(*p)); // msglen

    printf("Message has %d bytes\n", num);
    if (send(mux_socket, buffer, num, 0) != num) {
        perror("send");
        return 1;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, myfds)) {
        perror("socketpair");
        return 1;
    }

    // and send the file descriptor twice
    msg.msg_control = buf;
    msg.msg_controllen = sizeof buf;

    // remember that there has to be some data or the message is silently dropped
    msg.msg_iovlen = 1;
    msg.msg_iov = &iov;

    num = 0;
    iov.iov_base = &num;
    iov.iov_len = 1;

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    /* Initialize the payload: */
    fdptr = (int *) CMSG_DATA(cmsg);
    memcpy(fdptr, &myfds[0], sizeof(int));

    /* Sum of the length of all control messages in the buffer: */
    msg.msg_controllen = cmsg->cmsg_len;

    if (sendmsg(mux_socket, &msg, 0) != 1) {
        perror("sendmsg");
        return 1;
    }

    if (sendmsg(mux_socket, &msg, 0) != 1) {
        perror("sendmsg");
        return 1;
    }

    rc = recv(mux_socket, &buffer, BUFSIZ, 0);
    if (rc <= 0) {
        perror("recv");
        return 1;
    }

    p = (uint32_t *)buffer;
    num = ntohl(*p++);
    printf("Message has %d bytes\n", num);

    if (ntohl(*p) != MUX_S_SESSION_OPENED) {
        printf("Forwarding request failed\n");
        return 1;
    }

    // the socket we sent is now connected

    close(mux_socket);

    while ((rc = recv(myfds[1], &buffer, BUFSIZ, 0))) {
        write(1, &buffer, rc);
        if (send(myfds[1], &buffer, rc, 0) != rc) {
            perror("send");
            return 1;
        }
    }
    printf("Connection finished\n");

    rc = recv(mux_socket, &buffer, BUFSIZ, 0);
    if (rc <= 0) {
        perror("recv");
        return 1;
    }

    p = (uint32_t *)buffer;
    num = ntohl(*p++);
    printf("Message has %d bytes\n", num);

    return 0;
}
