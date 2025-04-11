#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace std;
int pipe_fd;
int file_descriptor;

void signal_handler(int signum) 
{
    cout << "P2: got signal (" << signum << ")" << endl;

    char buffer[151];
    char current_symbol;
    ssize_t counter = 0;

    if (file_descriptor < 0) 
    {
        cerr << "Error: Unable to open p2.txt.\n";
        exit(EXIT_FAILURE);
    }

    while (read(file_descriptor, &current_symbol, 1) > 0) 
    {
        if (current_symbol == '\n') 
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

    if (write(pipe_fd, buffer, counter) < 0) 
    {
        perror("P2: Failed to write to pipe");
        exit(EXIT_FAILURE);
    }
    printf("P2: After write: %s, counter: %ld\n", buffer, counter);
}

int main(int argc, char* argv[]) 
{
    if (argc != 2) 
    {
        cerr << "Process2: incorrect agmunets quantity" << endl;
        exit(EXIT_FAILURE);
    }

    if(signal(SIGUSR2,
           [](int signal)
           {
               cout << "P2: got usr2, exiting..." << endl;
               exit(EXIT_SUCCESS);
           }) == SIG_ERR){
                perror("P2: SIGUSR2 error");
                exit(EXIT_FAILURE);
           }

    pipe_fd = stoi(argv[1]);

    file_descriptor = open("p2.txt", O_RDONLY);

    signal(SIGUSR1, signal_handler);
    cout << "proc_p2: Ready to process signals.\n";

    if(kill(getppid(),SIGUSR1) < 0)
    {
        cerr << "P2: Error to send SIGUSR1 to parent process" << endl;
        exit(EXIT_FAILURE);
    }

    while (true) 
    {
        sleep(1);
    }

    return 0;
}
