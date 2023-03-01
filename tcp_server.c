#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8989
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // set address for socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d\n", PORT);

    // accept incoming connection and read data
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen))) {
        char filename[BUFFER_SIZE] = {0};
        int filesize;
        FILE *fp;

        // read filename and filesize from client
        valread = read(new_socket, filename, BUFFER_SIZE);
        if (valread < 0) {
            perror("read filename failed");
            exit(EXIT_FAILURE);
        }
        valread = read(new_socket, &filesize, sizeof(int));
        if (valread < 0) {
            perror("read filesize failed");
            exit(EXIT_FAILURE);
        }

        // create file with filename and save incoming data to file
        fp = fopen(filename, "wb");
        if (!fp) {
            perror("open file failed");
            exit(EXIT_FAILURE);
        }

        int bytes_received = 0;
        while (bytes_received < filesize) {
            int bytes_to_read = BUFFER_SIZE;
            if (bytes_received + bytes_to_read > filesize) {
                bytes_to_read = filesize - bytes_received;
            }
            valread = read(new_socket, buffer, bytes_to_read);
            if (valread < 0) {
                perror("read data failed");
                exit(EXIT_FAILURE);
            }
            fwrite(buffer, sizeof(char), valread, fp);
            bytes_received += valread;
        }

        fclose(fp);
        printf("Received file %s (%d bytes)\n", filename, filesize);
    }

    return 0;
}
