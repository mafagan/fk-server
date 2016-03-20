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
            "%s %d %s\r\n", "HTTP/1.1", HTTP_OK, "OK");
    write_cursor += ret;

    ret = snprintf(response_buffer + write_cursor,
            HTTP_RESPONSE_BUF_SIZE - write_cursor - 1,
            "server: feike\r\n");
    write_cursor += ret;



    char file_content[81920];
    char file_path[256];
    uint32_t path_cursor = 0;

    ret = snprintf(file_path, 256, HTTP_ROOT_PATH);
    path_cursor += ret;

    ret = sscanf(session->request.uri, "%s", file_path + path_cursor);

    uint32_t len = strlen(file_path);

    if (file_path[len-1] == '/')
        strcat(file_path, "index.html");


    //strcat(file_path, session->request.uri);


    FILE *stream = fopen(file_path, "r");
    int num_read = fread(file_content, sizeof(char), 81920, stream);

    fclose(stream);

    ret = snprintf(response_buffer + write_cursor,
            HTTP_RESPONSE_BUF_SIZE - write_cursor - 1,
            "Content-Length: %d\r\n", num_read);

    write_cursor += ret;

    ret = snprintf(response_buffer + write_cursor,
            HTTP_RESPONSE_BUF_SIZE - write_cursor - 1,
            "Content-Type: text/html\r\n");

    write_cursor += ret;
    response_buffer[write_cursor++] = '\r';
    response_buffer[write_cursor++] = '\n';
    response_buffer[write_cursor] = '\0';

    strcat(response_buffer, file_content);

    ret = write(session->sock, response_buffer, write_cursor + num_read);

    close(session->sock);
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
    if (session->parse_cursor - session->pre_parse_cursor == 2) {
        session->pre_parse_cursor = session->parse_cursor;
        session->parse_status = REQUEST_END;
    } else {
        session->pre_parse_cursor = session->parse_cursor;
    }
}

void do_http_request_parse(struct session* session)
{
    http_request_t *request = &(session->request);

    if (session->parse_status ==  REQUEST_LINE) {
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
    }


    if (session->parse_status == REQUEST_HEADER) {

        while (session->parse_cursor < session->buffer_cursor) {
            if (session->header[session->parse_cursor] == '\n') {
                session->parse_cursor ++;

                /* header ending check will be performed */
                do_http_request_header_parse(session);

            }else {
                session->parse_cursor ++;
            }
        }
    }

    if (session->parse_status == REQUEST_DATA) {
        //TODO
    }

    if (session->parse_status == REQUEST_END) {
        do_response(session);
    }


    return;
}
