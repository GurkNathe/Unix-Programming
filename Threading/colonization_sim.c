#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <atomic>

/**
 * Name: Ethan Krug
 * Assignment: Lab 3
 * Date: 19/2/2022
 */

std::atomic<int> end;
std::atomic<int> M;
std::atomic<int> N;
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// Code from announcment
int WriteToBinFile(char *sFileName, char *bArray, int nSize)
{
    FILE *pFile = NULL;
    if ((pFile = fopen(sFileName, "wb+")) == NULL)
    {
        printf("Error in creating the file:%s\n", sFileName);
        return -1;
    }
    else
    {
        fwrite(bArray, sizeof(char), nSize, pFile);
        fclose(pFile);
        return 0;
    }
}

// Code from announcment
int ReadFromBinFile(char *sFileName, char **bArray, int *nSize)
{
    FILE *pFile = NULL;
    if ((pFile = fopen(sFileName, "rb+")) == NULL)
    {
        printf("Error in opening the file:%s\n", sFileName);
        return -1;
    }
    else
    {
        // calculate the size of the file
        // position the file pointer to the end of the file
        fseek(pFile, 0, SEEK_END);
        // get the size of the file in bytes
        *nSize = ftell(pFile);
        // allocate the space for the array
        *bArray = (char *)malloc(*nSize * sizeof(char));
        fseek(pFile, 0, SEEK_SET);

        // second solution reading all at once (fast)
        fread(*bArray, sizeof(char), *nSize, pFile);
        fclose(pFile);
        return 0;
    }
}

// Prints the map data to the standard output
int PrintMap(char *bArray, int M, int N)
{
    for (int i = 0; i < N; i++)
    {
        printf("-\t");
    }
    printf("\n");
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (bArray[i * N + j] == 0)
            {
                printf("%d\t", bArray[i * N + j]);
            }
            else
            {
                printf("%c\t", bArray[i * N + j]);
            }
        }
        printf("\n");
    }
    for (int i = 0; i < N; i++)
    {
        printf("-\t");
    }
    printf("\n");
    return 0;
}

void *team_thread(void *arg)
{
    // Team character value
    char team = (char)(long)arg;
    int rows = M.load();
    int cols = N.load();

    struct flock lock;
    lock.l_len = 1;
    lock.l_whence = SEEK_SET;

    while (!end.load())
    {
        srand(time(NULL));
        int x = rand() % cols;
        int y = rand() % rows;

        char *OutArray;
        char sFileName[] = "map.bin";
        int nElements = 0;

        // set lock parameters
        lock.l_type = F_WRLCK;
        lock.l_pid = getpid();

        // lock the file at (x, y)
        lock.l_start = x * cols + y;

        int fd = open(sFileName, O_RDWR);

        // Check if the file is opened properly
        if (fd < 0)
        {
            perror("Error opening file\n");
            exit(1);
        }

        fcntl(fd, F_SETLKW, &lock);
        pthread_mutex_lock(&myMutex);
        ReadFromBinFile(sFileName, &OutArray, &nElements);
        printf("Player from Team %c is firing at (%d, %d).\n", team - 32, x, y);

        // If the cell is empty, place the team character
        if (OutArray[x * cols + y] == 0)
        {
            OutArray[x * cols + y] = team;
            printf("Player from Team %c has conquered (%d, %d).\n", team - 32, x, y);
            int neighborTeam = 1;
            /*
                If there are more team characters in the 8 neighbor cells,
                convert all 8 neighbor cells to the team character.
                If statement conditions to guard against out of bounds array access,
                and to check if the neighbor cell is a conquered by player's team.
            */
            if (x > 0)
            {
                if (y > 0 && (OutArray[(x - 1) * cols + (y - 1)] == team ||
                              OutArray[(x - 1) * cols + (y - 1)] == team - 32))
                    neighborTeam++;
                if (OutArray[(x - 1) * cols + y] == team ||
                    OutArray[(x - 1) * cols + y] == team - 32)
                    neighborTeam++;
                if (y + 1 < rows && (OutArray[(x - 1) * cols + (y + 1)] == team ||
                                     OutArray[(x - 1) * cols + (y + 1)] == team - 32))
                    neighborTeam++;
            }

            if (y > 0 && (OutArray[x * cols + (y - 1)] == team ||
                          OutArray[x * cols + (y - 1)] == team - 32))
                neighborTeam++;
            if (y + 1 < rows && (OutArray[x * cols + (y + 1)] == team ||
                                 OutArray[x * cols + (y + 1)] == team - 32))
                neighborTeam++;
            if (x + 1 < cols)
            {
                if (y > 0 && (OutArray[(x + 1) * cols + (y - 1)] == team ||
                              OutArray[(x + 1) * cols + (y - 1)] == team - 32))
                    neighborTeam++;
                if (OutArray[(x + 1) * cols + y] == team ||
                    OutArray[(x + 1) * cols + y] == team - 32)
                    neighborTeam++;
                if (y + 1 < rows && (OutArray[(x + 1) * cols + (y + 1)] == team ||
                                     OutArray[(x + 1) * cols + (y + 1)] == team - 32))
                    neighborTeam++;
            }

            // If majority of 8 surrounding cells are the team character,
            // convert the cell to the team character
            if (neighborTeam >= 4)
            {
                printf("Team %c has conquered the 8 cells surrounding (%d, %d).\n", team - 32, x, y);
                if (x > 0)
                {
                    if (y > 0 && (OutArray[(x - 1) * cols + (y - 1)] == 'a' ||
                                  OutArray[(x - 1) * cols + (y - 1)] == 'b' ||
                                  OutArray[(x - 1) * cols + (y - 1)] == 0))
                    {

                        lock.l_start = (x - 1) * cols + (y - 1);
                        fcntl(fd, F_SETLKW, &lock);
                        OutArray[(x - 1) * cols + (y - 1)] = team;
                    }
                    if (OutArray[(x - 1) * cols + y] == 'a' ||
                        OutArray[(x - 1) * cols + y] == 'b' ||
                        OutArray[(x - 1) * cols + y] == 0)
                    {
                        lock.l_start = (x - 1) * cols + y;
                        fcntl(fd, F_SETLKW, &lock);
                        OutArray[(x - 1) * cols + y] = team;
                    }
                    if (y + 1 < rows && (OutArray[(x - 1) * cols + (y + 1)] == 'a' ||
                                         OutArray[(x - 1) * cols + (y + 1)] == 'b' ||
                                         OutArray[(x - 1) * cols + (y + 1)] == 0))
                    {
                        lock.l_start = (x - 1) * cols + (y + 1);
                        fcntl(fd, F_SETLKW, &lock);
                        OutArray[(x - 1) * cols + (y + 1)] = team;
                    }
                }

                if (y > 0 && (OutArray[x * cols + (y - 1)] == 'a' ||
                              OutArray[x * cols + (y - 1)] == 'b' ||
                              OutArray[x * cols + (y - 1)] == 0))
                {
                    lock.l_start = x * cols + (y - 1);
                    fcntl(fd, F_SETLKW, &lock);
                    OutArray[x * cols + (y - 1)] = team;
                }
                if (y + 1 < rows && (OutArray[x * cols + (y + 1)] == 'a' ||
                                     OutArray[x * cols + (y + 1)] == 'b' ||
                                     OutArray[x * cols + (y + 1)] == 0))
                {
                    lock.l_start = x * cols + (y + 1);
                    fcntl(fd, F_SETLKW, &lock);
                    OutArray[x * cols + (y + 1)] = team;
                }

                if (x + 1 < cols)
                {
                    if (y > 0 && (OutArray[(x + 1) * cols + (y - 1)] == 'a' ||
                                  OutArray[(x + 1) * cols + (y - 1)] == 'b' ||
                                  OutArray[(x + 1) * cols + (y - 1)] == 0))
                    {
                        lock.l_start = (x + 1) * cols + (y - 1);
                        fcntl(fd, F_SETLKW, &lock);
                        OutArray[(x + 1) * cols + (y - 1)] = team;
                    }
                    if (OutArray[(x + 1) * cols + y] == 'a' ||
                        OutArray[(x + 1) * cols + y] == 'b' ||
                        OutArray[(x + 1) * cols + y] == 0)
                    {
                        lock.l_start = (x + 1) * cols + y;
                        fcntl(fd, F_SETLKW, &lock);
                        OutArray[(x + 1) * cols + y] = team;
                    }
                    if (y + 1 < rows && (OutArray[(x + 1) * cols + (y + 1)] == 'a' ||
                                         OutArray[(x + 1) * cols + (y + 1)] == 'b' ||
                                         OutArray[(x + 1) * cols + (y + 1)] == 0))
                    {
                        lock.l_start = (x + 1) * cols + (y + 1);
                        fcntl(fd, F_SETLKW, &lock);
                        OutArray[(x + 1) * cols + (y + 1)] = team;
                    }
                }
            }
            WriteToBinFile(sFileName, OutArray, nElements);

            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &lock);
        }
        else
        {
            printf("The cell has already been captured!\n");
        }
        PrintMap(OutArray, M.load(), N.load());
        free(OutArray);
        pthread_mutex_unlock(&myMutex);

        if (close(fd) < 0)
        {
            perror("close error\n");
            exit(EXIT_FAILURE);
        }

        sleep((rand() % 3) + 1);
    }
    pthread_exit(NULL);
}

// Thread function for determining if the game is over
void *supervisor_thread(void *arg)
{
    char *OutArray;
    char sFileName[] = "map.bin";
    int nElements = 0;

    char winner;
    while (!end.load())
    {
        pthread_mutex_lock(&myMutex);
        ReadFromBinFile(sFileName, &OutArray, &nElements);

        // Check if there are any 0s in the map
        // Count each teams score
        int zero_count = 0;
        int T1 = 0;
        int T2 = 0;
        for (int i = 0; i < nElements; i++)
        {
            if (OutArray[i] == 0)
                zero_count++;
            else if (OutArray[i] == 'A' || OutArray[i] == 'a')
                T1++;
            else if (OutArray[i] == 'B' || OutArray[i] == 'b')
                T2++;
        }

        if (zero_count == 0)
        {
            printf("The game is over.\n");
            winner = (T1 > T2) ? 'A' : 'B';
            winner = (T1 == T2) ? 'T' : winner;
            end.store(1);
            pthread_mutex_unlock(&myMutex);
            break;
        }
        pthread_mutex_unlock(&myMutex);
        sleep(5);
    }
    printf("Ending the game...\nThe final map looks like:\n");
    PrintMap(OutArray, M.load(), N.load());
    if (winner == 'T')
        printf("The game is a tie.\n");
    else
        printf("The winner is %c\n", winner);
    free(OutArray);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // Checking if 4 arguments have been passed as input
    if (argc != 5)
    {
        printf("Usage: %s <number of team 1 players> <number of team 2 players> <number of rows on map> <number of columns on map>\n", argv[0]);
        return 1;
    }

    // Initialize and create map
    {
        // Initialize game variables
        int nTeam1Players = atoi(argv[1]);
        int nTeam2Players = atoi(argv[2]);
        M.store(atoi(argv[3]));
        N.store(atoi(argv[4]));

        int rows = M.load();
        int cols = N.load();

        // Checking for valid inputs
        if (rows < 1 || cols < 1)
        {
            printf("The map must have at least 1 row and 1 column.\n");
            return 1;
        }
        else if (((nTeam1Players + nTeam2Players) > (rows * cols)))
        {
            printf("There can't be more players than the number of cells on the map.\n");
            return 1;
        }

        // Initialize map variables
        char sFileName[] = "map.bin";
        int nSize = rows * cols;
        char InArray[nSize] = {0};

        // Initialize and create map
        srand(time(NULL));
        // Initialize Team 1 players
        for (int i = 0; i < nTeam1Players; i++)
        {
            while (1)
            {
                int x = rand() % rows;
                int y = rand() % cols;
                if (InArray[x * cols + y] == 0)
                {
                    InArray[x * cols + y] = 'A';
                    break;
                }
            }
        }

        // Initialize Team 2 players
        for (int i = 0; i < nTeam2Players; i++)
        {
            while (1)
            {
                int x = rand() % rows;
                int y = rand() % cols;
                if (InArray[x * cols + y] == 0)
                {
                    InArray[x * cols + y] = 'B';
                    break;
                }
            }
        }

        // Write starting map to file.
        WriteToBinFile(sFileName, InArray, nSize);

        // print the content of the array to standard output
        PrintMap(InArray, rows, cols);

        // Initialize both teams' players
        pthread_t threads[nTeam1Players];
        pthread_t threads2[nTeam2Players];

        // Start the threads
        for (int i = 0, j = 0;; i++, j++)
        {
            if (i < nTeam1Players)
            {
                pthread_create(&threads[i], NULL, team_thread, (void *)(long)'a');
            }
            if (j < nTeam2Players)
            {
                pthread_create(&threads2[j], NULL, team_thread, (void *)(long)'b');
            }
            if (i >= nTeam1Players && j >= nTeam2Players)
                break;
        }

        // Initialize and start supervisor thread
        pthread_t supervisor_thread_id;
        pthread_create(&supervisor_thread_id, NULL, supervisor_thread, NULL);

        // Wait for all threads to finish
        for (int i = 0; i < nTeam1Players; i++)
        {
            pthread_join(threads[i], NULL);
        }

        for (int i = 0; i < nTeam2Players; i++)
        {
            pthread_join(threads2[i], NULL);
        }

        pthread_join(supervisor_thread_id, NULL);
    }
    remove("map.bin");
    return 0;
}
