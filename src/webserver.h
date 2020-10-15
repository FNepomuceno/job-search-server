#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <netinet/in.h>
#include <stdbool.h>

typedef struct web_server
{
	int server_fd;
	struct sockaddr_in address;
	int addrlen;
	bool is_successful;
	void (*handle_client)(int);
} web_server;

void clear_server(web_server * server);
web_server new_server(int port, void (*handle_client)(int));
void run_server(web_server * server);

#endif

