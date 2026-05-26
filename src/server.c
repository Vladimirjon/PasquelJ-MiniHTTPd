#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include "server.h"
#include "http.h"

static int set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags < 0)
        return -1;

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int create_listen_socket(int port)
{
    int listenfd;
    int opt = 1;
    struct sockaddr_in addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        return -1;
    }

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        perror("listen");
        close(listenfd);
        return -1;
    }

    if (set_nonblocking(listenfd) < 0) {
        perror("fcntl");
        close(listenfd);
        return -1;
    }

    return listenfd;
}

int run_server(int port, const char *root_dir)
{
    int listenfd;
    int epfd;
    int nready;
    int clientfd;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];

    listenfd = create_listen_socket(port);
    if (listenfd < 0)
        return 1;

    epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("epoll_create1");
        close(listenfd);
        return 1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        perror("epoll_ctl listenfd");
        close(listenfd);
        close(epfd);
        return 1;
    }

    while (1) {
        nready = epoll_wait(epfd, events, MAX_EVENTS, -1);

        if (nready < 0) {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nready; i++) {
            int fd = events[i].data.fd;

            if (fd == listenfd) {
                while (1) {
                    clientfd = accept(listenfd, NULL, NULL);

                    if (clientfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        perror("accept");
                        break;
                    }

                    ev.events = EPOLLIN;
                    ev.data.fd = clientfd;

                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev) < 0) {
                        perror("epoll_ctl clientfd");
                        close(clientfd);
                    }
                }
            } else {
                int keep = handle_http_request(fd, root_dir);

                if (!keep) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                }
            }
        }
    }

    close(listenfd);
    close(epfd);
    return 0;
}