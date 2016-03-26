
#ifndef _HTTP_PARSE_H
#define _HTTP_PARSE_H

#include <stdint.h>

/* http status code */
#define HTTP_CONTINUE                  100
#define HTTP_SWITCHING_PROTOCOLS       101
#define HTTP_PROCESSING                102

#define HTTP_OK                        200
#define HTTP_CREATED                   201
#define HTTP_ACCEPTED                  202
#define HTTP_NO_CONTENT                204
#define HTTP_PARTIAL_CONTENT           206

#define HTTP_SPECIAL_RESPONSE          300
#define HTTP_MOVED_PERMANENTLY         301
#define HTTP_MOVED_TEMPORARILY         302
#define HTTP_SEE_OTHER                 303
#define HTTP_NOT_MODIFIED              304
#define HTTP_TEMPORARY_REDIRECT        307

#define HTTP_BAD_REQUEST               400
#define HTTP_UNAUTHORIZED              401
#define HTTP_FORBIDDEN                 403
#define HTTP_NOT_FOUND                 404
#define HTTP_NOT_ALLOWED               405

/* http method */
#define HTTP_GET                       0x0002
#define HTTP_HEAD                      0x0004
#define HTTP_POST                      0x0008
#define HTTP_PUT                       0x0010
#define HTTP_DELETE                    0x0020

/* http protocol */
#define HTTP_0_9                       0x0002
#define HTTP_1_0                       0x0004
#define HTTP_1_1                       0x0008
#define HTTP_2_0                       0x0010

#define HTTP_PROTOCOL_LEN              0x0008

#define HTTP_RESPONSE_BUF_SIZE         40960


#define HTTP_ROOT_PATH                 "/home/winter/fk-server/html"
#define HTTP_404_FILE                   "/404.html"

struct session;

typedef enum {
    REQUEST_LINE,
    REQUEST_HEADER,
    REQUEST_DATA,
    REQUEST_END
} request_parse_status_t;

typedef struct http_request {
    uint16_t request_method;
    uint16_t request_protocal;

    char *Accept;

    char *uri;

} http_request_t;

typedef struct http_response {
    uint16_t protocal;
    uint16_t status_code;

    char *rp_buf;

    /* buffer length to write */
    uint32_t bltw;
    uint32_t write_cr;
} http_response_t;

void do_http_request_parse(struct session*);

#endif
