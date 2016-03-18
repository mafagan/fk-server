
CC=gcc
LIBS=-lpthread -levent
CFLAGS=-c -g -std=c11 -O2
OBJS=main.o\
	 net.o\
	 util.o\
	 http_parse.o

feike: $(OBJS)
	$(CC) $^ -o $@ $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) $< -o $@

util.o: util.c util.h
	$(CC) $(CFLAGS) $< -o $@

net.o: net.c net.h
	$(CC) $(CFLAGS) $< -o $@

http_request.o: http_request.c http_request.h
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	-rm *.o feike
