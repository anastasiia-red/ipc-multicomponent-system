#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <cstring> 
#include <stdio.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

using namespace std;
#define BUFF_SIZE 256

int main(int argc, char* argv[])
{
    if (argc < 3) 
    {
        cerr << "zadanie <port1> <port2>" << endl;
        exit(EXIT_FAILURE);
    }

    string port2 = argv[2];
        signal(SIGUSR2,
               [](int signal)
               {
                   cout << "Serv2: got usr2, exiting..." << endl;
                   exit(EXIT_SUCCESS);
               });

    if (kill(getppid(), SIGUSR1) == -1)
    {
        cerr << "Serv2: SIGUSR1 kill error" << endl;
        exit(EXIT_FAILURE);
    }

    int socket_descriptor;
    if ((socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        cerr << "Serv2: Socket can`t be created" << endl; 
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET; 
    my_addr.sin_port = htons(stoi(port2));
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_descriptor, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1)
    {
        cerr << "Serv2: bind socket error" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Serv2: Socket je vytovereny" << endl;
    }

    int serv2_file = open("serv2.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (serv2_file == -1) {
        cerr << "Serv2: cant open file 'serv2.txt'" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Serv2: file opened\n"<< endl;
    
    char buffer[BUFF_SIZE];
    for (size_t i = 0; i < 10; i++)
    {
        ssize_t get_size = recvfrom(socket_descriptor, &buffer, BUFF_SIZE, 0, nullptr, nullptr);
        if(get_size == -1){
            cerr << "Serv2: recvfrom error" << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Serv2: Got data: " << buffer << endl;

        size_t buf_len = strlen(buffer);
        buffer[buf_len] = '\n';
        buffer[buf_len + 1] = '\0';

        size_t result_bytes = write(serv2_file, buffer, strlen(buffer));
        if (result_bytes == -1UL)
        {
            perror("Serv2: fwrite error");
            exit(EXIT_FAILURE);
        }
        printf("Serv2: writed data: %s with size: %ld\n", buffer, strlen(buffer));
    }
    printf("Serv2: exit of loop\n");

    kill(getppid(), SIGUSR2);

    return 0;

}