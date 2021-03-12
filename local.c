/*
** EPITECH PROJECT, 2019
** boring project
** File description:
** Header File
*/

#include "my_ftp.h"

unsigned long get_ip_address() {
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs *ifa = NULL;
    unsigned long ip = 16777343;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {

        if (strcmp(ifa->ifa_name, "lo") != 0 && ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            ip = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;

            if (ifAddrStruct != NULL)
                freeifaddrs(ifAddrStruct);
            return ip;
        }
    }
    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);

    return ip;
}

int send_list(info_t *info, int fd, int id, char *main_path, char *file_path) {
    DIR *dir;
    struct dirent *file;
    struct passwd *pws;
    struct group *group;
    struct tm *date;
    struct stat st;
    time_t time;
    char *path, *base_path;

    char *const months[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    if (file_path)
        base_path = merge_paths(main_path, file_path);
    else
        base_path = main_path;

    dir = opendir(base_path);
    if (dir == NULL) {
        perror("opendir failed");

        if (file_path)
            free(base_path);
        return 0;
    }
    file = readdir(dir);

    if (fd == 0)
        fd = create_data_buffer(info, id);

    while (file) {

        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
            file = readdir(dir);
            continue;
        }

        path = merge_paths(base_path, file->d_name);

        if (stat(path, &st) ==  -1) {
            perror("stat failed");
            free(path);
            file = readdir(dir);
            continue;
        }
        free(path);

        pws = getpwuid(st.st_uid);
        group = getgrgid(st.st_uid);
        time = (time_t)st.st_mtimespec.tv_sec;
        date = localtime(&time);

        dprintf(fd, S_ISDIR(st.st_mode) ? "d" : "-");
        dprintf(fd, (st.st_mode & S_IRUSR) ? "r" : "-");
        dprintf(fd, (st.st_mode & S_IWUSR) ? "w" : "-");
        dprintf(fd, (st.st_mode & S_IXUSR) ? "x" : "-");
        dprintf(fd, (st.st_mode & S_IRGRP) ? "r" : "-");
        dprintf(fd, (st.st_mode & S_IWGRP) ? "w" : "-");
        dprintf(fd, (st.st_mode & S_IXGRP) ? "x" : "-");
        dprintf(fd, (st.st_mode & S_IROTH) ? "r" : "-");
        dprintf(fd, (st.st_mode & S_IWOTH) ? "w" : "-");
        dprintf(fd, (st.st_mode & S_IXOTH) ? "x" : "-");
        dprintf(fd, "\t%hu ", st.st_nlink);
        dprintf(fd, "%s\t%s\t%lld\t", pws->pw_name, group->gr_name, st.st_size);
        dprintf(fd, "%s %02d  %d ", months[date->tm_mon], date->tm_mday, date->tm_year + 1900);
        dprintf(fd, "%s\r\n", file->d_name);
        file = readdir(dir);
    }
    closedir(dir);

    if (file_path)
        free(base_path);

    return 1;
}

int delete_file(char *path, char *file) {
    char *file_path = merge_paths(path, file);

    if (remove(file_path) == 0) {
        free(file_path);
        return 1;
    } else {
        free(file_path);
        return 0;
    }
}

int send_file(info_t *info, int fd, int id, char *pwd, char *file_path) {
    char *path = merge_paths(pwd, file_path);
    char *buff;
    FILE *file;
    long size;
    struct stat st;

    if (stat(path, &st) == -1 || S_ISDIR(st.st_mode))
        return 0;

    file = fopen(path, "rb");
    info->client[id].data_socket.mode = MODE_WRITE;

    if (file == NULL)
        return 0;

    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    buff = malloc(size + 1);
    buff[size] = '\0';

    fread(buff, size, 1, file);
    fclose(file);

    if (fd == 0)
        fd = create_data_buffer(info, id);

    write(fd, buff, size);
    free(buff);

    return 1;
}


int recv_file(info_t *info, int id, char *main_path, char *file_path) {
    char *path = merge_paths(main_path, file_path);

    info->client[id].data_socket.mode = MODE_READ;
    info->client[id].data_socket.buffer_data = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);

    if (info->client[id].data_socket.buffer_data == -1) {
        info->client[id].data_socket.buffer_data = 0;
        return 0;
    }

    return 1;
}
