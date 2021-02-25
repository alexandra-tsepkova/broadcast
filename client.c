#include "libs.h"

int main(int argc, char **argv) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        printf("Can't create socket");
        return -1;
    }

    const struct sockaddr_in server_addr = {AF_INET, htons(PORT), inet_addr("127.0.0.1")};

    struct in_addr b_addr;
    b_addr.s_addr = INADDR_BROADCAST;
    struct sockaddr_in b_sock_addr;
    b_sock_addr.sin_family = AF_INET;
    b_sock_addr.sin_port = htons(B_PORT);
    b_sock_addr.sin_addr = b_addr;


    if(argc < 2) {
        printf("Not enough arguments\n");
        return -1;
    }

    char *buf = calloc(MAX_MESSAGE_SIZE, 1);
    strcpy(buf, argv[1]);
    if(argv[2]) {
        strcat(buf, " ");
        strcat(buf, argv[2]);
    }

    int b_sent = sendto(sock_fd, "Is anybody here?", strlen("Is anybody here?"), 0, (struct sockaddr*)&b_addr, sizeof(b_addr));
    if(b_sent < 0) {
        printf("Error while sending (broadcast)\n");
        return -1;
    }

    int sent = sendto(sock_fd, buf, strlen(buf), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(sent < 0) {
        printf("Error while sending\n");
        return -1;
    }

    close(sock_fd);
    return 0;
}