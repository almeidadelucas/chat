#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// Método responsável por receber e logar as mensagens recebidas via TCP
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

// Método responsável por receber e logar as mensagens recebidas via UDP
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

    // cria os 2 tipos de sockets que serão utilizados
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(0); // Porta dinâmica
    local_addr.sin_addr.s_addr = INADDR_ANY;

    // faz o bind para o socket UDP
    if (bind(udp_sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("Erro ao vincular socket UDP");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // se conecta ao socket TCP
    connect(tcp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // solicitação do nome de usuário e repasse para o servidor
    printf("Digite seu nickname: ");
    char nickname[32];
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = '\0';
    send(tcp_sock, nickname, strlen(nickname), 0);

    socklen_t len = sizeof(local_addr);
    getsockname(udp_sock, (struct sockaddr*)&local_addr, &len); // Resgata a porta atribuída dinamicamente

    // Envio da porta UDP para o servidor
    char udp_port[16];
    sprintf(udp_port, "%d", ntohs(local_addr.sin_port)); // Porta atribuída dinamicamente
    send(tcp_sock, udp_port, strlen(udp_port), 0);

    // Inicia 2 threads, uma para receber as mensages do servidor via UDP e outra para receber via TCP
    pthread_t tcp_thread, udp_thread;
    pthread_create(&tcp_thread, NULL, receive_tcp, &tcp_sock);
    pthread_create(&udp_thread, NULL, receive_udp, &udp_sock);

    // Aguarda por novas mensagens e faz o envio para o servidor
    char message[BUFFER_SIZE];
    while (1) {
        fgets(message, sizeof(message), stdin);
        send(tcp_sock, message, strlen(message), 0);
    }

    close(tcp_sock);
    return 0;
}
