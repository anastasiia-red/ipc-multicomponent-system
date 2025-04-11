#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

char buff[512];
void sm_read(const char* sm) 
{
    buff[0] = '\0';
    strcpy(buff, sm);
    printf("D: Readed from shared memory: %s\n", buff);
}

void semaphore_lock(int sem_id, int sem_num) 
{
    cout << "D: semaphore lock at: " << sem_id << endl;
    struct sembuf op{(unsigned short)sem_num, 1, SEM_UNDO};
    if (semop(sem_id, &op, 1) < 0) 
    {
        perror("D: semop lock error");
        exit(EXIT_FAILURE);
    }
}

void semaphore_unlock(int sem_id, int sem_num) 
{
    cout << "D: semaphore unlock at: " << sem_id << endl;
    struct sembuf op{(unsigned short)sem_num, -1, SEM_UNDO};
    if (semop(sem_id, &op, 1) < 0) 
    {
        perror("D: semop unlock error");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) 
{
    if (argc != 4) 
    {
        fprintf(stderr, "Usage: %s <shared_mem_id> <semaphore_id> <port>\n", argv[0]);
        return 1;
    }

    signal(SIGUSR2,
           [](int signal)
           {
               cout << "D: got usr2, exiting..." << endl;
               exit(EXIT_SUCCESS);
           });

    if (kill(getppid(), SIGUSR1) == -1)
    {
        std::cerr << "ProcD: SIGUSR1 kill error" << std::endl;
        exit(EXIT_FAILURE);
    }

    int shm_id = atoi(argv[1]);        
    int sem_id = atoi(argv[2]);        
    int port = atoi(argv[3]);          

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("D: Socket creation error");
        exit(EXIT_FAILURE);
    }
    cout << "D: socket created " << endl;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port); 
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("D: Bind error");
        exit(EXIT_FAILURE);
    }
    cout << "D: socket binded" << endl;

    char *sm = (char *)shmat(shm_id, NULL, 0);
    if (sm == (void *)-1)
    {
        perror("D: Shared memory error");
        exit(EXIT_FAILURE);
    }

    while (1) 
    {
        sm_read(sm);
        semaphore_lock(sem_id, 0);  
        if (write(sock, buff, (size_t)(strlen(buff) + 1)) == -1)
        {
            perror("D: Socket write error");
            exit(EXIT_FAILURE);
        }
        printf("D: buffer writed : %s\n", buff);
        semaphore_unlock(sem_id, 1);
        sleep(3);
    }

    return 0;
}
