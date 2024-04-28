#include <stdio.h> // Standard input/output library
#include <arpa/inet.h> // For the in_addr structure and the inet_pton function
#include <sys/time.h> // For the gettimeofday function
#include <time.h> // For the time function
#include <sys/socket.h> // For the socket function
#include <netinet/in.h> // For the sockaddr_in structure
#include <netinet/tcp.h> // for TCP_CONGESTION
#include <unistd.h> // For the close function
#include <string.h> // For the memset function
#include <stdlib.h> 
#define BUFFER_SIZE 8192*4
#define EXPECTED_BYTES 2097152

// This function is called whenever an error occurs.
// It prints an error message, closes any open sockets, and then terminates the program.
void handleError(int sock, int sender_socket, const char* msg) {
    perror(msg);
    if (sender_socket != -1) close(sender_socket);
    if (sock != -1) close(sock);
    exit(1);
}

// function for setting up sockets while error handling using handleError helper function.
int setupSocket(int receiver_port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    // check if an error occured.
    if (sock == -1) handleError(-1, -1, "socket(2)");

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        handleError(sock, -1, "setsockopt(2)");

    struct sockaddr_in receiver;
    memset(&receiver, 0, sizeof(receiver));
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (receiver.sin_addr.s_addr == INADDR_NONE) 
        handleError(sock, -1, "inet_addr(3)");
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(receiver_port);
    
    if (bind(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0)
        handleError(sock, -1, "bind(2)");

    if (listen(sock, 1) < 0)
        handleError(sock, -1, "listen(2)");
    // returns the socket descriptor to accept connections from clients.
    return sock;
}


int main(int argc, char *argv[]) {
    clock_t start_t, end_t;
    double total_t;
    double avgTime_t;
    int times_sent = 0;
    double total_bandwidth = 0;
    if (argc != 5)
    {
        fprintf(stderr, "5 arguments needed !\n");
        return 1;
    }

    const int receiver_port = atoi(argv[2]);
    const char *algo = argv[4];

    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    // create a socket, bind it to the specified port, and start listening for incoming connections.
    int sock = setupSocket(receiver_port);
    fprintf(stdout, "Listening for incoming connections on port %d...\n", receiver_port);
    // Blocks until a client connects to the Receiver.
    int sender_socket = accept(sock, (struct sockaddr *)&sender, &sender_len);
    // Sets congestion control algos, the user chooses which algo to use by input.
    if (setsockopt(sender_socket, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0)
    {
        handleError(sock, sender_socket, "setsockopt(2) - TCP_CONGESTION");
    }
    if (sender_socket < 0)
    {
        handleError(sock, -1, "accept(2)");
    }
    fprintf(stdout, "Sender connected, beginning to receive file..\n");


    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    int bytes_received = 0;
    int total_bytes_received = 0;
    
    start_t = clock();
    while (total_bytes_received < EXPECTED_BYTES)
    {
        bytes_received = recv(sender_socket, buffer, BUFFER_SIZE, 0);
        total_bytes_received += bytes_received;
        // If the message receiving failed, print an error message and return 1.
        if (bytes_received < 0){
        perror("recv(2)");
        close(sender_socket);
        close(sock);
        return 1;
        } else if (bytes_received == 0){
            fprintf(stdout, "Sender disconnected\n\n");
            break;
        }
        
        if (total_bytes_received>=EXPECTED_BYTES){
            fprintf(stdout, "File transfer completed.\n");
            times_sent++;
            end_t = clock();
            double time_taken = ((double)(end_t - start_t) / CLOCKS_PER_SEC) * 1000;
            double singleBandwidth = ((double)total_bytes_received / (time_taken / 1000)) /(1024 * 1024);
            fprintf(stdout, "Run# %d Data: Time=%.2f; Speed=%.2fMB/s \n", times_sent,time_taken, singleBandwidth);
            
            total_t += time_taken;
            total_bandwidth += singleBandwidth;
            total_bytes_received = 0;
            singleBandwidth = 0;
            time_taken = 0;
            fprintf(stdout, "Waiting for a sender response...\n");
            
        }
        buffer[bytes_received] = '\0'; // null-terminate the received data
        if(strcmp(buffer, "yes") == 0) { // sender decided to send again
            start_t = clock();
        }
        if (strcmp(buffer, "EXIT") == 0){
        fprintf(stdout, "Sender sent EXIT message \n\n");
        break;
        }
    }
    
    /******************STATISTICS*************************/
    avgTime_t = (total_t / times_sent);
    fprintf(stdout, "------------------------------------------------\n\t\t * Statistics * \t\t\n\n");
    fprintf(stdout, "- Average time: %.2f ms\n", avgTime_t);        
    //double avgBandwidth = (((double)total_bandwidth*times_sent) / total_t); // Calculate the average bandwidth in megabytes per second
    double avgBandwidth = (total_bandwidth / times_sent); // average bandwidth in MB/s
    fprintf(stdout, "- Average bandwidth: %.2f MB/s\n", avgBandwidth);
    fprintf(stdout, "------------------------------------------------\n");
    // Close the sender's socket and continue to the next iteration.
    close(sender_socket);
    close(sock);
    fprintf(stdout, "Receiver done.\n");
    return 0;
}
