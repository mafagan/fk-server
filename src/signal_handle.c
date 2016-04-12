#include "signal_handle.h"
#include "util.h"

#include <signal.h>
#include <event2/event.h>
#include <assert.h>

extern struct event_base *base;

static void master_signal_cb(evutil_socket_t, short, void *);
static void worker_signal_cb(evutil_socket_t, short, void *);


void master_sinal_event_init()
{

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

void master_signal_cb(evutil_socket_t signal_socket, short event, void *arg)
{
    //TODO send signal to workers
}

void worker_signal_cb(evutil_socket_t signal_socket, short event, void *arg)
{
    worker_quit();
}
