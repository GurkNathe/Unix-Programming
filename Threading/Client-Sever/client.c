#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

/**
 * Name: Ethan Krug
 * Assignment: Lab 4
 * Date: 4/3/2022
 *
 * Notes: Code based off of:
 * https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
 */

#define BUFFER_SIZE 1024
#define SA struct sockaddr

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 54321
#define DEFAULT_MAX_BURGERS 10

// Client function
void call_server(int sockfd, int max_burgers)
{
    int max_b_start = max_burgers;
    int burgers_eaten = 0;
    while (max_burgers > 0)
    {
        char buff[BUFFER_SIZE] = "Burger";
        printf("Client is asking for a burger.\n");
        // Send message to server
        write(sockfd, buff, sizeof(buff));

        // Clear buffer to receive message from server
        bzero(buff, sizeof(buff));
        // Read message from server
        read(sockfd, buff, sizeof(buff));

        if (strlen(buff) == 0)
        {
            printf("Shop is closed.\n");
            break;
        }

        srand(time(NULL));
        int eat_time = ((rand() % 3) * 2) + 1;
        if ((strncmp(buff, "Avail", 5)) == 0)
        {
            burgers_eaten++;
            printf("Client got their burger, and ate it in %d seconds.\n", eat_time);
            printf("Client has eaten %d burgers, and can eat %d more.\n", burgers_eaten, max_b_start - burgers_eaten);
            max_burgers--;
        }
        else if ((strncmp(buff, "NoBurgers", 9)) == 0)
        {
            printf("There are no more burgers.\n");
            break;
        }
        else if ((strncmp(buff, "Waiting", 7)) == 0)
        {
            printf("Client is waiting for a burger.\n");
        }
        sleep(eat_time);
    }

    printf("Client is done eating. Leaving shop.\n");
    write(sockfd, "Done", sizeof("Done"));
}

/**
 * 1. IP
 * 2. Port
 * 3. Max number of burgers client can eat
 */

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;

    char *ip = DEFAULT_IP;
    int port = DEFAULT_PORT;
    int max_burgers = DEFAULT_MAX_BURGERS;

    // Parse arguments
    if (argc == 1) // Default if no inputs
    {
        printf("Using default IP: %s\n", ip);
        printf("Using default port: %d\n", port);
        printf("Using default max_burgers: %d\n", max_burgers);
    }
    else if (argc == 4) // Use inputs
    {
        ip = argv[1];
        port = atoi(argv[2]);
        max_burgers = atoi(argv[3]);

        if (max_burgers < 0)
        {
            printf("Invalid max_burgers number.\n");
            return 1;
        }

        printf("Using IP: %s\n", ip);
        printf("Using port: %d\n", port);
        printf("Using max_burgers: %d\n", max_burgers);
    }
    else // Invalid inputs
    {
        printf("Error: Invalid parameters.\nUsage: %s <ip> <port> <max_burgers>\n", argv[0]);
        return 1;
    }

    // Create socket and make sure it was created successfully
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: socket creation failed.\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));

    // Assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if ((servaddr.sin_addr.s_addr = inet_addr(ip)) < 0)
    {
        printf("Error: inet_pton error occured\n");
        exit(1);
    }

    // Connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Error: Connect Failed. Check if %s is a valid IP address and that port %d is the correct port to connect to. \n", ip, port);
        exit(1);
    }

    // Client function
    call_server(sockfd, max_burgers);

    // Close the socket
    close(sockfd);
    return 0;
}
