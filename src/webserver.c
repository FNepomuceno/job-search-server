#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "webserver.h"

void clear_server(web_server * server)
{
    // Close the socket
    close(server->server_fd);
    server->server_fd = 0;
}

web_server new_server(int port, handle_func handle_client)
{
    web_server result;
    result.is_successful = true;
    result.handle_client = handle_client;
    result.is_running = false;

    // Create socket file descriptor
    result.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (result.server_fd == 0)
    {
        perror("Could not open socket");
        result.is_successful = false;
        return result;
    }

    // Make socket "reusable"
    int option = 1;
    setsockopt(result.server_fd, SOL_SOCKET, SO_REUSEADDR, &option,
            sizeof(option));

    // Make internet-style socket address structure
    result.addrlen = sizeof(result.address);

    result.address.sin_family = AF_INET;
    result.address.sin_addr.s_addr = INADDR_ANY;
    result.address.sin_port = htons(port);

    memset(result.address.sin_zero, '\0',
        sizeof(result.address.sin_zero));

    // Bind address to socket
    if (bind(result.server_fd,
        (struct sockaddr *)&result.address,
        sizeof(result.address)) < 0)
    {
        perror("Could not bind address");
        result.is_successful = false;
        return result;
    }

    // Open socket for requests
    if (listen(result.server_fd, 1024) < 0)
    {
        perror("Problem with listen");
        result.is_successful = false;
        return result;
    }

    // Resulting socket ready for response loop
    return result;
}

void run_server(web_server * server, void * extra_data)
{
    if (!(server->is_successful)) return;

    server->is_running = true;
    while(server->is_running)
    {
        int clientfd = accept(
            server->server_fd,
            (struct sockaddr *)&(server->address),
            (socklen_t *)&(server->addrlen)
        );
        if (clientfd < 0)
        {
            perror("Could not accept connection");
            server->is_successful = false;
            return;
        }

        (server->handle_client)(clientfd, extra_data);
        close(clientfd);
    }
}
