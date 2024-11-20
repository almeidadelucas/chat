#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define TCP_PORT 8080
#define UDP_PORT 8081
#define BUFFER_SIZE 1024

void *receive_udp_notifications(void *arg) {
    int udp_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        recv(udp_socket, buffer, sizeof(buffer), 0);
        printf("Notification: %s\n", buffer);
    }
    return NULL;
}

int main() {
    int tcp_socket, udp_socket;
    struct sockaddr_in tcp_addr, udp_addr;
    char buffer[BUFFER_SIZE];
    char nickname[50];

    printf("Enter your nickname: ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = '\0';

    // Configuração do socket TCP
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(TCP_PORT);
    inet_pton(AF_INET, "127.0.0.1", &tcp_addr.sin_addr);
    connect(tcp_socket, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr));

    // Enviar nickname e dados do cliente
    send(tcp_socket, nickname, strlen(nickname), 0);

    // Configuração do socket UDP
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr));

    send(tcp_socket, &udp_addr, sizeof(udp_addr), 0);

    // Thread para receber notificações UDP
    pthread_t udp_thread;
    pthread_create(&udp_thread, NULL, receive_udp_notifications, &udp_socket);

    // Receber e enviar mensagens de bate-papo
    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        send(tcp_socket, buffer, strlen(buffer), 0);
    }

    close(tcp_socket);
    close(udp_socket);
    return 0;
}
