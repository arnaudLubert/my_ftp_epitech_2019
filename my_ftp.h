/*
** EPITECH PROJECT, 2019
** my_ftp
** File description:
** Header File
*/
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef MY_FTP
#define MY_FTP

typedef struct data_s {
    char mode;
    int client_fd;
    int server_fd;
    int buffer_data;
    int buffer_client;
    fd_set readfds;
    socklen_t addrlen;
    struct sockaddr_in address;
} data_t;

typedef struct client_s {
    int fd;
    int logged;
    int log_try;
    char *username;
    char *pwd;
    data_t data_socket;
} client_t;

typedef struct info_s {
    int run, opt, max_clients;
    int server;
    char buffer[10240];
    char *def_path;
    fd_set readfds;
    client_t *client;
    socklen_t addrlen;
    struct sockaddr_in address;
} info_t;

enum buffer_mode{ MODE_WRITE = 0, MODE_READ = 1 };

int init(info_t *, int, char *);
void clear_all(info_t *);
void clear_data_socket(data_t *);

int fill_sockset(info_t *);
int fill_data_sockset(data_t *);
int fd_isset_data(info_t *);

void listen_data(info_t *, int);
void listen_client(info_t *, int);
void execute(info_t *, int);

void add_client(info_t *);
int create_data_socket(info_t *, int);
int create_data_buffer(info_t *, int);
void change_username(client_t *, char *);
int login(client_t *, char *, char *);
void disconnect_client(client_t *);

unsigned long get_ip_address();
int send_list(info_t *, int, int, char *, char *);
int delete_file(char *, char *);
int send_file(info_t *, int, int, char *, char *);
int recv_file(info_t *, int, char *, char *);

int change_wide_directory(char **, char *);
int change_directory(char **, char *);
int checkFolder(char *, char *);
int create_directory(char *);
void my_strcat(char **, char *);
char *substr(char *);
char *merge_paths(char *, char *);

#endif
