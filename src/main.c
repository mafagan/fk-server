#include <event2/event.h>
#include <event2/util.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#define CLOG_MAIN
#include "clog.h"
#include "net.h"
#include "util.h"
#include "signal_handle.h"

#define MY_LOGGER 0

struct event_base *base;
pid_t workers_pid[100];
int workers;

int main(int argc, char **argv)
{
    int ret = clog_init_fd(MY_LOGGER, stdout->_fileno);
    assert(ret == 0);
    /* O_APPEND is set */
    //clog_init_path(MY_LOGGER, "feike.log");
    workers = sysconf(_SC_NPROCESSORS_CONF);
    pid_t master_pid = getpid();
    log_debug("master %d begin to work", master_pid);
    log_debug("detected %d cpu cores", workers);



    base = event_base_new();
    evutil_socket_t listener = create_listen_scoket();
    log_debug("get listen socket %d", listener);

    /* create worker process */

    int workers_success_num = start_worker_processes(workers, workers_pid);

    pid_t verify_pid = getpid();

    if (verify_pid != master_pid) {
        /* This is a worker process */
        event_reinit(base);

        log_debug("worker %d begin to work", verify_pid);

        eventadd_listen_socket(listener);

        worker_signal_event_init();

        event_base_dispatch(base);

        log_debug("worker %d quit", verify_pid);
        clog_free(MY_LOGGER);
        event_base_free(base);

    } else {
        /* This is a master process */
        if (workers_success_num < workers) {
            log_error("failed to create workers, success num %d",
                    workers_success_num);
            exit(EXIT_FAILURE);
        }

        log_debug("master %d continue to work", master_pid);
        close(listener);
        event_base_free(base);
        master_signal_event_init();
        pause();

        int i;

        for (i = 0; i < workers_success_num; i++) {
            waitpid(workers_pid[i], NULL, WUNTRACED);
        }
    }

    return 0;
}


