#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    return 0;
    
}