#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void save_permissions();
void load_permissions();

#define MAX_USERS 5
#define MAX_LEN 50

typedef struct {
    char username[MAX_LEN];
    unsigned long password_hash;
} User;

#define MAX_FILES 20

typedef struct {
    char filename[MAX_LEN];
    char owner[MAX_LEN];
    int owner_read, owner_write, owner_execute;
    int group_read, group_write, group_execute;
    int others_read, others_write, others_execute;
} FilePermission;

FilePermission file_perms[MAX_FILES];
int file_perm_count = 0;
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

int find_file_perm(const char* filename) {
    for (int i = 0; i < file_perm_count; i++) {
        if (strcmp(file_perms[i].filename, filename) == 0) {
            return i;
        }
    }
    return -1;
}

void register_file_permission(const char* filename, const char* owner) {
    if (file_perm_count >= MAX_FILES) {
        printf("Warning: file permission table full.\n");
        return;
    }
    FilePermission* fp = &file_perms[file_perm_count];
    strcpy(fp->filename, filename);
    strcpy(fp->owner, owner);
    fp->owner_read = 1;  fp->owner_write = 1;  fp->owner_execute = 1;
    fp->group_read = 1;  fp->group_write = 0;  fp->group_execute = 0;
    fp->others_read = 1; fp->others_write = 0;  fp->others_execute = 0;

    file_perm_count++;
    save_permissions();
}

int check_permission(const char* username, const char* filename, char permission_type) {
    int idx = find_file_perm(filename);
    if (idx == -1) {
        printf("Permission error: no permission record for '%s'.\n", filename);
        return 0;
    }

    FilePermission* fp = &file_perms[idx];
    int is_owner = (strcmp(fp->owner, username) == 0);

    if (is_owner) {
        if (permission_type == 'r') return fp->owner_read;
        if (permission_type == 'w') return fp->owner_write;
        if (permission_type == 'x') return fp->owner_execute;
    } else {
        if (permission_type == 'r') return fp->others_read;
        if (permission_type == 'w') return fp->others_write;
        if (permission_type == 'x') return fp->others_execute;
    }
    return 0;
}

void print_permissions(const char* filename) {
    int idx = find_file_perm(filename);
    if (idx == -1) {
        printf("No permission record for '%s'.\n", filename);
        return;
    }
    FilePermission* fp = &file_perms[idx];
    printf("Permissions for '%s' (owner: %s):\n", fp->filename, fp->owner);
    printf("  Owner:  r=%d w=%d x=%d\n", fp->owner_read, fp->owner_write, fp->owner_execute);
    printf("  Group:  r=%d w=%d x=%d\n", fp->group_read, fp->group_write, fp->group_execute);
    printf("  Others: r=%d w=%d x=%d\n", fp->others_read, fp->others_write, fp->others_execute);
}
void save_permissions() {
    FILE* f = fopen("permissions.dat", "w");
    if (f == NULL) {
        printf("Warning: could not save permissions.\n");
        return;
    }
    for (int i = 0; i < file_perm_count; i++) {
        FilePermission* fp = &file_perms[i];
        fprintf(f, "%s %s %d %d %d %d %d %d %d %d %d\n",
                fp->filename, fp->owner,
                fp->owner_read, fp->owner_write, fp->owner_execute,
                fp->group_read, fp->group_write, fp->group_execute,
                fp->others_read, fp->others_write, fp->others_execute);
    }
    fclose(f);
}

void load_permissions() {
    FILE* f = fopen("permissions.dat", "r");
    if (f == NULL) {
        return;
    }

    file_perm_count = 0;
    FilePermission* fp;
    while (file_perm_count < MAX_FILES) {
        fp = &file_perms[file_perm_count];
        int fields = fscanf(f, "%s %s %d %d %d %d %d %d %d %d %d",
               fp->filename, fp->owner,
               &fp->owner_read, &fp->owner_write, &fp->owner_execute,
               &fp->group_read, &fp->group_write, &fp->group_execute,
               &fp->others_read, &fp->others_write, &fp->others_execute);

        if (fields != 11) break;  // stop at end of file or malformed line
        file_perm_count++;
    }
    fclose(f);
}

void create_file(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        printf("Error: could not create file '%s'.\n", filename);
        return;
    }
    fclose(f);
    printf("File '%s' created successfully.\n", filename);
    register_file_permission(filename, current_user); 
    log_action(current_user, "CREATE", filename);
}

void write_file(const char* filename, const char* content) {
    if (!check_permission(current_user, filename, 'w')) {
        printf("Access denied: '%s' does not have write permission on '%s'.\n",
               current_user, filename);
        log_action(current_user, "WRITE-DENIED", filename);
        return;
    }

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
    if (!check_permission(current_user, filename, 'r')) {
        printf("Access denied: '%s' does not have read permission on '%s'.\n",
               current_user, filename);
        log_action(current_user, "READ-DENIED", filename);
        return;
    }

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
    if (!check_permission(current_user, filename, 'w')) {
        printf("Access denied: '%s' does not have write permission on '%s'.\n",
               current_user, filename);
        log_action(current_user, "DELETE-DENIED", filename);
        return;
    }

    if (remove(filename) == 0) {
        printf("File '%s' deleted successfully.\n", filename);
        log_action(current_user, "DELETE", filename);

        int idx = find_file_perm(filename);
        if (idx != -1) {
            for (int i = idx; i < file_perm_count - 1; i++) {
                file_perms[i] = file_perms[i + 1];
            }
            file_perm_count--;
            save_permissions();
        }
    } else {
        printf("Error: could not delete '%s'. Does it exist?\n", filename);
    }
}

#define ENCRYPTION_KEY "MySecretKey2026" 

void xor_encrypt_decrypt(char* data, int length, const char* key) {
    int key_len = strlen(key);
    for (int i = 0; i < length; i++) {
        data[i] = data[i] ^ key[i % key_len];  // cycles through the key repeatedly
    }
}
void encrypt_file(const char* filename) {
    if (!check_permission(current_user, filename, 'w')) {
        printf("Access denied: '%s' does not have write permission on '%s'.\n",
               current_user, filename);
        log_action(current_user, "ENCRYPT-DENIED", filename);
        return;
    }

    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Error: could not open '%s' for encryption.\n", filename);
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = malloc(size);
    fread(buffer, 1, size, f);
    fclose(f);

    xor_encrypt_decrypt(buffer, size, ENCRYPTION_KEY);

    FILE* out = fopen(filename, "wb");
    fwrite(buffer, 1, size, out);
    fclose(out);

    free(buffer);
    printf("File '%s' encrypted successfully.\n", filename);
    log_action(current_user, "ENCRYPT", filename);
}

void decrypt_file(const char* filename) {
    if (!check_permission(current_user, filename, 'r')) {
        printf("Access denied: '%s' does not have read permission on '%s'.\n",
               current_user, filename);
        log_action(current_user, "DECRYPT-DENIED", filename);
        return;
    }

    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Error: could not open '%s' for decryption.\n", filename);
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = malloc(size);
    fread(buffer, 1, size, f);
    fclose(f);

    xor_encrypt_decrypt(buffer, size, ENCRYPTION_KEY);

    FILE* out = fopen(filename, "wb");
    fwrite(buffer, 1, size, out);
    fclose(out);

    free(buffer);
    printf("File '%s' decrypted successfully.\n", filename);
    log_action(current_user, "DECRYPT", filename);
}

int main() {
    load_permissions();   

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
        printf("5. View file permissions\n");
        printf("6. Encrypt file\n");
        printf("7. Decrypt file\n");
        printf("8. Exit\n");
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
                printf("Enter filename: ");
                scanf("%s", filename);
                print_permissions(filename);
                break;
            case 6:
                printf("Enter filename: ");
                scanf("%s", filename);
                encrypt_file(filename);
                break;
            case 7:
                printf("Enter filename: ");
                scanf("%s", filename);
                decrypt_file(filename);
                break;
            case 8:
                printf("Exiting. Goodbye, %s.\n", current_user);
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 8);

    return 0;

}