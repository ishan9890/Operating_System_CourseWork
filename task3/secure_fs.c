#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERS 5
#define MAX_LEN 50

typedef struct {
    char username[MAX_LEN];
    unsigned long password_hash;
} User;

User users[MAX_USERS];
int user_count = 0;
char current_user[MAX_LEN];

//Simple hash function 
//used here to demonstrate the hashing concept
unsigned long hash_password(const char* password){
    unsigned long hash = 5381;
    int c;

    while ((c = *password++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void register_user(const char* username, const char* password){
    if(user_count >= MAX_USERS){
        printf("User limit reached. Cannot register more users.\n");
        return;
    }
    strcpy(users[user_count].username, username);
    users[user_count].password_hash = hash_password(password);
    user_count++;
    printf("User %s registered successfully.\n", username);
}

int login(const char* username, const char* password){
    unsigned long entered_hash = hash_password(password);
    for(int i = 0; i < user_count; i++){
        if(strcmp(users[i].username, username) == 0){
            if (users[i].password_hash == entered_hash){
                strcpy(current_user, username);
                printf("Login successful. Welcome, %s!\n", username);
                return 1;
            }else{
                printf("Login failed: incorrect password.\n");
                return 0;
            }
        }
    }
    printf("Login failed: user not found.\n");
    return 0;
}

void log_action(const char* username, const char* action, const char* filename){
    FILE* log = fopen("audit.log", "a");
    if(log == NULL){
        printf("Warning: could not write to audit log.\n");
        return;
    }

    time_t now = time(NULL);
    char* timestamp = ctime(&now);
    timestamp[strlen(timestamp) -1] = '\0';

    fprintf(log, "[%s] User '%s' performed '%s' on file '%s' \n",
            timestamp, username, action, filename );

    fclose(log);
}

void create_file(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        printf("Error: could not create file '%s'.\n", filename);
        return;
    }
    fclose(f);
    printf("File '%s' created successfully.\n", filename);
    log_action(current_user, "CREATE", filename);
}

void write_file(const char* filename, const char* content) {
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        printf("Error: could not open file '%s' for writing.\n", filename);
        return;
    }
    fprintf(f, "%s", content);
    fclose(f);
    printf("Content written to '%s'.\n", filename);
    log_action(current_user, "WRITE", filename);
}

void read_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        printf("Error: could not open file '%s' for reading. Does it exist?\n", filename);
        return;
    }
    printf("--- Contents of '%s' ---\n", filename);
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    printf("\n--- End of file ---\n");
    fclose(f);
    log_action(current_user, "READ", filename);
}

void delete_file(const char* filename) {
    if (remove(filename) == 0) {
        printf("File '%s' deleted successfully.\n", filename);
        log_action(current_user, "DELETE", filename);
    } else {
        printf("Error: could not delete '%s'. Does it exist?\n", filename);
    }
}

int main() {
    //Demo users
    register_user("ishan", "password123");
    register_user("faker", "faker123");

    char username[MAX_LEN], password[MAX_LEN];
    int logged_in = 0;
    int attemqpts = 0;

    printf("Welcome to the Secure File System\n");

    while(!logged_in && attemqpts < 3){
        printf("Enter username: ");
        scanf("%s", username);
        printf("Enter password: ");
        scanf("%s", password);

        logged_in = login(username, password);
        attemqpts++;

        if(!logged_in && attemqpts < 3){
            printf("Attempts remaining: %d\n", 3 - attemqpts);
        }
    }

    if(!logged_in){
        printf("Maximum login attempts reached. Exiting.\n");
        return 1;
    }

    printf("\nAuthenticated as: %s\n", current_user);
    
    int choice;
    char filename[MAX_LEN], content[256];

    do {
        printf("\n=== File Menu ===\n");
        printf("1. Create file\n");
        printf("2. Write to file\n");
        printf("3. Read file\n");
        printf("4. Delete file\n");
        printf("5. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter filename: ");
                scanf("%s", filename);
                create_file(filename);
                break;
            case 2:
                printf("Enter filename: ");
                scanf("%s", filename);
                printf("Enter content: ");
                scanf(" %[^\n]", content);  // reads a full line including spaces
                write_file(filename, content);
                break;
            case 3:
                printf("Enter filename: ");
                scanf("%s", filename);
                read_file(filename);
                break;
            case 4:
                printf("Enter filename: ");
                scanf("%s", filename);
                delete_file(filename);
                break;
            case 5:
                printf("Exiting. Goodbye, %s.\n", current_user);
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 5);

    return 0;

}