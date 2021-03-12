/*
** EPITECH PROJECT, 2019
** boring project
** File description:
** Header File
*/




/*

timeout= 5mn
Log try= 3

227
bind try
random port

*/

#include "my_ftp.h"

int main(int ac, char **av) {
    info_t info;
    int max_sd, ret;

    if (ac != 3 || strcmp(av[1], "–h") == 0 || strcmp(av[1], "–help") == 0) {
        printf("USAGE: ./myftp port path\n\tport is the port number on which the server socket listens\n\tpath is the path to the home directory for the Anonymous user");
        return 0;
    }

    srand(time(NULL));
    if (init(&info, atoi(av[1]), av[2]) == -1)
        return 84;

    while(info.run) {
        max_sd = fill_sockset(&info);
        ret = select(max_sd + 1, &info.readfds, NULL, NULL, NULL);

        if (ret == -1) {
            printf("select error");
            continue;
        }

        if (FD_ISSET(info.server, &info.readfds))
            add_client(&info);
        else if (fd_isset_data(&info))
            continue;
        else {
            for (int i = 0; i < info.max_clients; i++)
                listen_data(&info, i);
            for (int i = 0; i < info.max_clients; i++)
                listen_client(&info, i);
        }
    }
    clear_all(&info);

    return 0;
}

int init(info_t *info, int port, char *path) {

    if (port < 1 || port > 65534) {
        fprintf(stderr, "Port out of range (1-65535)\n");
        return -1;
    } else if (path == NULL || path[0] == '\0') {
        fprintf(stderr, "Empty path\n");
        return -1;
    }

    info->run = 1;
    info->opt = 1;
    info->def_path = strdup(path);
    info->max_clients = 10000;
    info->address.sin_family = AF_INET;
    info->address.sin_addr.s_addr = INADDR_ANY;
    info->address.sin_port = htons(port);
    info->client = malloc(sizeof(client_t) * info->max_clients);

    for(int i = 0; i < info->max_clients; i++) {
        info->client[i].fd = 0;
        info->client[i].logged = 0;
        info->client[i].log_try = 0;
        info->client[i].username = NULL;
        info->client[i].pwd = NULL;

        info->client[i].data_socket.mode = 0;
        info->client[i].data_socket.client_fd = 0;
        info->client[i].data_socket.server_fd = 0;
        info->client[i].data_socket.buffer_data = 0;
        info->client[i].data_socket.buffer_client = 0;
        info->client[i].data_socket.address.sin_family = AF_INET;
        info->client[i].data_socket.address.sin_addr.s_addr = INADDR_ANY;
    }
    info->server = socket(AF_INET, SOCK_STREAM, 0);

    if (info->server == 0) {
        perror("socket failed");
        free(info->def_path);
        free(info->client);
        return -1;
    }
    if (setsockopt(info->server, SOL_SOCKET, SO_REUSEADDR, (char *)&info->opt, sizeof(info->opt)) < 0) {
        perror("setsockopt");
        free(info->def_path);
        free(info->client);
        return -1;
    }
    if (bind(info->server, (const struct sockaddr *) &info->address, sizeof(info->address)) < 0) {
        perror("bind failed");
        free(info->def_path);
        free(info->client);
        return -1;
    }
    if (listen(info->server, 2) == -1) {
        perror("listen");
        free(info->def_path);
        free(info->client);
        return -1;
    }
    info->addrlen = sizeof(info->address);

    return 0;
}

void clear_all(info_t *info) {

    for(int i = 0; i < info->max_clients; i++) {
        if (info->client[i].username)
            free(info->client[i].username);
        if (info->client[i].pwd)
            free(info->client[i].pwd);

        clear_data_socket(&info->client[i].data_socket);
    }
    free(info->client);
    close(info->server);
    free(info->def_path);
}

void clear_data_socket(data_t *socket) {

    if (socket->client_fd)
        close(socket->client_fd);
    if (socket->server_fd)
        close(socket->server_fd);
    if (socket->buffer_data)
        close(socket->buffer_data);
    if (socket->buffer_client)
        close(socket->buffer_client);

    socket->client_fd = 0;
    socket->server_fd = 0;
    socket->buffer_data = 0;
    socket->buffer_client = 0;
}
