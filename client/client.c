/**
 * @file client.c
 * @brief A simple client program to download files from a server.
 *
 * This program connects to a server and requests a file to download.
 * The downloaded file is saved in the specified output folder.
 *
 * Usage: client -c <server_ip:port> -f <filename> -o <output_folder>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <getopt.h>
#include <stdint.h> // Include for portable integer types


int main(int argc, char *argv[])
{
    const char *server_ip = NULL;
    int32_t server_port = -1;
    const char *filename = NULL;
    const char *output_folder = NULL; // New argument for the output folder

    int opt;
    while ((opt = getopt(argc, argv, "c:f:o:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            server_ip = optarg;
            char *port_str = strchr(optarg, ':');
            if (port_str != NULL)
            {
                *port_str = '\0';
                port_str++;
                server_port = atoi(port_str);
            }
            break;
        case 'f':
            filename = optarg;
            break;
        case 'o':
            output_folder = optarg; // Store the output folder
            break;
        default:
            fprintf(stderr, "Usage: %s -c <server_ip:port> -f <filename> -o <output_folder>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (server_ip == NULL || filename == NULL || output_folder == NULL)
    {
        fprintf(stderr, "Usage: %s -c <server_ip:port> -f <filename> -o <output_folder>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int32_t client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send the filename to the server
    if (send(client_socket, filename, strlen(filename), 0) < 0)
    {
        perror("Error sending filename");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Create and open the file in the specified output folder
    char output_path[256];
    snprintf(output_path, sizeof(output_path), "%s/%s", output_folder, filename);

    FILE *file = NULL;
    int32_t dataReceived = 0;

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int32_t bytesRead = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytesRead < 0)
        {
            perror("Error receiving file");
            break;
        }
        else if (bytesRead == 0)
        {
            // Zero bytes received, indicating the file does not exist
            break;
        }
        else if (file == NULL)
        {
            // Open the file in the specified output folder for writing only when data is received
            file = fopen(output_path, "wb");
            if (file == NULL)
            {
                perror("Error opening file");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }

        fwrite(buffer, 1, bytesRead, file);
        dataReceived = 1;
    }

    if (file != NULL)
    {
        fclose(file);
    }
    else
    {
        printf("File not found on the server: %s\n", filename);
    }

    close(client_socket);

    if (dataReceived)
    {
        printf("File received: %s\n", filename);
    }

    return 0;
}
