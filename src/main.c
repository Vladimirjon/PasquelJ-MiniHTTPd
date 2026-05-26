#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "server.h"

int main(int argc, char *argv[])
{
    int port = 8080;

    if (argc == 2)
        port = atoi(argv[1]);

    signal(SIGPIPE, SIG_IGN);

    printf("MiniHTTPd escuchando en puerto %d\n", port);
    printf("Directorio raiz: www\n");

    return run_server(port, "www");
}