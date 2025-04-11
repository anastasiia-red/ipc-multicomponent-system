#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>

#define MAX_BUFFER_SIZE 512
using namespace std;

char* write_to_shared_memory(int shm_id) 
{
    char* shm_addr = (char*)(shmat(shm_id, nullptr, 0));
    if (shm_addr == (void*)-1) 
    {
        perror("T: shmat error");
        exit(EXIT_FAILURE);
    }
    return shm_addr;
}

void semaphore_lock(int sem_id, int sem_num) 
{
    struct sembuf op = {(unsigned short)(sem_num), 1, SEM_UNDO}; 
    if (semop(sem_id, &op, 1) == -1) 
    {
        perror("T: error semop lock");
        exit(EXIT_FAILURE);
    }
}

void semaphore_unlock(int sem_id, int sem_num) 
{
    struct sembuf op = {(unsigned short)(sem_num), -1, SEM_UNDO}; 
    if (semop(sem_id, &op, 1) == -1) 
    {
        perror("T: error semop unlock");
        exit(EXIT_FAILURE);
    }
}

char* read_data(int pipe_fd)
{
    if (pipe_fd < 0) 
    {
        perror("Invalid pipe descriptor");
        exit(EXIT_FAILURE);
    }

    static char buffer[MAX_BUFFER_SIZE];
    ssize_t len = 0;
    size_t index = 0;
    char ch;

    while ((len = read(pipe_fd, &ch, 1)) > 0) 
    {
        if (ch == '\n') 
        {
            break;
        }
        else if (index < MAX_BUFFER_SIZE - 1) 
        { 
            buffer[index++] = ch;
        }
    }
    buffer[index] = '\0';

    if (len < 0) 
    {
        perror("Error reading from pipe");
        exit(EXIT_FAILURE);
    }

    printf("T: buff LEN: %ld with value : %s\n", index, buffer);
    return buffer;
}

int main(int argc, char* argv[]) 
{
    if (argc != 4) 
    {
        cerr << "Usage: " << argv[0] << " <pipe_path> <shared_memory_id> <semaphore_id>" << endl;
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR2,
           [](int signal)
           {
               cout << "T: got usr2, exiting..." << endl;
               exit(EXIT_SUCCESS);
           });

    kill(getppid(),SIGUSR1);

    int shm_id = stoi(argv[2]);       
    int sem_id = stoi(argv[3]);       

    int pipe_fd = stoi(argv[1]);

    char* sm = write_to_shared_memory(shm_id);

    while (1) 
    {
        cout << "T: Cycle started" << endl;
        char* data = read_data(pipe_fd);
        semaphore_lock(sem_id, 1); 
        strncpy(sm, data, MAX_BUFFER_SIZE);

        cout << "T: writed to shared memory" << endl;
        semaphore_unlock(sem_id, 0);  

        cout << "T: Received from pipe and written to shared memory: " << data << endl;
    }

    return 0;
}
