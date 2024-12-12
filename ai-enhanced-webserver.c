//AI-Enhanced Web Server: A Dynamic, Multi-Threaded Web Server with AI Integration


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define LOG_FILE "server.log"

// List of sensitive words for content filtering
const char *sensitive_words[] = {"password", "secret", "classified", "private", NULL};

// Function to log server activity
void log_request(const char *client_ip, const char *request) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strcspn(time_str, "\n")] = '\0'; // Remove newline character
    fprintf(log_file, "[%s] Client: %s Requested: %s\n", time_str, client_ip, request);
    fclose(log_file);
}

// Function to check if the request contains sensitive words
int contains_sensitive_word(const char *request) {
    for (int i = 0; sensitive_words[i] != NULL; i++) {
        if (strstr(request, sensitive_words[i]) != NULL) {
            return 1; // Sensitive word found
        }
    }
    return 0;
}

// Function to process the client request
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        close(client_socket);
        pthread_exit(NULL);
    }

    buffer[bytes_read] = '\0';
    printf("Received request: %s\n", buffer);

    // Get client IP address
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_size);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    // Log the request
    log_request(client_ip, buffer);

    // Check for sensitive words
    if (contains_sensitive_word(buffer)) {
        const char *blocked_response = 
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "Access Denied: Sensitive content detected.\r\n";
        send(client_socket, blocked_response, strlen(blocked_response), 0);
    } else {
        // Simulated AI response (this can be connected to an actual AI API)
        const char *ai_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<!DOCTYPE html>"
            "<html><head><title>AI-Enhanced Server</title></head>"
            "<body><h1>Welcome to the AI-powered Web Server!</h1>"
            "<p>This page is dynamically generated using AI logic.</p>"
            "</body></html>";
        send(client_socket, ai_response, strlen(ai_response), 0);
    }

    close(client_socket);
    pthread_exit(NULL);
}

// Function to start the server
void start_server() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket == -1) {
            perror("Failed to accept client connection");
            continue;
        }

        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_sock_ptr) != 0) {
            perror("Failed to create thread");
            close(client_socket);
        } else {
            pthread_detach(thread_id);
        }
    }

    close(server_socket);
}

int main() {
    start_server();
    return 0;
}
