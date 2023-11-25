#include <arpa/inet.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define PORT 3030
#define MAX_BUFFER_SIZE 1024

int main() {
    void *lib_handle = dlopen("/usr/lib/libkipr.so", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Error loading libkipr.so: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    auto (*gyro_x)() = (signed short (*)()) dlsym(lib_handle, "gyro_x");
    if (!gyro_x) {
        fprintf(stderr, "Error getting gyro_x function: %s\n", dlerror());
        dlclose(lib_handle);
        exit(EXIT_FAILURE);
    }


    // Create socket
    int server_socket, client_socket;
    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (true) {
        // Accept incoming connection
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            continue;
        }

        // Read the HTTP request
        char buffer[MAX_BUFFER_SIZE];
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("Error reading from client");
            close(client_socket);
            continue;
        }

        // Check if it's a GET request for "/gyro_x"
        if (strstr(buffer, "GET /gyro_x") != nullptr) {
            // Call gyro_x and send the result as the response
            signed short gyro_x_result = gyro_x();
            char response[MAX_BUFFER_SIZE];
            snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%d", strlen(response),
                     gyro_x_result);
            write(client_socket, response, strlen(response));
        } else {
            // Respond with 404 Not Found for other requests
            char response[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            write(client_socket, response, sizeof(response) - 1);
        }

        // Close the client socket
        close(client_socket);
    }

    // Close the library and the server socket
    dlclose(lib_handle);
    close(server_socket);

    return 0;
}
