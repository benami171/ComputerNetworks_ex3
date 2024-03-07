#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <netinet/tcp.h> // for TCP_CONGESTION
#include <sys/time.h>   // For the gettimeofday function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <stdlib.h>     // Standard library
#include <time.h>       // For the time function

#define RECEIVER_IP "127.0.0.1"
#define RECEIVER_PORT 6606
#define FILE_SIZE 2097152

char *util_generate_random_data(unsigned int size);

int main(int argc,char *argv[]){
    if (argc != 7){
        fprintf(stderr, "not enough arguments\n");
        return 1;
    }
    const char *receiver_ip = argv[2];
    const int receiver_port = atoi(argv[4]);
    const char *algo = argv[6];
    char *random_file = util_generate_random_data(FILE_SIZE);

    int sock = -1;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0){
        perror("setsockopt(2)");
        free(random_file);
        close(sock);
        return 1;
    }

    if (sock == -1){
        perror("socket(2)");
        free(random_file);
        return 1;
    }
    struct sockaddr_in receiver;
    memset(&receiver, 0, sizeof(receiver));
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(receiver_port);

    if (inet_pton(AF_INET, receiver_ip, &receiver.sin_addr) <= 0){
        perror("inet_pton(3)");
        close(sock);
        free(random_file);
        return 1;
    }

    fprintf(stdout, "Connecting to %s:%d...\n", receiver_ip, receiver_port);

    // Try to connect to the receiver using the socket and the receiver structure.
    if (connect(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0){
        perror("connect(2)");
        free(random_file);
        close(sock);
        return 1;
    }
    fprintf(stdout, "Successfully connected to the receiver!\n");

    char user_decision;
    do{
        fprintf(stdout, "Trying to send random file...\n");

        int bytes_sent = send(sock, random_file, FILE_SIZE, 0);
        if (bytes_sent <= 0)
        {
            perror("send(2)");
            close(sock);
            free(random_file);
            return 1;
        }
        fprintf(stdout, "Sent %d bytes to the receiver!\n"
                        "User Decision: Send the file again? (y/n): ",bytes_sent);
                        
        scanf(" %c", &user_decision); // Note the space before %c to skip any newline character
        
        const char * spaceMessage = "yes";
        if(send(sock,spaceMessage,strlen(spaceMessage),0) == -1){
        perror("send(2)");
        free(random_file);
        close(sock);
        return 1;
        }
        
    } while (user_decision == 'y');

    // send the string "exit" to the receiver.

    fprintf(stdout, "User decided not to send the file again.\n");
    const char *exit_message = "EXIT";
    if (send(sock, exit_message, strlen(exit_message), 0) == -1){
        perror("send(2)");
        free(random_file);
        close(sock);
        return 1;
    }
    free(random_file);
    close(sock);

    fprintf(stdout, "Connection closed!\n");
    return 0;
}

char *util_generate_random_data(unsigned int size)
{
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
        return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
        return NULL;
    // Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}