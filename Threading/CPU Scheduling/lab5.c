#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

/**
 * Name: Ethan Krug
 * Assignment: Lab 5
 * Date: 11/3/2022
 */

#pragma pack(push, 1)
typedef struct _PCB
{
    char Priority;
    char Name[24];
    int ProcessID;
    char IsActive;
    int CPUBurstTime;
    int MemoryBase;
    long MemoryLimit;
    int NoOfFiles;
} PCB;
#pragma pack(pop)

// Array that stores the processing methods and the number of processes per method
double processing[4][2];
PCB *queues[4];
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// Process that handles the priority scheduling algorithm
int priority_sheduling(int index)
{
    pthread_mutex_lock(&myMutex);
    // Execute the processes in the priority queue
    for (int i = 0; i < processing[index][1]; i++)
    {
        // If the process is active
        if (queues[index][i].IsActive == 1)
        {
            // Execute the process
            printf("Priority Scheduling Processor:\n");
            printf("Executing process %s with priority %d\n", queues[index][i].Name, queues[index][i].Priority);
            sleep(queues[index][i].CPUBurstTime / 1000);
            printf("Finished executing process %s with priority %d\n\n", queues[index][i].Name, queues[index][i].Priority);
            queues[index][i].IsActive = 0;
        }
    }
    pthread_mutex_unlock(&myMutex);
}

// Process that handles the shortest job first algorithm
int sjf(int index)
{
    pthread_mutex_lock(&myMutex);
    // Execute the processes in the priority queue
    for (int i = 0; i < processing[index][1]; i++)
    {
        // If the process is active
        if (queues[index][i].IsActive == 1)
        {
            // Execute the process
            printf("Shortest Job First Processor:\n");
            printf("Executing process %s with CPU burst time of %d\n", queues[index][i].Name, queues[index][i].CPUBurstTime);
            sleep(queues[index][i].CPUBurstTime / 1000);
            printf("Finished executing process %s with CPU burst time of %d\n\n", queues[index][i].Name, queues[index][i].CPUBurstTime);
            queues[index][i].IsActive = 0;
        }
    }
    pthread_mutex_unlock(&myMutex);
}

// Process that handles the round robin algorithm
int round_robin(int index)
{
    pthread_mutex_lock(&myMutex);
    // Execute the processes in the priority queue
    for (int i = 0; i < processing[index][1]; i++)
    {
        // If the process is active
        if (queues[index][i].IsActive == 1)
        {
            // Execute the process
            printf("Round Robin Processor:\n");
            printf("Executing process %s with priority %d\n", queues[index][i].Name, queues[index][i].Priority);
            sleep(queues[index][i].CPUBurstTime / 1000);
            printf("Finished executing process %s with priority %d\n\n", queues[index][i].Name, queues[index][i].Priority);
            queues[index][i].IsActive = 0;
        }
    }
    pthread_mutex_unlock(&myMutex);
}

// Process that handles the first come first serve algorithm
int fcfs(int index)
{
    pthread_mutex_lock(&myMutex);
    // Execute the processes
    for (int i = 0; i < processing[index][1]; i++)
    {
        if (queues[index][i].IsActive == 1)
        {
            printf("First Come First Serve Processor:\n");
            printf("Executing process %s with priority %d\n", queues[index][i].Name, queues[index][i].Priority);
            sleep(queues[index][i].CPUBurstTime / 1000);
            printf("Finished executing process %s with priority %d\n\n", queues[index][i].Name, queues[index][i].Priority);
            queues[index][i].IsActive = 0;
        }
    }
    pthread_mutex_unlock(&myMutex);
}

void *scheduler(void *args)
{
    int index = (int)(long)args;
    pthread_mutex_lock(&myMutex);
    // If the method is priority scheduling, we need to sort the queues
    if (index == 0)
    {
        // Sort the queues
        for (int i = 0; i < processing[index][1]; i++)
        {
            for (int j = i + 1; j < processing[index][1]; j++)
            {
                if (queues[index][i].Priority > queues[index][j].Priority)
                {
                    PCB temp = queues[index][i];
                    queues[index][i] = queues[index][j];
                    queues[index][j] = temp;
                }
            }
        }
    }
    // If the method is shortest job first, we need to sort the queues
    else if (index == 1)
    {
        // Sort the queues
        for (int i = 0; i < processing[index][1]; i++)
        {
            for (int j = i + 1; j < processing[index][1]; j++)
            {
                if (queues[index][i].CPUBurstTime > queues[index][j].CPUBurstTime)
                {
                    PCB temp = queues[index][i];
                    queues[index][i] = queues[index][j];
                    queues[index][j] = temp;
                }
            }
        }
    }
    // If the method is round robin, we need to sort the queues
    else if (index == 2)
    {
        // Sort the queues
        for (int i = 0; i < processing[index][1]; i++)
        {
            for (int j = i + 1; j < processing[index][1]; j++)
            {
                if (queues[index][i].Priority > queues[index][j].Priority)
                {
                    PCB temp = queues[index][i];
                    queues[index][i] = queues[index][j];
                    queues[index][j] = temp;
                }
            }
        }
    }
    // If the method is FCFS, we don't need to sort the queues
    pthread_mutex_unlock(&myMutex);

    if (processing[index][0] == 1)
    {
        printf("\nPriority Scheduling:\n");
        priority_sheduling(index);
    }
    else if (processing[index][0] == 2)
    {
        printf("\nShortest Job First:\n");
        sjf(index);
    }
    else if (processing[index][0] == 3)
    {
        printf("\nRound Robin:\n");
        round_robin(index);
    }
    else
    {
        printf("\nFCFS:\n");
        fcfs(index);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // Checking for proper command line inputs
    if (argc == 1)
    {
        printf("No arguments given, nothing happens.\n");
        printf("Usage: %s <binary file> <processing method> <%% of file processed>\n", argv[0]);
        return 1;
    }
    else if (argc == 2)
    {
        printf("No processing methods given, nothing happens.\n");
        printf("Usage: %s <binary file> <processing method> <%% of file processed>\n", argv[0]);
        return 1;
    }
    else if (argc == 3)
    {
        printf("Processing method and binary file given, but %% of file processed wasn't. Nothing happens.\n");
        printf("Usage: %s <binary file> <processing method> <%% of file processed>\n", argv[0]);
        return 1;
    }
    else if (strtod(argv[3], NULL) > 1.0 || strtod(argv[3], NULL) < 0.0)
    {
        printf("%% of file processed must be greater than or equal to 0, and less than or equal to 1. Nothing happens\n");
        printf("Usage: %s <binary file> <processing method> <%% of file processed>\n", argv[0]);
        return 1;
    }
    else if (argc > 10)
    {
        printf("Too many arguments given, only 4 processing methods allowed.\n");
        printf("Usage: %s <binary file> <processing method> <%% of file processed>\n", argv[0]);
        return 1;
    }
    else if (argc % 2 != 0)
    {
        printf("Invalid number of arguments. Please add the %% of the file to be processed by the processing method.\n");
        printf("Usage: %s <binary file> <processing method> <%% of file processed>\n", argv[0]);
        return 1;
    }

    // Parse inputs for processing methods
    for (int i = 0; i < (argc - 2) / 2; i++)
    {
        processing[i][0] = strtod(argv[i * 2 + 2], NULL);
        processing[i][1] = strtod(argv[i * 2 + 3], NULL);
    }

    // Check to make sure the percentage of processes to be processed is equal to 1
    double total = 0;
    for (int i = 0; i < (argc - 2) / 2; i++)
    {
        total += processing[i][1];
    }

    if (total != 1.0)
    {
        printf("Total percentage of processes to be processed must be equal to 100%%.\n");
        printf("Usage: %s <binary file> <processing method> <%% of file processed>\n", argv[0]);
        return 1;
    }

    // open binary file
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        printf("The file '%s' was not found.\n", argv[1]);
        return 1;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // get number of processes
    int no_of_processes = file_size / sizeof(PCB);

    // Print the number of processes in the file
    printf("Number of processes in '%s': %d\n", argv[1], no_of_processes);

    // allocate memory for PCBs
    PCB *pcbs = (PCB *)malloc(no_of_processes * sizeof(PCB));
    if (pcbs == NULL)
    {
        printf("Could not allocate memory for PCBs.\n");
        return 1;
    }

    // read PCBs from binary file
    fread(pcbs, sizeof(PCB), no_of_processes, file);

    // Get total amount of memory allocated to processes
    long total_memory = 0;
    for (int i = 0; i < no_of_processes; i++)
    {
        total_memory += pcbs[i].MemoryLimit;
    }

    // Print total amount of memory allocated to processes
    printf("Total amount of memory allocated to processes: %ld\n", total_memory);

    // Get overall number of open files for processes
    int total_files = 0;
    for (int i = 0; i < no_of_processes; i++)
    {
        total_files += pcbs[i].NoOfFiles;
    }

    // Print total number of open files for processes
    printf("Total number of open files for processes: %d\n", total_files);
    free(pcbs);

    // Create a queue for each possible process
    for (int i = 0; i < (argc - 2) / 2; i++)
    {
        queues[i] = (PCB *)malloc((int)(processing[i][1] * no_of_processes * sizeof(PCB)));
        if (queues[i] == NULL)
        {
            printf("Could not allocate memory for queue %d.\n", i);
            return 1;
        }
        processing[i][1] = (int)(processing[i][1] * no_of_processes);
    }

    // Read the data for each queue for each possible process
    fseek(file, 0, SEEK_SET);
    for (int i = 0; i < (argc - 2) / 2; i++)
    {
        fread(queues[i], sizeof(PCB), (int)(processing[i][1]), file);
    }

    // Create thread for each processing method
    pthread_t threads[4];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);

    // Create thread for each processing method
    for (int i = 0; i < (argc - 2) / 2; i++)
    {
        pthread_create(&threads[i], &attr, scheduler, (void *)(long)(i));
    }

    // Wait for all threads to finish
    for (int i = 0; i < (argc - 2) / 2; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("All processes executed.\n");

    fclose(file);
    return 0;
}
