#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <event2/event.h>

#define DEFAULT_LISTEN_PORT 8080
#define DEFAULT_LISTEN_BACKLOG 5

#define HTTP_HEADER_SIZE 4096

#define bool    uint8_t

#define true    0x01
#define false   0x00

#define FREE(x) \
    if (x != NULL) {free(x);}

#define EV_FREE(x) \
    if (x != NULL) {event_free(x);}

unsigned long get_file_size(const char *path);

int start_worker_processes(uint32_t workers, pid_t *workers_pid);

void worker_quit();

#endif
