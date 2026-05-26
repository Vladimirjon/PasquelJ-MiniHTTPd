#ifndef SERVER_H
#define SERVER_H

#define MAX_EVENTS 64
#define MAX_FD     4096
#define CLIENT_TIMEOUT 30

int run_server(int port, const char *root_dir);

#endif
