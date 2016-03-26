#include <stdio.h>
#include <limits.h>

#define _GNU_SOURCE
#include <stdlib.h>

int
main(int argc, char *argv[])
{
    char *symlinkpath = argv[1];
    char actualpath [PATH_MAX];
    char *ptr;
    ptr = realpath(symlinkpath, actualpath);

    if (ptr == NULL) printf("heihei\n");
    printf("%s\n", ptr);
    printf("%s\n", actualpath);


    return 0;
}
