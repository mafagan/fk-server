
CC=gcc
LIBS=-lpthread -levent
CFLAGS=-Wall -c -g -D_BSD_SOURCE -D_POSIX_SOURCE -std=c99
OBJS=main.o \
	 net.o \
	 http_parse.o \
	 util.o \
	 signal_handle.o

feike: $(OBJS)
	$(CC) $^ -o $@ $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) $< -o $@

util.o: util.c util.h
	$(CC) $(CFLAGS) $< -o $@

http_request.o: http_request.c http_request.h
	$(CC) $(CFLAGS) $< -o $@

net.o: net.c net.h
	$(CC) $(CFLAGS) $< -o $@

signal_handle.o: signal_handle.c signal_handle.h
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean clear

clean:
	-rm *.o feike core

clear: clean
	-rm feike.log
