#include <event2/event.h>

#include "net.h"

struct event_base *base;
int main(int argc, char **argv)
{
    base = event_base_new();
    init_listen_scoket();
    event_base_dispatch(base);
    return 0;
}


