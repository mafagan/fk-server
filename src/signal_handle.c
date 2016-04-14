#include "signal_handle.h"
#include "util.h"
#include "clog.h"

#include <signal.h>
#include <event2/event.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>

extern struct event_base *base;
extern pid_t workers_pid[100];
extern int workers;

static void master_signal_cb(int);
static void worker_signal_cb(evutil_socket_t, short, void *);


void master_sinal_event_init()
{
    signal(SIGINT, master_signal_cb);
}

void worker_sinal_event_init()
{
    assert(base);
    struct event *signal_event;

    signal_event = evsignal_new(base, SIGINT, worker_signal_cb, NULL);
    event_add(signal_event, NULL);

    signal_event = evsignal_new(base, SIGHUP, worker_signal_cb, NULL);
    event_add(signal_event, NULL);

    signal_event = evsignal_new(base, SIGQUIT, worker_signal_cb, NULL);
    event_add(signal_event, NULL);

    signal_event = evsignal_new(base, SIGTERM, worker_signal_cb, NULL);
    event_add(signal_event, NULL);

}

void master_signal_cb(int signum)
{
    if (signum == SIGINT) {
        log_debug("master recv signal SIGINT, killing workers");

        int i;

        for (i = 0; i < workers; i++) {
            kill(workers_pid[i], SIGINT);
        }

    }
}

void worker_signal_cb(evutil_socket_t signal_socket, short event, void *arg)
{
    worker_quit();
}
