/**
 * @file file_server.c
 * @brief A simple file server that serves files to clients.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h> // For standard integer types

#define MAX_CLIENTS 5 // Maximum number of clients that can connect to the server for long-term file sharing

/**
 * @struct ThreadArgs
 * @brief Structure to hold arguments passed to the client-handling thread.
 */
struct ThreadArgs
{
    int32_t client_socket;
    const char *file_directory;
};

/**
 * @brief Handle a client's request to download a file.
 *
 * @param args A pointer to the ThreadArgs structure containing client_socket and file_directory.
 * @return NULL
 */
void *handle_client(void *args)
{
    struct ThreadArgs *thread_args = (struct ThreadArgs *)args;
    int32_t client_socket = thread_args->client_socket;
    const char *file_directory = thread_args->file_directory;
    char buffer[1024];

    // Receive the filename from the client
    memset(buffer, 0, sizeof(buffer));
    if (recv(client_socket, buffer, sizeof(buffer), 0) < 0)
    {
        perror("Error receiving filename");
        close(client_socket);
        free(thread_args);
        return NULL;
    }

    char file_path[2048];
    snprintf(file_path, sizeof(file_path), "%s/%s", file_directory, buffer);

    // Check if the requested file exists
    struct stat file_stat;
    if (stat(file_path, &file_stat) < 0 || !S_ISREG(file_stat.st_mode))
    {
        fprintf(stderr, "File not found or not a regular file: %s\n", buffer);
        close(client_socket);
        free(thread_args);
        return NULL;
    }

    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        close(client_socket);
        free(thread_args);
        return NULL;
    }

    // Send the file to the client
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
        if (bytesRead <= 0)
            break;

        if (send(client_socket, buffer, bytesRead, 0) < 0)
        {
            perror("Error sending file");
            break;
        }
    }

    fclose(file);
    close(client_socket);
    free(thread_args);
    return NULL;
}

/**
 * @brief Main function to start the file server.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return Exit status.
 */
int main(int argc, char *argv[])
{
    const char *server_ip = NULL;
    int32_t port = 0;
    const char *file_directory = NULL;

    // Parse command-line arguments
    for (int32_t i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
        {
            char *server_ip_port = argv[i + 1];
            char *server_ip_str = strtok(server_ip_port, ":");
            char *port_str = strtok(NULL, ":");
            if (server_ip_str != NULL && port_str != NULL)
            {
                server_ip = server_ip_str;
                port = atoi(port_str);
            }
            i++; // Skip the next argument
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
        {
            file_directory = argv[i + 1];
            i++; // Skip the next argument
        }
    }

    if (server_ip == NULL || port <= 0 || file_directory == NULL)
    {
        fprintf(stderr, "Usage: %s -c <server_ip:port> -f <file_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check if the file directory exists
    struct stat dir_stat;
    if (stat(file_directory, &dir_stat) < 0 || !S_ISDIR(dir_stat.st_mode))
    {
        fprintf(stderr, "Directory not found: %s\n", file_directory);
        exit(EXIT_FAILURE);
    }

    int32_t server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d...\n", server_ip, port);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Error accepting connection");
            continue;
        }

        // Get client IP address and port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        int32_t client_port = ntohs(client_addr.sin_port);

        printf("Connection from %s:%d\n", client_ip, client_port);

        struct ThreadArgs *thread_args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        if (thread_args == NULL)
        {
            perror("Error allocating thread arguments");
            close(client_socket);
            continue;
        }

        thread_args->client_socket = client_socket;
        thread_args->file_directory = file_directory;

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void *)thread_args) != 0)
        {
            perror("Error creating thread");
            close(client_socket);
            free(thread_args);
        }

        // Detach the thread to clean up resources automatically
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}
