#include "util.h"
#include "clog.h"

#include <sys/stat.h>
#include <unistd.h>

extern struct event_base *base;
extern void free_net();

unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;

    struct stat statbuff;

    if (stat(path, &statbuff) < 0) return filesize;
    else return statbuff.st_size;
}


int start_worker_processes(uint32_t workers, pid_t *workers_pid)
{

    int i;

    for (i = 0; i < workers; i++) {
        pid_t pid = fork();

        switch(pid) {
        case -1:
            log_error("failed to fork()");
            return i;
        case 0:
            //TODO
            return 0;

        default:
            workers_pid[i] = pid;
            break;
        }

    }

    return i;
}

void worker_quit()
{

    free_net();

    event_base_loopbreak(base);
    event_base_free(base);
}
