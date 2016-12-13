#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>

#include <linux/limits.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>

char *
get_executable_name(__kernel_pid_t pid)
{
    char *out, buf[40];
    int len;

    if (!(out = malloc(PATH_MAX + 1))) {
        return NULL;
    }

    sprintf(buf, "/proc/%d/exe", pid);

    len = readlink(buf, out, PATH_MAX);
    assert(len <= PATH_MAX);
    
    if (len < 0) {
        free(out);
        return NULL;
    }
    out[len] = '\0';
    return out;
}

void
print_arguments(__kernel_pid_t pid)
{
    char filename[40], buffer[BUFSIZ], *p = buffer;
    ssize_t len, slen, remaining = 0;
    int fd;

    sprintf(filename, "/proc/%d/cmdline", pid);

    fd = open(filename, O_RDONLY);
    if (fd < 0) return;

    printf("\tArguments:");
    while ( (len = read(fd, p, BUFSIZ - remaining)) > 0 ) {
        remaining += len;
        p = buffer;

        while ( (slen = strnlen(p, remaining)) < remaining ) {
            printf(" '%s'", p);
            p += slen + 1;
            remaining -= slen + 1;
        }

        memmove(buffer, p, remaining);
        p = buffer + remaining;
    }

    printf("\n");
}

static int
process_message(struct nl_msg *msg, void *arg)
{
    struct nlmsghdr *msg_hdr = nlmsg_hdr(msg);
    struct cn_msg *cn_msg = nlmsg_data(msg_hdr);
    struct proc_event *event = (struct proc_event *)cn_msg->data;
    char *executable = get_executable_name(event->event_data.fork.parent_pid);

    switch (event->what) {
        case PROC_EVENT_NONE:
            printf("Dummy event\n");
            break;
        case PROC_EVENT_FORK:
            printf("Process %s(%d) forked: child pid=%d\n",
                    executable,
                    event->event_data.fork.parent_pid,
                    event->event_data.fork.child_pid);
            break;
        case PROC_EVENT_EXEC:
            printf("Process %d changed its program to '%s'\n",
                    event->event_data.exec.process_pid,
                    executable);
            print_arguments(event->event_data.exec.process_pid);
            break;
        case PROC_EVENT_UID:
            printf("Process %s(%d) changed its uid %d -> %d\n",
                    executable,
                    event->event_data.id.process_pid,
                    event->event_data.id.r.ruid,
                    event->event_data.id.e.euid);
        case PROC_EVENT_GID:
            printf("Process %s(%d) changed its gid %d -> %d\n",
                    executable,
                    event->event_data.id.process_pid,
                    event->event_data.id.r.rgid,
                    event->event_data.id.e.egid);
            break;
        case PROC_EVENT_SID:
            printf("Process %s(%d) changed its session\n",
                    executable,
                    event->event_data.sid.process_pid);
            break;
        case PROC_EVENT_PTRACE:
            {
                char *tracer_name = get_executable_name(event->event_data.ptrace.tracer_pid);
                printf("Process %s(%d) is being traced by %s(%d)\n",
                        executable,
                        event->event_data.ptrace.process_pid,
                        tracer_name,
                        event->event_data.ptrace.tracer_pid);
                free(tracer_name);
            }
            break;
        case PROC_EVENT_COMM:
            printf("Process %s(%d) changed its name, new name is '%s'\n",
                    executable,
                    event->event_data.comm.process_pid,
                    event->event_data.comm.comm);
            break;
        case PROC_EVENT_EXIT:
            printf("Process %d stopped with exit code/signal %d/%d\n",
                    event->event_data.exit.process_pid,
                    event->event_data.exit.exit_code,
                    event->event_data.exit.exit_signal);
            break;
        default:
            printf("Unknown event %d\n", event->what);
            break;
    }

    free(executable);
    return NL_OK;
}

int
set_up_subscriptions(struct nl_sock *s, int enable)
{
    struct cn_msg *msg;
    size_t msg_len = sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op);
    
    if ((msg = calloc(1, msg_len)) == NULL) {
        perror("calloc");
        return 1;
    }

    msg->id.idx = CN_IDX_PROC;
    msg->id.val = CN_VAL_PROC;
    msg->len = sizeof(enum proc_cn_mcast_op);
    *msg->data = PROC_CN_MCAST_LISTEN;

    if (nl_send_simple(s, NLMSG_DONE, 0, msg, msg_len) < 0) {
        perror("nl_send_simple");
        return 1;
    }
    free(msg);

    return 0;
}

int main()
{
    struct nl_sock *s;
    int rc;

    s = nl_socket_alloc();
    if (!s) {
        perror("nl_socket_alloc");
        return 1;
    }

    if (nl_socket_modify_cb(s, NL_CB_FINISH, NL_CB_CUSTOM, process_message, NULL)) {
        perror("nl_socket_modify_cb");
        return 1;
    }

    /*
    if (nl_socket_modify_cb(s, NL_CB_OVERRUN, NL_CB_CUSTOM, process_message, "an invalid")) {
        perror("nl_socket_modify_cb");
        return 1;
    }
    */

    nl_socket_disable_seq_check(s);

    if (nl_connect(s, NETLINK_CONNECTOR)) {
        perror("nl_connect");
        return 1;
    }

    if ((rc = nl_socket_add_memberships(s, CN_IDX_PROC, 0))) {
        perror("nl_socket_add_memberships");
        return 1;
    }

    if ((rc = set_up_subscriptions(s, 1))) {
        perror("set_up_subscriptions");
        return 1;
    }

    while (1) {
        if ((rc = nl_recvmsgs_default(s))) {
            perror("nl_recvmsgs_default");
            break;
        }
    }

    nl_close(s);
    nl_socket_free(s);
}
