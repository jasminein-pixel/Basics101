#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int create_server() {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(server, (struct sockaddr *)&address, sizeof(address));
    listen(server, 10);

    return server;
}

int wait_for_client(int server) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    return accept(server, (struct sockaddr *)&address, (socklen_t*)&addrlen);
}

void read_request(int client, char *buffer, int size) {
    memset(buffer, 0, size);
    read(client, buffer, size - 1);
}

void send_response(int client, int result) {
    char response[2048];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n"
        "{\"result\": %d}", result);

    write(client, response, strlen(response));
}

int main() {
    printf("Server starting on port %d\n", PORT);
    printf("Ready to receive requests from frontend\n\n");

    int server = create_server();
    char buffer[4096];

    while (1) {
        int client = wait_for_client(server);
        read_request(client, buffer, sizeof(buffer));

        int a = 0, b = 0;
        if (strstr(buffer, "/add")) {
            sscanf(strstr(buffer, "a="), "a=%d", &a);
            sscanf(strstr(buffer, "b="), "b=%d", &b);
        }

        int result = a + b;

        printf("[AWS] Calculated: %d + %d = %d\n", a, b, result);
        send_response(client, result);

        close(client);
    }

    return 0;
}
