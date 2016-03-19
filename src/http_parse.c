#include "http_parse.h"
#include "net.h"

#include <string.h>
#include <unistd.h>

static void do_http_request_line_parse(struct session*);
static void do_http_request_header_parse(struct session*);
static void do_response(struct session*);



void do_response(struct session *session)
{
    char response_buffer[HTTP_RESPONSE_BUF_SIZE];
    uint32_t write_cursor = 0;

    //TODO execute request


    /****************
     * Test code    *
     ****************/
    int ret;
    ret = snprintf(response_buffer + write_cursor,
            HTTP_RESPONSE_BUF_SIZE - write_cursor - 1,
            "%s %d %s\n", "HTTP/1.1", HTTP_OK, "OK");
    write_cursor += ret;

    ret = snprintf(response_buffer + write_cursor,
            HTTP_RESPONSE_BUF_SIZE - write_cursor - 1,
            "server: feike\n");
    write_cursor += ret;



    char file_content[8192];
    char file_path[256];

    snprintf(file_path, 256, HTTP_ROOT_PATH);
    strcat(file_path, session->request.uri);

    FILE *stream = fopen(file_path, "r");
    int num_read = fread(file_content, sizeof(char), 8192, stream);

    fclose(stream);

    ret = snprintf(response_buffer + write_cursor,
            HTTP_RESPONSE_BUF_SIZE - write_cursor - 1,
            "Content-Length: %d\n", num_read);

    write_cursor += ret;

    ret = snprintf(response_buffer + write_cursor,
            HTTP_RESPONSE_BUF_SIZE - write_cursor - 1,
            "Content-Type: text/html\n");

    write_cursor += ret;
    response_buffer[write_cursor++] = '\n';
    response_buffer[write_cursor] = '\0';

    strcat(response_buffer, file_content);

    ret = write(session->sock, response_buffer, write_cursor + num_read);

    if (ret < 0) return;
}

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

    session->request.uri = header + tmpCursor;

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
    if (session->parse_cursor - session->pre_parse_cursor == 1) {
        session->pre_parse_cursor = session->parse_cursor;
        session->parse_status = REQUEST_END;
    } else {
        session->pre_parse_cursor = session->parse_cursor;
    }
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

                    if (session->parse_status == REQUEST_END) break;

                } else {
                    session->parse_cursor ++;
                }
            }


        case REQUEST_DATA:
            //TODO
            break;

        case REQUEST_END:
            do_response(session);
            break;
        default:

            break;

    }
    return;
}
