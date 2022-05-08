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
 * https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/ and
 * https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
 */

#define BUFFER_SIZE 1024
#define PORT 54321
#define SA struct sockaddr

#define DEFAULT_MAX_BURGERS 25
#define DEFAULT_NUM_CHEFS 2

// thread variables
int burgers = 0;
int available_burgers = 0;
int max_burgers = 0;
int chefs_done = 0;
int num_chefs = 0;
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// Function that handles the client connection
void client_call_handle(int connfd)
{
    char buff[BUFFER_SIZE];
    int n;
    // infinite loop for chat
    while (1)
    {
        bzero(buff, BUFFER_SIZE);

        // read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));

        if (strlen(buff) == 0)
        {
            printf("Client has left, finishing up making burgers.\n");
            break;
        }

        // Client requests a burger, so we give them one if we can
        if (strncmp("Burger", buff, 6) == 0 && available_burgers > 0)
        {
            available_burgers--;
            strcpy(buff, "Avail");
            printf("Client got a burger. There are %d burgers available.\n", available_burgers);
        }
        else if (strncmp("Burger", buff, 6) == 0 && available_burgers == 0 && burgers == max_burgers)
        {
            printf("There are no burgers available. Client didn't get a burger.\n");
            strcpy(buff, "NoBurgers");
        }
        else if (strncmp("Burger", buff, 6) == 0 && available_burgers == 0)
        {
            printf("Client is waiting for a burger.\n");
            strcpy(buff, "Waiting");
        }
        else if (strncmp("Done", buff, 4) == 0)
        {
            printf("Client is done eating.\n");
        }

        // and send that buffer to client
        write(connfd, buff, sizeof(buff));

        if (strncmp("NoBurgers", buff, 9) == 0)
        {
            printf("No more burgers available. Client leaves shop.\n");
            break;
        }
        else if (strncmp("Done", buff, 4) == 0)
        {
            // Waiting for chefs to finish making burgers
            // after client is finished eating
            while (chefs_done < num_chefs)
            {
                sleep(2);
            }
            break;
        }
    }
}

// Chef makes a burger
void *chef(void *arg)
{
    int id = (int)(long)arg;
    while (burgers < max_burgers)
    {
        srand(time(NULL));
        pthread_mutex_lock(&myMutex);
        burgers++;
        available_burgers++;
        int prod_time = ((rand() % 2) + 1) * 2;
        printf("Chef %d made a burger in %d seconds.\nThere are currently %d burgers available.\n", id, prod_time, available_burgers);
        printf("There are %d burgers left to make.\n", max_burgers - burgers);
        pthread_mutex_unlock(&myMutex);
        sleep(prod_time);
    }
    pthread_mutex_lock(&myMutex);
    chefs_done++;
    pthread_mutex_unlock(&myMutex);
    printf("Max number of burgers made, chef %d is going home.\n", id);
    pthread_exit(NULL);
}

/**
 * 1. Max number of burgers able to produce
 * 2. Max number of chefs available
 */

int main(int argc, char *argv[])
{
    int sockfd, connfd, len, opt = 1;
    struct sockaddr_in servaddr, cli;

    max_burgers = DEFAULT_MAX_BURGERS;
    int num_chefs = DEFAULT_NUM_CHEFS;

    // Parse command line arguments
    if (argc == 1)
    {
        printf("Using default max_burgers: %d\n", max_burgers);
        printf("Using default num_chefs: %d\n", num_chefs);
    }
    else if (argc == 3)
    {
        max_burgers = atoi(argv[1]);
        num_chefs = atoi(argv[2]);
        if (max_burgers < 0 || num_chefs < 0)
        {
            printf("Invalid max_burgers or num_chefs. Please enter a positive value for both.\n");
            return 1;
        }
        printf("Using max_burgers: %d\n", max_burgers);
        printf("Using num_chefs: %d\n", num_chefs);
    }
    else
    {
        printf("Error: Invalid parameters.\nUsage: %s <max_burgers> <num_chefs>\n", argv[0]);
        return 1;
    }

    // Create socket and check for error
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: socket creation failed.\n");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));

    // Assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("Error: socket binding failed.\n");
        exit(1);
    }

    int max_clients = 10;

    // Now server is ready to listen and verification
    if ((listen(sockfd, max_clients)) != 0)
    {
        printf("Error: listen failed.\n");
        exit(1);
    }

    len = sizeof(cli);

    fd_set readfds;
    int max_sd, sd, new_socket, client_socket[max_clients];

    // Create chef threads
    pthread_t chefs[num_chefs];
    for (int i = 0; i < num_chefs; i++)
    {
        pthread_create(&chefs[i], NULL, (void *)chef, (void *)(long)(i + 1));
    }

    for (int i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    // Get client sockets
    while (chefs_done < num_chefs || available_burgers > 0)
    {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(sockfd, &readfds);
        max_sd = sockfd;

        // add child sockets to set
        for (int i = 0; i < max_clients; i++)
        {
            // socket descriptor
            sd = client_socket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        if (FD_ISSET(sockfd, &readfds))
        {
            if ((new_socket = accept(sockfd, (SA *)&cli, &len)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Client gets burgers
            client_call_handle(new_socket);

            // add new socket to array of sockets
            for (int i = 0; i < max_clients; i++)
            {
                // if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }
    }

    // Join chef threads
    for (int i = 0; i < num_chefs; i++)
    {
        pthread_join(chefs[i], NULL);
    }

    // After chatting close the socket
    close(sockfd);
    return 0;
}
