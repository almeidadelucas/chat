#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define TCP_PORT 8080
#define UDP_PORT 8081
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int tcp_socket;
    struct sockaddr_in udp_address;
    char nickname[50];
} Client;

// Função para enviar uma mensagem UDP para todos os clientes
void send_udp_notification(Client clients[], int client_count, const char *message, int udp_socket) {
    // for (int i = 0; i < client_count; i++) {
    send(udp_socket, message, strlen(message), 0);
    // }
}

int main() {
    int tcp_socket, udp_socket;
    struct sockaddr_in tcp_addr, udp_addr;
    fd_set read_fds;
    Client clients[MAX_CLIENTS];
    int client_count = 0;
    char buffer[BUFFER_SIZE];

    // Configuração do socket TCP
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(TCP_PORT);
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    bind(tcp_socket, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr));
    listen(tcp_socket, MAX_CLIENTS);

    // Configuração do socket UDP
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr));

    printf("Servidor iniciado. Aguardando conexões...\n");

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(tcp_socket, &read_fds);
        int max_fd = tcp_socket;

        // Adicionar sockets de clientes ao conjunto de leitura
        for (int i = 0; i < client_count; i++) {
            FD_SET(clients[i].tcp_socket, &read_fds);
            if (clients[i].tcp_socket > max_fd) max_fd = clients[i].tcp_socket;
        }

        // Esperar por atividade em qualquer socket
        select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        // Novo cliente TCP
        if (FD_ISSET(tcp_socket, &read_fds)) {
            int new_socket = accept(tcp_socket, NULL, NULL);
            if (client_count < MAX_CLIENTS) {
                clients[client_count].tcp_socket = new_socket;
                char nickname[50];
                int n;
                if ( (n = read(new_socket, nickname, 49)) > 0) {
                    nickname[n] = 0;
                    printf("Mensagem do cliente: %s\n", nickname);
                    snprintf(clients[client_count].nickname, 50, "%s", nickname);
                    // clients[client_count].nickname = nickname;
                }
                // recv(new_socket, clients[client_count].nickname, sizeof(clients[client_count].nickname), 0);
                
                // Receber informações de UDP do cliente
                // recv(new_socket, &clients[client_count].udp_address, sizeof(clients[client_count].udp_address), 0);
                
                // Notificar todos os clientes sobre a nova conexão
                snprintf(buffer, BUFFER_SIZE, "User %s has joined.\n", clients[client_count].nickname);
                send_udp_notification(clients, client_count, buffer, udp_socket);
                
                client_count++;
            } else {
                close(new_socket);
            }
        }

        // Verificar mensagens de cada cliente TCP
        for (int i = 0; i < client_count; i++) {
            if (FD_ISSET(clients[i].tcp_socket, &read_fds)) {
                int bytes_received = recv(clients[i].tcp_socket, buffer, sizeof(buffer), 0);
                if (bytes_received <= 0) {
                    // Cliente desconectado
                    close(clients[i].tcp_socket);
                    snprintf(buffer, BUFFER_SIZE, "User %s has left.\n", clients[i].nickname);
                    send_udp_notification(clients, client_count, buffer, udp_socket);

                    // Remover o cliente da lista
                    for (int j = i; j < client_count - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    client_count--;
                    i--;
                } else {
                    // Encaminhar a mensagem para outros clientes
                    buffer[bytes_received] = '\0';
                    for (int j = 0; j < client_count; j++) {
                        if (j != i) send(clients[j].tcp_socket, buffer, bytes_received, 0);
                    }
                }
            }
        }
    }

    close(tcp_socket);
    close(udp_socket);
    return 0;
}
