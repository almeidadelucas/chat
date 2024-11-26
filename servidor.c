#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

const char *colors[] = {
    "\033[0;31m", // Vermelho
    "\033[0;32m", // Verde
    "\033[0;33m", // Amarelo
    "\033[0;34m", // Azul
    "\033[0;35m", // Magenta
    "\033[0;36m"  // Ciano
};

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
    char nickname[32];
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;

void broadcast_tcp(const char *message, int exclude_sockfd) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].sockfd != exclude_sockfd) {
            send(clients[i].sockfd, message, strlen(message), 0);
        }
    }
}

void broadcast_udp(const char *message, struct sockaddr_in *udp_addr, int udp_sock) {
    for (int i = 0; i < client_count; i++) {
        sendto(udp_sock, message, strlen(message), 0,
               (struct sockaddr*)&clients[i].addr, sizeof(clients[i].addr));
    }
}

void remove_client(int sockfd) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].sockfd == sockfd) {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
}

int main() {
    int server_sock, udp_sock, max_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    fd_set read_fds, all_fds;

    // Configuração do servidor TCP
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_CLIENTS);

    // Configuração do servidor UDP
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_port = htons(8081);
    bind(udp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Inicializa o conjunto de descritores
    FD_ZERO(&all_fds);
    FD_SET(server_sock, &all_fds);
    FD_SET(udp_sock, &all_fds);
    max_fd = server_sock > udp_sock ? server_sock : udp_sock;

    printf("Servidor iniciado na porta 8080 (TCP) e 8081 (UDP)...\n");

    while (1) {
        read_fds = all_fds;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Erro no select");
            exit(EXIT_FAILURE);
        }

        // Verifica novas conexões TCP
        if (FD_ISSET(server_sock, &read_fds)) {
            int new_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
            if (client_count >= MAX_CLIENTS) {
                printf("Máximo de clientes atingido.\n");
                close(new_sock);
                continue;
            }

            // Recebe o nickname do cliente
            char nickname[32];
            recv(new_sock, nickname, sizeof(nickname), 0);

            // Recebe o endereço UDP do cliente
            char udp_port[16];
            recv(new_sock, udp_port, sizeof(udp_port), 0);
            
            // Adiciona o cliente à lista
            clients[client_count].sockfd = new_sock;
            clients[client_count].addr = client_addr;
            // Configura o endereço UDP do cliente
            clients[client_count].addr.sin_port = htons(atoi(udp_port));
            strcpy(clients[client_count].nickname, nickname);
            client_count++;

            FD_SET(new_sock, &all_fds);
            if (new_sock > max_fd) max_fd = new_sock;

            // Notifica os clientes via UDP
            char notification[BUFFER_SIZE];
            sprintf(notification, "Novo cliente conectado: %s\n", nickname);
            printf("%s", notification);
            broadcast_udp(notification, &client_addr, udp_sock);
        }

        // Verifica mensagens dos clientes
        for (int i = 0; i < client_count; i++) {
            int sockfd = clients[i].sockfd;

            if (FD_ISSET(sockfd, &read_fds)) {
                char buffer[BUFFER_SIZE];
                int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

                if (bytes_received <= 0) {
                    // Cliente desconectou
                    printf("Cliente %s desconectou.\n", clients[i].nickname);
                    char notification[BUFFER_SIZE + 100];
                    sprintf(notification, "Cliente %s desconectou.\n", clients[i].nickname);
                    // Envio da notificação de cliente desconectado
                    broadcast_udp(notification, &clients[i].addr, udp_sock);

                    // Fechamento do socket
                    close(sockfd);
                    FD_CLR(sockfd, &all_fds);

                    // Remoção do cliente da lista
                    remove_client(sockfd);
                } else {
                    buffer[bytes_received] = '\0';

                    char message[BUFFER_SIZE + 100];
                    int color_index = i % 6;

                    // Envio da mensagem recebida para os demais clientes
                    sprintf(message, "%s[%s]:\033[0m %s", colors[color_index], clients[i].nickname, buffer);
                    printf("%s", message);
                    broadcast_tcp(message, sockfd);
                }
            }
        }
    }

    close(server_sock);
    close(udp_sock);
    return 0;
}
