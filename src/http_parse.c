#include "http_parse.h"
#include "net.h"

#include <string.h>

static void do_http_request_line_parse(struct session*);
static void do_http_request_header_parse(struct session*);


void do_http_request_line_parse(struct session *session)
{
    char *header = session->header;
    uint32_t parse_cursor = session->parse_cursor;
    uint32_t pre_parse_cursor = session->pre_parse_cursor;
    uint32_t tmpCursor = session->pre_parse_cursor;

    while (tmpCursor < session->parse_cursor
            && session->header[tmpCursor++] != ' ') ;

    uint32_t method_str_length = tmpCursor - pre_parse_cursor;

    if (strncmp("GET", header + pre_parse_cursor, method_str_length) == 0) {
        session->request.request_method = HTTP_GET;
    } else {
        //TODO
    }

    session->request.reuqest_path = header + tmpCursor;

    while (tmpCursor < session->parse_cursor) {
        if (header[tmpCursor] == ' ') {
            tmpCursor ++;
            break;
        }

        tmpCursor ++;
    }

    if (tmpCursor == parse_cursor) {
        //TODO
    }

    if (parse_cursor - tmpCursor != HTTP_PROTOCOL_LEN + 1) {
        //TODO
    }

    if (strncmp(header + tmpCursor, "HTTP/0.9", 8) == 0)
        session->request.request_protocal = HTTP_0_9;
    else if (strncmp(header + tmpCursor, "HTTP/1.0", 8) == 0)
        session->request.request_protocal = HTTP_1_0;
    else if (strncmp(header + tmpCursor, "HTTP/1.1", 8) == 0)
        session->request.request_protocal = HTTP_1_1;
    else {
        //TODO
    }

}

void do_http_request_header_parse(struct session *session)
{

}

void do_http_request_parse(struct session* session)
{
    http_request_t *request = &(session->request);

    switch(session->parse_status) {
        case REQUEST_LINE:

            while (session->parse_cursor < session->buffer_cursor) {
                if (session->header[session->parse_cursor] == '\n') {
                    session->parse_cursor ++;

                    do_http_request_line_parse(session);

                    session->pre_parse_cursor = session->parse_cursor;
                    session->parse_status = REQUEST_HEADER;
                    break;
                } else {
                    session->parse_cursor ++;
                }
            }



        case REQUEST_HEADER:

            while (session->parse_cursor < session->buffer_cursor) {
                if (session->header[session->parse_cursor] == '\n') {
                    session->parse_cursor ++;

                    /* header ending check will be performed */
                    do_http_request_header_parse(session);
                } else {
                    session->parse_cursor ++;
                }
            }


        case REQUEST_DATA:
            //TODO
            break;

        default:

            break;

    }
    return;
}
