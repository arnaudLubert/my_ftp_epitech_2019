/*
** EPITECH PROJECT, 2019
** boring project
** File description:
** Header File
*/

#include "my_ftp.h"

void add_client(info_t *info) {
    int new_client = accept(info->server, (struct sockaddr *) &info->address, &info->addrlen);

    if (new_client < 0) {
        printf("accept failed (client)\r\n");
        return;
    }

    printf("New connection , socket fd is %d , ip is : %s , port : %d\r\n", new_client, inet_ntoa(info->address.sin_addr), ntohs(info->address.sin_port));

    if (write(new_client, "220 Service ready for new user.\r\n", 33) != 33)
        printf("send failed");

    for (int i = 0; i < info->max_clients; i++)
        if (info->client[i].fd == 0) {
            info->client[i].fd = new_client;
            info->client[i].username = NULL;
            info->client[i].pwd = NULL;
            break;
        }
}

int login(client_t *client, char *pass, char *def_path) {

    if (strcmp(client->username, "Anonymous") == 0) {
        client->logged = 1;
        client->pwd = strdup(def_path);
        return 0;
    }

    return -1;
}


void change_username(client_t *client, char *name) {

    if (client->username)
        free(client->username);
    client->username = strdup(name);

}

void disconnect_client(client_t *client) {
    close(client->fd);
    client->fd = 0;
    client->logged = 0;
    client->log_try = 0;
    if (client->username) {
        free(client->username);
        client->username = NULL;
    }
    if (client->pwd) {
        free(client->pwd);
        client->pwd = NULL;
    }
}
