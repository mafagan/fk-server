#include "util.h"
#include "net.h"
#include "http_parse.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>

extern struct event_base *base;

static void read_cb(int sock, short event, void *arg);
static void setnonblock(int fd);
static void do_accept(int sock, short event, void *arg);

void setnonblock(int fd)
{
    int flags;
    flags = fcntl(fd, F_GETFL);

    if (flags < 0) return;

    flags |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) < 0)
        exit(EXIT_FAILURE);

}

void read_cb(int sock, short event, void *arg)
{
    session_t *session = (session_t*)arg;
    int ret = read(sock, session->header,
            HTTP_HEADER_SIZE - session->buffer_cursor);


    printf("recv %d bytes\n", ret);
    if (ret == 0) {
        finalize_session(session);
    } else if (ret < 0) {

        //TODO
    }
    else {
        session->buffer_cursor += ret;


        if (session->buffer_cursor >= HTTP_HEADER_SIZE) {
            //TODO
        }

        do_http_request_parse(session);

    }
}

static void do_accept(int sock, short event, void *arg)
{
    struct sockaddr_in cli_addr;
    int sin_size = sizeof(struct sockaddr_in);
    int newfd = accept(sock, (struct sockaddr*)&cli_addr,
            &sin_size);

    if (newfd < 0) {
        perror("accept fail");
        return;
    }


    session_t *new_session = (session_t*)malloc(sizeof(struct session));
    new_session->buffer_cursor = 0;
    new_session->parse_cursor = 0;
    new_session->parse_status = REQUEST_LINE;
    new_session->sock = newfd;

    struct event *read_ev = event_new(base, newfd, EV_READ|EV_PERSIST,
            read_cb, (void*)new_session);

    if (read_ev == NULL) {
        perror("event_new() fail");
        return;
    }

    /* do not add write_ev here */
    new_session->read_ev = read_ev;
    event_add(read_ev, NULL);

}

void  init_listen_scoket()
{
    assert(base);
    evutil_socket_t listener;
    listener = socket(AF_INET, SOCK_STREAM, 0);

    assert(listener > 0);

    evutil_make_listen_socket_reuseable(listener);
    setnonblock(listener);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(DEFAULT_LISTEN_PORT);

    if(bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if(listen(listener, DEFAULT_LISTEN_BACKLOG) < 0) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }


    struct event *listen_event = event_new(base, listener, EV_READ|EV_PERSIST,
            do_accept, NULL);

    assert(listen_event);

    event_add(listen_event, NULL);
}

void finalize_session(session_t *session)
{
    //TODO destroy session
}
