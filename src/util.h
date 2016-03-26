#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

#define DEFAULT_LISTEN_PORT 1080
#define DEFAULT_LISTEN_BACKLOG 5

#define HTTP_HEADER_SIZE 4096

#define bool    uint8_t

#define true    0x01
#define false   0x00


unsigned long get_file_size(const char *path);

#endif
