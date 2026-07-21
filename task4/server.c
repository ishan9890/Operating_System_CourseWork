#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_USERS 5
#define MAX_LEN 50

typedef struct {
    char username[MAX_LEN];
    unsigned long password_hash;
} User;

User users[MAX_USERS];
int user_count = 0;

unsigned long hash_password(const char* password) {
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void register_user(const char* username, const char* password) {
    strcpy(users[user_count].username, username);
    users[user_count].password_hash = hash_password(password);
    user_count++;
}

int check_credentials(const char* username, const char* password) {
    unsigned long entered_hash = hash_password(password);
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            users[i].password_hash == entered_hash) {
            return 1;
        }
    }
    return 0;
}

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int authenticated = 0;
    char logged_in_user[MAX_LEN] = "";

    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        buffer[strcspn(buffer, "\n")] = '\0';

        char response[BUFFER_SIZE];

        // Validation 1: reject empty input
        if (strlen(buffer) == 0) {
            snprintf(response, BUFFER_SIZE, "ERROR Empty command\n");
            write(client_fd, response, strlen(response));
            continue;
        }

        printf("Received: %s\n", buffer);

        if (strncmp(buffer, "LOGIN", 5) == 0) {
    char username[MAX_LEN] = "", password[MAX_LEN] = "";
    int parsed = 0;

    if (strlen(buffer) > 5) {
        parsed = sscanf(buffer + 5, "%49s %49s", username, password);
    }

    if (parsed != 2) {
        snprintf(response, BUFFER_SIZE, "ERROR Usage: LOGIN <username> <password>\n");
    } else if (check_credentials(username, password)) {
        authenticated = 1;
        strcpy(logged_in_user, username);
        snprintf(response, BUFFER_SIZE, "OK Login successful for %s\n", username);
    } else {
        snprintf(response, BUFFER_SIZE, "ERROR Invalid credentials\n");
    }

} else if (strncmp(buffer, "MSG", 3) == 0) {
    if (!authenticated) {
        snprintf(response, BUFFER_SIZE, "ERROR Please LOGIN before sending messages\n");
    } else if (strlen(buffer) <= 4) {   // "MSG" alone, or "MSG " with nothing after
        snprintf(response, BUFFER_SIZE, "ERROR Usage: MSG <text>\n");
    } else {
        snprintf(response, BUFFER_SIZE, "OK [%s] Message received: %s\n",
                 logged_in_user, buffer + 4);
    }
} else if (strcmp(buffer, "QUIT") == 0) {
    snprintf(response, BUFFER_SIZE, "OK Goodbye\n");
    write(client_fd, response, strlen(response));
    break;

} else {
    snprintf(response, BUFFER_SIZE, "ERROR Unknown command. Valid commands: LOGIN, MSG, QUIT\n");
}

        write(client_fd, response, strlen(response));
    }

    if (bytes_read == 0) {
        printf("Client disconnected gracefully.\n");
    } else if (bytes_read < 0) {
        perror("Read error");
    }

    close(client_fd);
}
void* client_thread(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);  
    handle_client(client_fd);
    return NULL;
}
int main() {
    register_user("ishan", "ishan123");
    register_user("faker", "faker123");

    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {  // loop forever, accepting new clients
        int* client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        if (*client_fd_ptr < 0) {
            perror("Accept failed");
            free(client_fd_ptr);
            continue;  
        }

        printf("Client connected (fd=%d).\n", *client_fd_ptr);

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, client_fd_ptr);
        pthread_detach(tid);  
    }

    close(server_fd);  
    return 0;
}