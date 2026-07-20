#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock_fd;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server.\n");
    printf("Commands: LOGIN <user> <pass> | MSG <text> | QUIT\n");

    while (1) {
        printf("> ");
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) break;

        write(sock_fd, input, strlen(input));

        int bytes_read = read(sock_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            printf("Server closed the connection.\n");
            break;
        }
        buffer[bytes_read] = '\0';
        printf("Server: %s", buffer);

        if (strncmp(input, "QUIT", 4) == 0) {
            break;
        }
    }

    close(sock_fd);
    return 0;
}