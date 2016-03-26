#include "util.h"

#include <sys/stat.h>

unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;

    struct stat statbuff;

    if (stat(path, &statbuff) < 0) return filesize;
    else return statbuff.st_size;
}
