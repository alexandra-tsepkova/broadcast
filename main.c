//Broadcast
//1. 255.255.255.255 local network
//2. 198.168.1.0
//first three numbers (24 bits) = apply mask
//        mask 0.0.0.255
//        get broadcast address  : 192.168.1.255
//send to 255.255.255.255

#include <stdio.h>
#include "libs.h"

char *to_addr (struct sockaddr_in *rec_addr) {
    char *addr = (char *)calloc(25, 1);
    char *inet_addr = calloc(20, 1);
    sprintf(addr, "%s", inet_ntop(AF_INET, (const void *)&(rec_addr->sin_addr.s_addr), inet_addr, 20)); //, (unsigned short)ntohs(rec_addr->sin_port));
    free(inet_addr);
    return addr;
}


char *get_path(char *key, char **table) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (table[i] == NULL) {
            table[i] = calloc(1, sizeof(key) + 1);
            strcpy(table[i], key);
            table[TABLE_SIZE + i] = calloc(1, PATH_MAX);
            strcpy(table[TABLE_SIZE + i], DEFAULT_PATH);
            return table[TABLE_SIZE + i];
        } else if(strcmp(key, table[i]) == 0) {
            return table[TABLE_SIZE + i];
        }
    }
    exit(1);
}


int main(int argc, char **argv) {
    char **table = calloc(TABLE_SIZE * 2, sizeof(char*));

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0) {
        printf("Can't create socket\n");
        return -1;
    }
    struct in_addr addr;
    addr.s_addr = inet_addr("0.0.0.0");
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(PORT);
    sock_addr.sin_addr = addr;

    if (bind(sock_fd, (const struct sockaddr*)(&sock_addr), (socklen_t)sizeof(sock_addr)) < 0) {
        printf("Can't assign address to a socket\n");
        close(sock_fd);
        return -1;
    }

    int b_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(b_sock_fd < 0) {
        printf("Can't create socket\n");
        return -1;
    }

    int yes = 1;
    setsockopt(b_sock_fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

    struct in_addr b_addr;
    b_addr.s_addr = INADDR_ANY;
    struct sockaddr_in b_sock_addr;
    b_sock_addr.sin_family = AF_INET;
    b_sock_addr.sin_port = htons(B_PORT);
    b_sock_addr.sin_addr = b_addr;

    if (bind(b_sock_fd, (const struct sockaddr*)(&b_sock_addr), (socklen_t)sizeof(b_sock_addr)) < 0) {
        perror("Can't assign address to a socket (broadcast)\n");
        close(b_sock_fd);
        exit(-1);
    }




    while (1) {
        struct sockaddr_in rec_addr;
        char *buf = calloc(MAX_MESSAGE_SIZE, 1);
        int addr_size = sizeof(rec_addr);
        int res = recvfrom(sock_fd, buf, MAX_MESSAGE_SIZE, 0, (struct sockaddr * restrict)&rec_addr, (socklen_t * restrict)&addr_size);
        if (res < 0) {
            printf("Can't read message\n");
            return (-1);
        }
        char *key = to_addr(&rec_addr);
        printf("%s\n", key);
        puts(buf);

        if(strncmp(buf, "cd", 2) == 0) {
            char *path = get_path(key, table);
            char *new_path = calloc(1, PATH_MAX);
            char *cur_path = calloc(1, PATH_MAX);
            strcpy(new_path, (buf + 3));
            fflush(stdout);
            if(chdir(path) != 0){
                perror("Can't go to this directory\n");
                return -1;
            }
            if(chdir(new_path) != 0){
                perror("Can't go to this directory\n");
            }
            getcwd(cur_path, PATH_MAX);
            printf("new path: %s, old_path: %s\n", cur_path, path);
            strcpy(path, cur_path);
            free(new_path);

        } else if(strncmp(buf, "ls", 2) == 0) {
            char *path = get_path(key, table);
            DIR *dir = NULL;
            struct dirent *this_dir;
            dir = opendir(path);
            if(!dir) {
                perror("Can't read directory\n");
                exit(-1);
            }
            while((this_dir = readdir(dir)) != NULL) {
                printf("%s\n", this_dir->d_name);
            }
            if(closedir(dir) != 0){
                perror("Can't close directory\n");
                exit(-1);
            }
        }
        free(buf);
        free(key);
    }
    close(sock_fd);
    printf("\nDisconnected\n");
}

