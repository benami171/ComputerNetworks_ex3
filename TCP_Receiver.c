#include <stdio.h> // Standard input/output library
#include <arpa/inet.h> // For the in_addr structure and the inet_pton function
#include <sys/time.h> // For the gettimeofday function
#include <sys/socket.h> // For the socket function
#include <netinet/in.h> // For the sockaddr_in structure
#include <netinet/tcp.h> // for TCP_CONGESTION
#include <unistd.h> // For the close function
#include <string.h> // For the memset function
#include <stdlib.h> // Standard library
#include <unistd.h> // For the close function
#define CLIENTS 1
#define ADDR "127.0.0.1" // local host

/*
 * @brief The TCP's receiver port.
 * @note The default port is 6000.
*/
#define PORT 6606

/*
 * @brief The maximum number of clients that the receiver can handle.
 * @note The default maximum number of clients is 1.
*/
#define MAX_CLIENTS 1

/*
 * @brief The buffer size to store the received message.
 * @note The default buffer size is 1024.
*/
#define BUFFER_SIZE 8192*4

/*
 * @brief TCP receiver main function.
 * @param None
 * @return 0 if the receiver runs successfully, 1 otherwise.
*/
int main(void) {
    int total_bytes_received = 0;
    int sock = -1;

    struct sockaddr_in receiver;
    struct sockaddr_in sender;
    // Stores the sender's structure length.
    socklen_t sender_len = sizeof(sender);

    int opt = 1;
    memset(&receiver, 0, sizeof(receiver));
    memset(&sender, 0, sizeof(sender));
    // Try to create a TCP socket (IPv4, stream-based, default protocol).
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("socket(2)");
        return 1;
    }

    // Set the socket option to reuse the receiver's address.
    // This is useful to avoid the "Address already in use" error message when restarting the receiver.
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(2)");
        close(sock);
        return 1;
    }

    receiver.sin_addr.s_addr = INADDR_ANY;
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(PORT
);

    // Try to bind the socket to the receiver's address and port.
    if (bind(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0)
    {
        perror("bind(2)");
        close(sock);
        return 1;
    }

    // Try to listen for incoming connections.
    if (listen(sock, 1) < 0)
    {
        perror("listen(2)");
        close(sock);
        return 1;
    }

    fprintf(stdout, "Listening for incoming connections on port %d...\n", PORT
);

    // The receiver's main loop.
    while (1)
    {
        // Try to accept a new sender connection.
        int sender_socket = accept(sock, (struct sockaddr *)&sender, &sender_len);

        // If the accept call failed, print an error message and return 1.
        if (sender_socket < 0)
        {
            perror("accept(2)");
            close(sock);
            return 1;
        }
        fprintf(stdout, "Client %s:%d connected\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));

        char buffer[BUFFER_SIZE] = {0};
        struct timeval start, end;
        gettimeofday(&start, NULL); // Get the current time before receiving the file
        while (1){
            int bytes_received = recv(sender_socket, buffer, BUFFER_SIZE, 0);

            // If the message receiving failed, print an error message and return 1.
            if (bytes_received < 0)
            {
            perror("recv(2)");
            close(sender_socket);
            close(sock);
            return 1;
            }
            // If the amount of received bytes is 0, the sender has disconnected.
            // Close the sender's socket and continue to the next iteration.
            else if (bytes_received == 0)
            {
            fprintf(stdout, "Client %s:%d disconnected\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
            break;
            }
            buffer[bytes_received] = '\0'; // null-terminate the received data
            if (strcmp(buffer, "EXIT") == 0)
            {
            fprintf(stdout, "Received EXIT message from client %s:%d\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
            break;
            }

            total_bytes_received += bytes_received;
            
        }
        gettimeofday(&end, NULL); // Get the current time after receiving the file

        long time_taken = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)) / 1000; // Calculate the time taken in milliseconds
        double bandwidth = (double)total_bytes_received / time_taken * 1000; // Calculate the average bandwidth in bytes per second

        fprintf(stdout, "Time taken to receive the file: %ld ms\n", time_taken);
        fprintf(stdout, "Average bandwidth: %.2f bytes/s\n", bandwidth);
       // Close the sender's socket and continue to the next iteration.
        close(sender_socket);
        fprintf(stdout, "Total bytes received from the sender %s:%d: %d\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port), total_bytes_received);
        fprintf(stdout, "Client %s:%d disconnected\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
    }

    // Close the receiver's socket.
    close(sock);
    fprintf(stdout, "Receiver finished!\n");

    return 0;
}