/*
** EPITECH PROJECT, 2019
** boring project
** File description:
** Header File
*/

#include "my_ftp.h"

int change_wide_directory(char **path, char *target) {
    int ret, decay = 0;
    char *temp;

    while (target[decay] == '/')
        decay++;

    if (decay != 0)
        temp = strdup("/");
    else
        temp = strdup(*path);

    ret = change_directory(&temp, &target[decay]);

    if (ret == 0)
        free(temp);
    else {
        free(*path);
        *path = temp;
    }

    return ret;
}

int change_directory(char **path, char *target)
{
    int folder_len = 1;
    char *folder;
    char *p;

    if (strlen(target) == 0)
        return 1;

    folder = substr(target);

    if (folder) {
        if (strcmp(folder, "..") == 0) {
            if (strlen(*path) != 1) {
                p = strrchr(*path, '/');

                if (p - *path != 0)
                    p[0] = '\0';
                else
                    p[1] = '\0';
            }
        } else if (strcmp(folder, ".") != 0) {
            if (checkFolder(*path, folder)) {

                if ((*path)[strlen(*path) - 1] != '/')
                    my_strcat(path, "/");
                my_strcat(path, folder);
            } else {
                free(folder);
                return 0;
            }
        }
        folder_len = strlen(folder);
        free(folder);
    }

    return change_directory(path, &target[folder_len]);
}

int checkFolder(char *path, char *folder)
{
    DIR *dir;
    struct dirent *file;
    struct stat st;
    char *st_path;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir failed");
        return 0;
    }
    file = readdir(dir);

    while (file) {
        if (strcmp(file->d_name, folder) == 0) {

            st_path = merge_paths(path, file->d_name);

            if (stat(st_path, &st) == -1) {
                perror("stat failed");
                free(st_path);
                file = readdir(dir);
                continue;
            }
            free(st_path);

            if (S_ISDIR(st.st_mode)) {
                closedir(dir);
                return 1;
            } else {
                closedir(dir);
                return 0;
            }
        }
        file = readdir(dir);
    }
    closedir(dir);

    return 0;
}

int create_directory(char *path) {
    struct stat st;

    if (stat(path, &st) == -1) {
        if (mkdir(path, 0777) == -1)
            return 0;
        else
            return 1;
    }

    return 2;
}

char *substr(char *target)
{
    char *found = strchr(target, '/');
    int len = (int)(strchr(target, '/') - target);
    char *str;

    if (found == NULL)
        len = strlen(target);
    else
        len = (int)(found - target);

    if (len == 0)
        return NULL;

    str = malloc(len + 1);
    str[len] = '\0';
    strncpy(str, target, len);

    return str;
}

void my_strcat(char **s1, char *s2)
{
    int len = strlen(*s1) + strlen(s2);
    char *str = malloc(len + 1);


    strcpy(str, *s1);
    strcpy(&str[strlen(*s1)], s2);
    str[len] = '\0';

    if (*s1)
        free(*s1);
    *s1 = str;
}

char *merge_paths(char *path, char *file) {
    int len = strlen(path) + strlen(file);
    char *file_path;

    if (file[0] == '/')
        file_path = strdup(file);
    else if (path[strlen(path) - 1] == '/') {
        file_path = malloc(len + 1);
        strcpy(file_path, path);
        strcpy(&file_path[strlen(path)], file);
        file_path[len] = '\0';
    } else {
        file_path = malloc(len + 2);
        strcpy(file_path, path);
        strcpy(&file_path[strlen(path)], "/");
        strcpy(&file_path[strlen(path) + 1], file);
        file_path[len + 1] = '\0';
    }

    return file_path;
}
