#include <stdio.h>      // Standard input/output library
#include <arpa/inet.h>  // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <sys/time.h>   // For the gettimeofday function
#include <unistd.h>     // For the close function
#include <string.h>     // For the memset function
#include <stdlib.h>     // Standard library
#include <time.h>       // For the time function

#define RECEIVER_IP "127.0.0.1"
#define RECEIVER_PORT 6606
#define BUFFER_SIZE 8192 * 4

char *util_generate_random_data(unsigned int size);

int main(void){

    unsigned int file_size = 1 << 21;
    char *random_file = util_generate_random_data(file_size);
    char *file_message = random_file;
    unsigned int total_bytes_sent = 0;
    unsigned int bytes_left = file_size;
    int bytes_sent;
    int sock = -1;

    char buffer[BUFFER_SIZE + sizeof(struct timeval)] = {0};
    struct timeval timestamp;


    struct sockaddr_in receiver;
    memset(&receiver, 0, sizeof(receiver));

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("socket(2)");
        free(random_file);
        return 1;
    }

    if (inet_pton(AF_INET, RECEIVER_IP, &receiver.sin_addr) <= 0)
    {
        perror("inet_pton(3)");
        free(random_file);
        close(sock);
        return 1;
    }

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(RECEIVER_PORT);

    fprintf(stdout, "Connecting to %s:%d...\n", RECEIVER_IP, RECEIVER_PORT);

    // Try to connect to the receiver using the socket and the receiver structure.
    if (connect(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0)
    {
        perror("connect(2)");
        free(random_file);
        close(sock);
        return 1;
    }

    fprintf(stdout, "Successfully connected to the receiver!\n");

    fprintf(stdout, "Trying to send random file...\n");

    while (total_bytes_sent < file_size) {
        bytes_sent = send(sock, file_message + total_bytes_sent, bytes_left, 0);
        if (bytes_sent <= 0)
        {
            perror("send(2)");
            close(sock);
            return 1;
        }
        total_bytes_sent += bytes_sent;
        bytes_left -= bytes_sent;
    }
    fprintf(stdout, "Sent %d bytes to the receiver!\n"
                    "Waiting for the receiver to respond...\n",
            bytes_sent);
    fprintf(stdout, "User Decision: Send the file again? (y/n): ");
    char user_decision;
    scanf("%c", &user_decision);
    while (user_decision == 'y')
{
    total_bytes_sent = 0;
    bytes_left = file_size;
    file_message = random_file;
    while (total_bytes_sent < file_size) {
        bytes_sent = send(sock, file_message + total_bytes_sent, bytes_left, 0);
        if (bytes_sent <= 0)
        {
            perror("send(2)");
            close(sock);
            free(random_file);
            return 1;
        }
        total_bytes_sent += bytes_sent;
        bytes_left -= bytes_sent;
    }
    fprintf(stdout, "Sent %d bytes to the receiver!\n"
                    "Waiting for the receiver to respond...\n",
            bytes_sent);
    fprintf(stdout, "User Decision: Send the file again? (y/n): ");
    scanf(" %c", &user_decision); // Note the space before %c to skip any newline character
}
// send the string "exit" to the receiver.

fprintf(stdout, "User decided not to send the file again.\n");
const char* exit_message = "EXIT";
if (send(sock, exit_message, strlen(exit_message), 0) == -1) {
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