#ifndef _NET_H
#define _NET_H

#include <event2/event.h>
#include <stdint.h>

#include "util.h"
#include "http_parse.h"


typedef struct session {
    int sock;

    struct event *read_ev;
    struct event *write_ev;

    char header[HTTP_HEADER_SIZE];
    uint32_t buffer_cursor;
    uint32_t parse_cursor;
    uint32_t pre_parse_cursor;

    uint16_t is_request_valid;

    request_parse_status_t parse_status;

    http_request_t request;
    http_response_t response;
} session_t;

void init_listen_scoket();
void finalize_session(session_t *session);

#endif
