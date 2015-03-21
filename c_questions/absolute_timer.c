#include <sys/timerfd.h>
#include <sys/time.h>

/* for uint64_t */
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>

int main()
{
    struct itimerspec ts;
    struct timeval tv;
    uint64_t expirations;
    time_t seconds;
    int timerfd;

    printf("Getting a realtime timer\n");
    if ((timerfd = timerfd_create(CLOCK_REALTIME, 0)) == -1) {
        perror("timerfd_create");
        return 1;
    }

    printf("Getting the current time:\n");
    if (gettimeofday(&tv, NULL)) {
        perror("gettimeofday");
        return 1;
    }
    printf("%ld.%06ld\n", tv.tv_sec, tv.tv_usec);

    /* Every minute, on the minute */
    ts.it_interval.tv_sec = 60;
    ts.it_interval.tv_nsec = 0;

    ts.it_value.tv_nsec = 0;

    seconds = tv.tv_sec;
    seconds -= seconds % 60;
    seconds += 60;
    ts.it_value.tv_sec = seconds;

    /* or we could just not care and trigger the first read immediately, then
     * on the minute */
    ts.it_value.tv_sec = 60;

    printf("Setting the timer for the next full minute (%ld)\n", ts.it_value.tv_sec);
    if (timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &ts, NULL)) {
        perror("timerfd_settime");
        return 1;
    }

    printf("Preparing to sleep\n");
    while (read(timerfd, &expirations, sizeof(expirations))) {
        printf("Timer expired %lu times, getting the current time again: ", expirations);
        if (gettimeofday(&tv, NULL)) {
            perror("gettimeofday");
            return 1;
        }

        printf("%ld.%06ld\n", tv.tv_sec, tv.tv_usec);
    }

    close(timerfd);
    return 0;
}
