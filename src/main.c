#include <event2/event.h>

#define CLOG_MAIN
#include "clog.h"
#include "net.h"

#define MY_LOGGER 0

struct event_base *base;

int main(int argc, char **argv)
{
    int ret = clog_init_fd(MY_LOGGER, stdout->_fileno);

    base = event_base_new();
    init_listen_scoket();
    event_base_dispatch(base);

    clog_free(MY_LOGGER);
    return 0;
}


