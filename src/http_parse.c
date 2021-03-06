#include "http_parse.h"
#include "net.h"
#include "util.h"
#include "clog.h"

//#define _GNU_SOURCE
#include <stdlib.h>


#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <assert.h>
static void do_http_request_line_parse(struct session*);
static void do_http_request_header_parse(struct session*);
static void do_GET_response(struct session*);
static void do_response(struct session *session);
static void add_content_type_header(char *file, char *buffer);
static void add_status_line(int, int, char *);
static void add_content_length_header(uint32_t, char *);

static void write_response_cb(int sock, short event, void *arg);

static void remove_arguments(char *);


extern struct event_base *base;

void remove_arguments(char *uri)
{
    char *cr = uri;

    while (*cr != '\0') {
        if (*cr == '?') {
            *cr = '\0';
            break;
        }

        cr += 1;
    }
}

void write_response_cb(int sock, short event, void *arg)
{
    session_t *session = (session_t*)arg;
    char *rp_buf = session->response.rp_buf;
    uint32_t write_cr = session->response.write_cr;
    uint32_t bltw = session->response.bltw;

    uint32_t wl = write(session->sock, rp_buf, bltw - write_cr);

    if (wl < 0) {

    } else if (wl == bltw - write_cr){
        /* finish response */
        free_session(session);
    } else {
        session->response.write_cr += wl;
        event_add(session->write_ev, NULL);
    }
}

void add_content_length_header(uint32_t length, char *buffer)
{
    uint32_t buf_used_len = strlen(buffer);
    char *write_cr = buffer + buf_used_len;

    snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
            "Content-Length: %d\r\n", length);
}

void add_server_header(char *buffer)
{
    assert(buffer);

    int buf_used_len = strlen(buffer);
    char *write_cr = buffer + buf_used_len;

    snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
        "server: feike\r\n");

}

void add_status_line(int protocol, int status_code, char *buffer)
{
    assert(buffer);

    int buf_used_len = strlen(buffer);
    char *write_cr = buffer + buf_used_len;

    int ret = 0;
    if (protocol == HTTP_0_9) {
        ret = snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
            "%s", "HTTP/0.9");
    } else if (protocol == HTTP_1_0) {
        ret = snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
            "%s", "HTTP/1.0");
    } else if (protocol == HTTP_1_1) {

        /* HTTP 1.1 not available temporarily */
        ret = snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
            "%s", "HTTP/1.0");
    } else {
        ;
    }
    write_cr += ret;

    if (status_code == HTTP_OK) {
        sprintf(write_cr, " 200 OK\r\n");
    } else if (status_code == HTTP_NOT_FOUND) {
        sprintf(write_cr, " 404 Not Found\r\n");
    } else {
        //TODO
    }

}

void add_content_type_header(char *file, char *buffer)
{
    assert(buffer);
    assert(file);

    int str_length = strlen(file);
    int buf_used_len = strlen(buffer);
    char *write_cr = buffer + buf_used_len;

    if (str_length >=5 && strncmp(".html", file + str_length - 5, 5) == 0x00) {
        snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
                "Content-Type: text/html\r\n");
    } else if (str_length >= 4 &&
            strncmp(".css", file + str_length - 4, 4) == 0x00) {
        snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
                "Content-Type: text/css\r\n");

    } else if (str_length >= 3 &&
            strncmp(".js", file + str_length - 3, 3) == 0x00) {
        snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
                "Content-Type: text/javascript\r\n");
    } else {
        snprintf(write_cr, HTTP_RESPONSE_BUF_SIZE - buf_used_len - 1,
                "Content-Type: application/octet-stream\r\n");
    }
}


void do_GET_response(struct session *session)
{

    /****************
     * Test code    *
     ****************/

    session->response.status_code = HTTP_OK;
    session->response.protocal = session->request.request_protocal;
    char file_path[PATH_MAX];
    char r_path[PATH_MAX];
    uint32_t p_cr = 0;
    int ret = 0;

    ret = snprintf(file_path, PATH_MAX, HTTP_ROOT_PATH);
    p_cr += ret;

    ret = sscanf(session->request.uri, "%s", file_path + p_cr);

    /* temporary solution */
    remove_arguments(file_path);

    uint32_t fp_len = strlen(file_path);

    if (file_path[fp_len - 1] == '/')
        strcat(file_path, "index.html");

    log_debug("request from fd %d: %s", session->sock, file_path);

    char *r_ptr = realpath(file_path, r_path);


    if (r_ptr == NULL) {
        session->response.status_code = HTTP_NOT_FOUND;
        /* change r_path to 404.html */

        file_path[0] = '\0';
        strcat(file_path, HTTP_ROOT_PATH);
        strcat(file_path, HTTP_404_FILE);

        r_ptr = realpath(file_path, r_path);
    }


    unsigned long f_size = get_file_size(r_path);

    /* response buffer size = header size + content size
     *      !!! linit file size here !!!
     **************************************************/

    session->response.rp_buf = (char*)calloc(HTTP_HEADER_SIZE + f_size,
            sizeof(char));

    add_status_line(session->response.protocal,
            session->response.status_code, session->response.rp_buf);

    add_content_length_header(f_size, session->response.rp_buf);
    add_content_type_header(r_path, session->response.rp_buf);
    add_server_header(session->response.rp_buf);

    strcat(session->response.rp_buf, "\r\n");

    uint32_t hd_len = strlen(session->response.rp_buf);

    FILE *stream = fopen(r_path, "r");

    if (stream == NULL) {
        log_error("Open file failed: %s", file_path);
        return;
    }

    int read_num = fread(session->response.rp_buf + hd_len,
            sizeof(char), f_size, stream);

    if (read_num != f_size) {
        log_error("read file failed: %s", r_path);
        return;
    }
    fclose(stream);




    uint32_t bl = hd_len + f_size;
    session->response.bltw = bl;

    uint32_t wl = write(session->sock, session->response.rp_buf,
            session->response.bltw);

    log_debug("write to fd(%d) %d bytes, successfully write %d bytes",
            session->sock, bl, wl);

    if (wl < 0) {
    } else if (wl == session->response.bltw) {
        /* response finish, clear session */
        //printf("%s", session->response.rp_buf);
        free_session(session);
    } else {
        session->response.write_cr = wl;
        struct event *write_ev = event_new(base, session->sock, EV_WRITE,
                write_response_cb, (void*)session);

        event_add(write_ev, NULL);
    }
}

void do_response(struct session *session)
{

    if (session->request.request_method == HTTP_GET)
        do_GET_response(session);
    else {
        log_error("not method GET");
        //TODO
    }
}

void do_http_request_line_parse(struct session *session)
{

    char *header = session->header;
    uint32_t parse_cursor = session->parse_cursor;
    uint32_t pre_parse_cursor = session->pre_parse_cursor;
    log_debug("pre_parse_cursor: %d", pre_parse_cursor);
    uint32_t tmpCursor = session->pre_parse_cursor;

    while (tmpCursor < session->parse_cursor
            && session->header[tmpCursor++] != ' ') ;

    uint32_t method_str_length = tmpCursor - pre_parse_cursor - 1;


    if (strncmp("GET", header + pre_parse_cursor, method_str_length) == 0) {
        session->request.request_method = HTTP_GET;
    } else {
        log_error("can not parse GET");
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

    if (session->parse_status ==  REQUEST_LINE) {
        log_debug("parsing line");

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

        log_debug("parsing header");

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
        log_debug("doing response");
        do_response(session);
    } else {
        log_debug("%s", session->header);
    }


    return;
}
