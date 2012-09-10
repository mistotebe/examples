#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * Will fork furiously until it can acquire the specified pid
 * Code not mine, but taken from somewhere on the 'netz.
 */

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Specify the desired PID on the command-line\n");
        return 1;
    }
    pid_t wanted = (pid_t)atoi(argv[1]);
    int rc;
    int status;
    while((rc = fork()) != wanted && rc != 0)
    {
        if(rc == -1)
        {
            printf("fork error\n");
            return 1;
        }
        if(wait(&status) == -1)
            printf("wait error\n");
    }
    if(rc == 0)
    {
        if(getpid() == wanted)
        {
            printf("Got pid %d, sleeping\n", wanted);
            sleep(200);
        }
        return 0;
    }
    wait(&status);
    return 0;
}
