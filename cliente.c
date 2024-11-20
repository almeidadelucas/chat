#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

void *receive_tcp(void *sock) {
    int sockfd = *(int*)sock;
    char buffer[BUFFER_SIZE];

    while (1) {
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Desconectado do servidor.\n");
            exit(0);
        }
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }
}

void *receive_udp(void *sock) {
    int udp_sock = *(int*)sock;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    while (1) {
        int bytes_received = recvfrom(udp_sock, buffer, sizeof(buffer) - 1, 0, 
                                      (struct sockaddr*)&server_addr, &addr_len);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
        }
    }
}

int main() {
    int tcp_sock, udp_sock;
    struct sockaddr_in server_addr;

    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(tcp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Digite seu nickname: ");
    char nickname[32];
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = '\0';
    send(tcp_sock, nickname, strlen(nickname), 0);

    pthread_t tcp_thread, udp_thread;
    pthread_create(&tcp_thread, NULL, receive_tcp, &tcp_sock);
    pthread_create(&udp_thread, NULL, receive_udp, &udp_sock);

    char message[BUFFER_SIZE];
    while (1) {
        // printf("\033[1;37meu: \033[0m");
        fgets(message, sizeof(message), stdin);
        send(tcp_sock, message, strlen(message), 0);
    }

    close(tcp_sock);
    return 0;
}
