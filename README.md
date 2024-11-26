# Exercício 7: Serviço de Bate-Papo com Notificações de Status

Este projeto implementa um serviço de bate-papo com notificações de status, utilizando os protocolos TCP e UDP para diferentes tipos de mensagens. O trabalho foi desenvolvido em linguagem C.

---

## Funcionalidades

1. **Servidor Centralizado de Bate-Papo (TCP):**
   - Gerencia a comunicação entre os clientes.
   - Encaminha mensagens de um cliente para outro utilizando o protocolo TCP para garantir entrega confiável.

2. **Notificações de Status (UDP):**
   - Envia notificações rápidas, como entrada e saída de usuários, para todos os clientes via UDP.
   - Inclui o nickname do cliente em cada notificação.

3. **Lista de Usuários Conectados:**
   - Cada cliente recebe uma lista atualizada de usuários conectados ao entrar no chat.
   - A lista é mantida sincronizada em tempo real com notificações via UDP.

4. **Nickname do Cliente:**
   - Os clientes escolhem um apelido (nickname) ao se conectar, que é exibido nas mensagens e notificações.

5. **Desconexão:**
   - O cliente pode sair do chat a qualquer momento, e o servidor atualiza a lista de usuários e notifica os outros clientes.

6. **Servidor Concorrente:**
   - Suporta múltiplos clientes simultaneamente, gerenciando mensagens de chat (TCP) e notificações de status (UDP).

7. **Log de Eventos:**
   - O servidor registra eventos importantes, como conexões, desconexões e mensagens trocadas.

8. **Conexão via Endereço IP:**
   - O cliente pode se conectar ao servidor usando o endereço IP.

---

## Requisitos do Ambiente

- GCC (compilador C com suporte à flag `-Wall` para análise de warnings).
- Sistema operacional Linux ou Unix.
- Acesso ao terminal.

---

## Estrutura do Projeto

- **`servidor.c`**: Código do servidor responsável pelo gerenciamento centralizado de mensagens e notificações.
- **`cliente.c`**: Código do cliente que se conecta ao servidor e interage no bate-papo.

---

## Instruções de Compilação

1. Compile o cliente e servidor via makebuild:
   ```bash
   make

---

## Instruções de Execução
1. Inicie o servidor:
   ```bash
   ./servidor
2. Inicie quantos clientes forem necessários:
   ```bash   
   ./cliente
   ```
   - Insira o nickname quando solicitado.
   - Envie mensagens via chat (TCP).
   - Receba notificações de status de outros usuários via UDP.
