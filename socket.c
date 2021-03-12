/*
** EPITECH PROJECT, 2019
** boring project
** File description:
** Header File
*/

#include "my_ftp.h"

int create_data_socket(info_t *info, int id) {
    int port = 0;
    int try = 0;

    if (info->client[id].data_socket.client_fd != 0) {
        close(info->client[id].data_socket.client_fd);
        info->client[id].data_socket.client_fd = 0;
    }

    if (info->client[id].data_socket.server_fd != 0) {
        close(info->client[id].data_socket.server_fd);
        info->client[id].data_socket.server_fd = 0;
    }

    info->client[id].data_socket.address.sin_family = AF_INET;
    info->client[id].data_socket.address.sin_addr.s_addr = INADDR_ANY;
    info->client[id].data_socket.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (info->client[id].data_socket.server_fd == 0) {
        perror("socket failed");
        return 0;
    }

    while (port == 0 && try < 100) {
        port = rand() % 64511 + 1024;
        info->client[id].data_socket.address.sin_port = htons(port);
        info->client[id].data_socket.addrlen = sizeof(info->client[id].data_socket.address);

        if (setsockopt(info->server, SOL_SOCKET, SO_REUSEADDR, (char *)&info->opt, sizeof(info->opt)) < 0) {
            perror("setsockopt");
            return 0;
        }
        if (bind(info->client[id].data_socket.server_fd, (const struct sockaddr *) &info->client[id].data_socket.address, info->client[id].data_socket.addrlen) < 0) {
            perror("bind failed");
            try++;
            continue;
        }
        if (listen(info->client[id].data_socket.server_fd, 2) == -1) {
            perror("listen");
            return 0;
        }
    }

    if (try == 100)
        return 0;
    else
        return port;
}

int create_data_buffer(info_t *info, int id) {
    char *temp_path = strdup("temp/#dataNUM#");

    temp_path[12] = (char)(id % 10) + '0';
    temp_path[11] = (char)(id % 100 / 10) + '0';
    temp_path[10] = (char)(id % 1000 / 100) + '0';
    info->client[id].data_socket.buffer_data = open(temp_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    free(temp_path);

    temp_path = strdup("temp/#clntNUM#");
    temp_path[12] = (char)(id % 10) + '0';
    temp_path[11] = (char)(id % 100 / 10) + '0';
    temp_path[10] = (char)(id % 1000 / 100) + '0';
    info->client[id].data_socket.buffer_client = open(temp_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    free(temp_path);

    return info->client[id].data_socket.buffer_data;
}

int fd_isset_data(info_t *info) {
    int new_client;

    for (int i = 0; i < info->max_clients; i++) {
        if (info->client[i].data_socket.client_fd == 0 && FD_ISSET(info->client[i].data_socket.server_fd, &info->readfds)) {
            new_client = accept(info->client[i].data_socket.server_fd, (struct sockaddr *) &info->client[i].data_socket.address, &info->client[i].data_socket.addrlen);

            if (new_client == -1) {
                perror("accept failed (server)");
                continue;
            }

            printf("new Data client connected %d %d\n", i, info->client[i].data_socket.buffer_data);
            info->client[i].data_socket.client_fd = new_client;

            if (info->client[i].data_socket.buffer_data != 0 && info->client[i].data_socket.mode == MODE_WRITE) {
                struct stat st;
                char *buff;

                fstat(info->client[i].data_socket.buffer_data, &st);
                buff = malloc(st.st_size + 1);
                lseek(info->client[i].data_socket.buffer_data, 0, SEEK_SET);
                read(info->client[i].data_socket.buffer_data, buff, st.st_size);
                write(info->client[i].data_socket.client_fd, buff, st.st_size);
                free(buff);

                fstat(info->client[i].data_socket.buffer_client, &st);
                buff = malloc(st.st_size + 1);
                lseek(info->client[i].data_socket.buffer_client, 0, SEEK_SET);
                read(info->client[i].data_socket.buffer_client, buff, st.st_size);
                write(info->client[i].fd, buff, st.st_size);
                free(buff);

                close(info->client[i].data_socket.client_fd);
                close(info->client[i].data_socket.server_fd);
                close(info->client[i].data_socket.buffer_client);
                close(info->client[i].data_socket.buffer_data);
                info->client[i].data_socket.client_fd = 0;
                info->client[i].data_socket.server_fd = 0;
                info->client[i].data_socket.buffer_client = 0;
                info->client[i].data_socket.buffer_data = 0;
            }

            return 1;
        }
    }

    return 0;
}

int fill_sockset(info_t *info) {
    int max_sd = info->server;

    FD_ZERO(&info->readfds);
    FD_SET(info->server, &info->readfds);

    for (int i = 0; i < info->max_clients; i++) {

        if (info->client[i].fd != 0)
            FD_SET(info->client[i].fd, &info->readfds);
        if (info->client[i].data_socket.server_fd != 0)
            FD_SET(info->client[i].data_socket.server_fd, &info->readfds);
        if (info->client[i].data_socket.client_fd != 0)
            FD_SET(info->client[i].data_socket.client_fd, &info->readfds);

        if (info->client[i].fd > max_sd)
            max_sd = info->client[i].fd;
        if (info->client[i].data_socket.server_fd > max_sd)
            max_sd = info->client[i].data_socket.server_fd;
        if (info->client[i].data_socket.client_fd > max_sd)
            max_sd = info->client[i].data_socket.client_fd;
    }

    return max_sd;
}

int fill_data_sockset(data_t *socket) {
    int max_sd;

    FD_ZERO(&socket->readfds);
    FD_SET(socket->server_fd, &socket->readfds);
    max_sd = socket->server_fd;

    if (socket->client_fd > 0)
        FD_SET(socket->client_fd, &socket->readfds);

    if (socket->client_fd > max_sd)
        max_sd = socket->client_fd;

    return max_sd;
}
