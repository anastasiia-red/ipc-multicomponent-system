#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring> 
#include <cstdlib>
#include <cstdio> 
#include <string>

using namespace std;
int pipe_fd;
int file_descriptor;

void signal_handler(int signum) 
{
    cout << "P1: got signal (" << signum << ")" << endl;
    if(file_descriptor < 0 )
    {
        perror("P1: Erorr with reading input file\n");
        exit(EXIT_FAILURE);
    }

    char buffer[151];
    char current_symbol;
    ssize_t counter = 0;

    while(read(file_descriptor, &current_symbol, 1) > 0)
    {
        if(current_symbol == '\n')
        {
            buffer[counter] = current_symbol;
            ++counter; 
            break;
        }
        if ((unsigned long) counter < 151) 
        {
            buffer[counter] = current_symbol;
            ++counter;
        }
    }
    buffer[counter] = '\0';

    ssize_t bytes_result = write(pipe_fd, buffer, counter);
    if (bytes_result < 0) 
    {
        perror("P1: Error writing to pipe");
        exit(EXIT_FAILURE);
    }
    printf("P1: After write: %s, counter: %ld\n", buffer, (long int)counter);
}

int main(int argc, char* argv[]) 
{
    if (argc != 2) 
    {
        cerr << "Process1: incorrect agmunets quantity" << endl;
        exit(EXIT_FAILURE);
    }

    pipe_fd = atoi(argv[1]);

    file_descriptor = open("p1.txt", O_RDONLY);

    signal(SIGUSR1, signal_handler);
        signal(SIGUSR2,
           [](int signal)
           {
               cout << "P1: got usr2, exiting..." << endl;
               exit(EXIT_SUCCESS);
           });
    cout << "proc_p1: can get signals" << endl;

    if(kill(getppid(), SIGUSR1) < 0)
    {
        cerr << "P1: kill error" << endl;
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        sleep(1);
    }

    return 0;
}
