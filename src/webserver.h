#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <netinet/in.h>
#include <stdbool.h>

typedef void (*handle_func) (int, void *);

typedef struct web_server
{
    int server_fd;
    struct sockaddr_in address;
    int addrlen;
    bool is_successful;
    bool is_running;
    handle_func handle_client;
} web_server;

void clear_server(web_server * server);
web_server new_server(int port, handle_func handle_client);
void run_server(web_server * server, void * extra_data);

#endif

