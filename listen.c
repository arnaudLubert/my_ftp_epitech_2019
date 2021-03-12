/*
** EPITECH PROJECT, 2019
** boring project
** File description:
** Header File
*/

#include "my_ftp.h"

void listen_data(info_t *info, int id) {
    int valread;

    if (info->client[id].data_socket.client_fd != 0 && FD_ISSET(info->client[id].data_socket.client_fd, &info->readfds)) {
        valread = read(info->client[id].data_socket.client_fd, info->buffer, sizeof(info->buffer));

        if (valread == 0) {
            printf("Data client disconnected ID:%d Mode:%d\n", id, info->client[id].data_socket.mode);
            dprintf(info->client[id].fd, "226 File transefered. Closing data connection.\r\n");

            if (info->client[id].data_socket.mode == MODE_READ) {
                close(info->client[id].data_socket.buffer_data);
                info->client[id].data_socket.buffer_data = 0;
                info->client[id].data_socket.mode = 0;
            }

            close(info->client[id].data_socket.client_fd);
            info->client[id].data_socket.client_fd = 0;
        } else if (info->client[id].data_socket.mode == MODE_READ) {
            printf("write %d bytes in fd %d\n", valread, info->client[id].data_socket.buffer_data);
            write(info->client[id].data_socket.buffer_data, info->buffer, valread);
            dprintf(info->client[id].fd, "226 File transefered. Closing data connection.\r\n");

            if (valread < sizeof(info->buffer)) {
                close(info->client[id].data_socket.buffer_data);
                close(info->client[id].data_socket.client_fd);
                info->client[id].data_socket.buffer_data = 0;
                info->client[id].data_socket.client_fd = 0;
                info->client[id].data_socket.mode = 0;
            }
        }
    }
}

void listen_client(info_t *info, int id) {
    int valread;

    if (FD_ISSET(info->client[id].fd, &info->readfds)) {
        valread = read(info->client[id].fd, info->buffer, sizeof(info->buffer));

        if (valread == 0) {
            getpeername(info->client[id].fd, (struct sockaddr *) &info->address, &info->addrlen);
            printf("Host disconnected, ip %s, port %d \r\n", inet_ntoa(info->address.sin_addr), ntohs(info->address.sin_port));
            disconnect_client(&info->client[id]);
            return;
        }

        info->buffer[valread] = '\0';
        if (valread != 0 && info->buffer[valread - 1] == '\n') {
            info->buffer[valread - 1] = '\0';
            valread--;
        }
        if (valread != 0 && info->buffer[valread - 1] == '\r') {
            info->buffer[valread - 1] = '\0';
            valread--;
        }
        printf("Received: %s\r\nFrom: %s\r\n", info->buffer, inet_ntoa(info->address.sin_addr));

        if (valread == 0)
            return;
        else
            execute(info, id);
    }
}

void execute(info_t *info, int id) {
    char *occ = strchr(info->buffer, ' ');
    char *arg = NULL;
    char *command;
    int fd = info->client[id].fd;

    if (occ == NULL) {
        command = info->buffer;
    } else {
        *occ = '\0';
        command = info->buffer;

        if (strlen(&occ[1]) > 0)
            arg = &occ[1];
    }

    if (strcmp(command, "USER") == 0) {
        if (info->client[id].logged != 0)
            write(fd, "530 Can\'t change from guest user.\r\n", 35);
        else if (arg) {
            change_username(&info->client[id], arg);
            write(fd, "331 Please specify the password.\r\n", 34);
        } else
            write(fd, "530 Permission denied.\r\n", 24);

    } else if (strcmp(command, "PASS") == 0) {
        if (info->client[id].logged != 0)
            write(fd, "230 Already logged in.\r\n", 24);
        else if (info->client[id].username == NULL)
            write(fd, "503 Login with USER first.\r\n", 28);
        else if (login(&info->client[id], arg, info->def_path) != -1)
            write(fd, "230 Login successful.\r\n", 23);
        else
            write(fd, "530 Login incorrect.\r\n", 22);

    } else if (strcmp(command, "stop") == 0)
        info->run = 0;
    else if (info->client[id].logged == 0)
        write(fd, "530 Please login with USER and PASS.\r\n", 38);
    else if (strcmp(command, "NOOP") == 0)
        write(fd, "200 NOOP ok.\r\n", 14);
    else if (strcmp(command, "PASV") == 0) {
        unsigned long ip = get_ip_address();
        int port = create_data_socket(info, id);

        if (port != 0)
            dprintf(fd, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", (int)(ip & 0xFF), (int)((ip & 0xFF00) >> 8), (int)((ip & 0xFF0000) >> 16), (int)((ip & 0xFF000000) >> 24), (int)(port / 256), port % 256);
        else
            write(fd, "425 Can\'t open data connection.\r\n", 33);

    } else if (strcmp(command, "LIST") == 0) {
        if (info->client[id].data_socket.server_fd == 0)
            dprintf(fd, "425 Use PORT or PASV first.\r\n");
        else if (info->client[id].data_socket.buffer_data != 0)
            dprintf(fd, "550 Data pending in buffer.\r\n");
        else if (send_list(info, info->client[id].data_socket.client_fd, id, info->client[id].pwd, arg)) {
            dprintf(fd, "150 Here comes the directory listing.\r\n");

            if (info->client[id].data_socket.buffer_data == 0) {
                close(info->client[id].data_socket.client_fd);
                close(info->client[id].data_socket.server_fd);
                info->client[id].data_socket.client_fd = 0;
                info->client[id].data_socket.server_fd = 0;
                info->client[id].data_socket.mode = 0;
                dprintf(fd, "226 Directory send OK.\r\n");
            } else
                dprintf(info->client[id].data_socket.buffer_client, "226 Directory send OK.\r\n");
        } else
            dprintf(fd, "550 Failed to access location.\r\n");
    } else if (strcmp(command, "PWD") == 0)
        dprintf(fd, "257 \"%s\"\r\n", info->client[id].pwd);
    else if (strcmp(command, "CDUP") == 0) {

        change_wide_directory(&info->client[id].pwd, "..");
        dprintf(fd, "250 Directory successfully changed.\r\n");

    } else if (strcmp(command, "CWD") == 0 || strcmp(command, "cd") == 0) {
        if (arg && change_wide_directory(&info->client[id].pwd, arg))
            dprintf(fd, "250 Directory successfully changed.\r\n");
        else
            dprintf(fd, "550 Failed to change directory.\r\n");

    } else if (strcmp(command, "DELE") == 0) {

        if (arg && delete_file(info->client[id].pwd, arg))
            dprintf(fd, "250 File successfully deleted.\r\n");
        else
            dprintf(fd, "550 Failed to delete file.\r\n");

    } else if (strcmp(command, "RETR") == 0) {
        if (info->client[id].data_socket.buffer_data != 0) {
            close(info->client[id].data_socket.buffer_data);
            info->client[id].data_socket.buffer_data = 0;
        }
        if (info->client[id].data_socket.buffer_client != 0) {
            close(info->client[id].data_socket.buffer_client);
            info->client[id].data_socket.buffer_client = 0;
        }

        if (info->client[id].data_socket.server_fd == 0)
            dprintf(fd, "425 Use PORT or PASV first.\r\n");
        else if (arg == NULL)
            dprintf(fd, "501 Argument is missing.\r\n");
        else if (send_file(info, info->client[id].data_socket.client_fd, id, info->client[id].pwd, arg)) {
            dprintf(fd, "150 Opening binary mode data connection for %s.\r\n", arg);

            if (info->client[id].data_socket.buffer_data == 0) {
                close(info->client[id].data_socket.client_fd);
                close(info->client[id].data_socket.server_fd);
                info->client[id].data_socket.client_fd = 0;
                info->client[id].data_socket.server_fd = 0;
                dprintf(fd, "226 Transfer complete.\r\n");
            } else
                dprintf(info->client[id].data_socket.buffer_client, "226 Transfer complete.\r\n");
        } else
            dprintf(fd, "550 Failed to open file.\r\n");

    } else if (strcmp(command, "STOR") == 0) {
        if (info->client[id].data_socket.buffer_data != 0) {
            close(info->client[id].data_socket.buffer_data);
            info->client[id].data_socket.buffer_data = 0;
        }
        if (info->client[id].data_socket.buffer_client != 0) {
            close(info->client[id].data_socket.buffer_client);
            info->client[id].data_socket.buffer_client = 0;
        }

        if (info->client[id].data_socket.server_fd == 0)
            dprintf(fd, "425 Use PORT or PASV first.\r\n");
        else if (arg == NULL)
            dprintf(fd, "501 Argument is missing.\r\n");
        else if (recv_file(info, id, info->client[id].pwd, arg))
            dprintf(fd, "150 Ready to receive file %s.\r\n", arg);
        else
            dprintf(fd, "550 Failed to create/open file.\r\n");

    } else if (strcmp(command, "MKD") == 0) {
        if (arg) {
            char *path = merge_paths(info->client[id].pwd, arg);
            int ret = create_directory(path);

            if (ret == 1)
                dprintf(fd, "257 \"%s\" directory created\r\n", path);
            else if (ret == 2) {
                dprintf(fd, "521-\"%s\" directory already exists;\r\n", path);
                dprintf(fd, "521 taking no action.\r\n");
            } else
                dprintf(fd, "550 Could not create the directory.\r\n");
            free(path);
        } else
            dprintf(fd, "501 Argument is missing.\r\n");

    } else if (strcmp(command, "RMD") == 0) {
        if (arg == NULL)
            dprintf(fd, "501 Argument is missing.\r\n");
        else if (delete_file(info->client[id].pwd, arg))
            dprintf(fd, "257 Directory successfully removed.\r\n");
        else
            dprintf(fd, "550 Could not delete the directory.\r\n");

    } else if (strcmp(command, "TYPE") == 0) {
        if (arg && strcmp(arg, "A") == 0)
            dprintf(fd, "200 Switching to ASCII mode.\r\n");
        else if (arg && strcmp(arg, "I") == 0)
            dprintf(fd, "200 Switching to Binary mode.\r\n");
        else
            dprintf(fd, "500 Unrecognised TYPE command.\r\n");

    } else if (strcmp(command, "SYST") == 0)
        dprintf(fd, "215 EPITECH myftp server\r\n");

    else if (strcmp(command, "HELP") == 0) {
        dprintf(fd, "214-The following commands are recognized.\r\n CDUP CWD DELEHELP LIST MKD NOOP PASS PASV PORT PWD QUIT USER RETR RMD\r\nSTOR SYST TYPE\r\n cd ls\r\n");
        dprintf(fd, "214 Help OK.\r\n");

    } else if (strcmp(command, "QUIT") == 0) {
        dprintf(fd, "221 Goodbye.\r\n");
        disconnect_client(&info->client[id]);

    } else if (strcmp(command, "ls") == 0)
        send_list(info, fd, id, info->client[id].pwd, arg);
    else
        write(fd, "500 Unknown command.\r\n", 22);
}
