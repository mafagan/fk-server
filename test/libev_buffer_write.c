/*
 * libevent echo server example.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* For inet_ntoa. */
#include <arpa/inet.h>

/* Required by event.h. */
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>

/* Easy sensible linked lists. */
#include "queue.h"

/* Libevent. */
#include <event.h>
/* Port to listen on. */
#define SERVER_PORT 5555

/* Length of each buffer in the buffer queue. Also becomes the amount
 * of data we try to read per call to read(2). */
#define BUFLEN 1024

/**
 * 在以事件为基础的编程时，我们需要一个队列来存储要写入的数据，
 * 直到libevent通知我们可以写入，才能够真正的写入数据。这个简单的buffer队列用
 * queue.h中的TAILQ来写入。
 */
struct bufferq
{
    /* The buffer. */
    u_char *buf;

    /* The length of buf. */
    int len;

    /* 本次写buffer的开始位置偏移。 */
    int offset;

    /* 链表结构。 */
    TAILQ_ENTRY(bufferq) entries;
};

/**
 * 指定的客户数据结构，也包含一个存储一列用户的指针。
 *
 * 在基于事件的程序中，通常需要为每个客户保存一些状态信息的对象。
 */
struct client
{
    /* 事件对象。我们需要两个事件对象，一个用于读事件一个用于写事件通知。 */
    struct event ev_read;
    struct event ev_write;

    /* 这是数据队列，它将被写入这个客户端。我们只能等到libevent告诉我们这个socket准备好写入时，
     * 才能真正写入。 */
    TAILQ_HEAD(, bufferq) writeq;
};

/**
 * 设置socket为非阻塞模式。
 */
int
setnonblock(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (flags < 0)
        return flags;
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;

    return 0;
}

/**
 * 当客户端socket可读时，libevent调用这个函数。
 */
void
on_read(int fd, short ev, void *arg)
{
    struct client *client = (struct client *)arg;
    struct bufferq *bufferq;
    u_char *buf;
    int len;

    /* 因为我们需要在可写事接收到通知，所以我们需要分配读取缓冲区，
     * 并且将它放到客户数据对象的写入队列。 */
    buf = malloc(BUFLEN);
    if (buf == NULL)
        err(1, "malloc failed");

    len = read(fd, buf, BUFLEN);
    if (len == 0)
    {
        /* 客户端断开连接，在这里移除读事件并释放客户数据结构。 */
        printf("Client disconnected.\n");
        close(fd);
        event_del(&client->ev_read);
        free(client);
        return;
    }
    else if (len < 0)
    {
        /* 一些其它的错误发生，在这里关闭socket、事件对象并释放客户数据结构。 */
        printf("Socket failure, disconnecting client: %s",
               strerror(errno));
        close(fd);
        event_del(&client->ev_read);
        free(client);
        return;
    }

    /* 我们不能仅仅将buffer写回，因为我们需要在可以写入时响应libevent的可写事件，
     * 将缓冲区放到客户数据结构中的写队列，并安排一个可写事件。 */
    bufferq = calloc(1, sizeof(*bufferq));
    if (bufferq == NULL)
        err(1, "malloc faild");
    bufferq->buf = buf;
    bufferq->len = len;
    bufferq->offset = 0;
    TAILQ_INSERT_TAIL(&client->writeq, bufferq, entries);

    /* 由于现在我们拥有需要写回到客户端的数据，所以添加一个可写事件通知。 */
    event_add(&client->ev_write, NULL);
}

/**
 * 当客户端socket准备好写入时，libevent调用这个函数。
 */
void
on_write(int fd, short ev, void *arg)
{
    struct client *client = (struct client *)arg;
    struct bufferq *bufferq;
    int len;

    /* 将第一个元素移出写队列。我们可能不用再探测写队列是否是空，但是应该确认返回值不是NULL。 */
    bufferq = TAILQ_FIRST(&client->writeq);
    if (bufferq == NULL)
        return;

    /* 写buffer里的数据。buffer中的一部分可能已经在前面写入了，所以只写剩下的字节。 */
    len = bufferq->len - bufferq->offset;
    len = write(fd, bufferq->buf + bufferq->offset,
                bufferq->len - bufferq->offset);
    if (len == -1)
    {
        if (errno == EINTR || errno == EAGAIN)
        {
            /* 写操作被一个信号打断，或者我们不能写入数据，调整一下并返回。 */
            event_add(&client->ev_write, NULL);
            return;
        }
        else
        {
            /* 一些其它的socket错误发生，退出吧。 */
            err(1, "write");
        }
    }
    else if ((bufferq->offset + len) < bufferq->len)
    {
        /* 不是所有的数据都被写入了，更新写入的偏移并调整写入事件。 */
        bufferq->offset += len;
        event_add(&client->ev_write, NULL);
        return;
    }

    /* 数据已经完整的写入了，从写队列中移除这个buffer。 */
    TAILQ_REMOVE(&client->writeq, bufferq, entries);
    free(bufferq->buf);
    free(bufferq);
}

/**
 * 当有一个连接请求准备被接受时，libevent将调用这个函数。
 */
void
on_accept(int fd, short ev, void *arg)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct client *client;

    /* 接受这个新的连接请求。 */
    client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1)
    {
        warn("accept failed");
        return;
    }

    /* 设置客户端socket为非阻塞模式。 */
    if (setnonblock(client_fd) < 0)
        warn("failed to set client socket non-blocking");

    /* 我们已经接受了一个新的客户，分配一个客户数据结构对象来保存这个客户的状态。 */
    client = calloc(1, sizeof(*client));
    if (client == NULL)
        err(1, "malloc failed");

    /* 设置读事件，当客户端socket可读时，libevent将调用on_read()函数。
     * 我们也不断的构造可读事件，所以不用再每次读取时从新加入读事件了。 */
    event_set(&client->ev_read, client_fd, EV_READ|EV_PERSIST, on_read,
              client);

    /* 设置的事件没有激活，添加事件以便激活它。 */
    event_add(&client->ev_read, NULL);

    /* 创建写事件，但是在我们有数据可写之前，不要添加它。 */
    event_set(&client->ev_write, client_fd, EV_WRITE, on_write, client);

    /* 初始化客户端写队列。 */
    TAILQ_INIT(&client->writeq);

    printf("Accepted connection from %s\n",
           inet_ntoa(client_addr.sin_addr));
}

int
main(int argc, char **argv)
{
    int listen_fd;
    struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;

    /* socket的接受事件对象。 */
    struct event ev_accept;

    /* 初始化 libevent. */
    event_init();

    /* 创建我们的监听socket。 This is largely boiler plate
     * code that I'll abstract away in the future. */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        err(1, "listen failed");
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on,
                   sizeof(reuseaddr_on)) == -1)
        err(1, "setsockopt failed");
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(SERVER_PORT);
    if (bind(listen_fd, (struct sockaddr *)&listen_addr,
             sizeof(listen_addr)) < 0)
        err(1, "bind failed");
    if (listen(listen_fd, 5) < 0)
        err(1, "listen failed");

    /* 设置socket为非阻塞模式，这个在使用libevent编程中时必须的。 */
    if (setnonblock(listen_fd) < 0)
        err(1, "failed to set server socket to non-blocking");

    /* 我们现在有了一个监听的socket，我们创建一个读事件，以便有客户连接请求时能够接收到通知。 */
    event_set(&ev_accept, listen_fd, EV_READ|EV_PERSIST, on_accept, NULL);
    event_add(&ev_accept, NULL);

    /* 启动 libevent 时间循环. */
    event_dispatch();

    return 0;
}
