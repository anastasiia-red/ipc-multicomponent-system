#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <errno.h>

using namespace std;

bool serv2_ended = 0;

struct processData
{
    pid_t P1;
    pid_t P2;
    pid_t PR;
    pid_t T;
    pid_t D;
    pid_t S;
    pid_t Serv1;
    pid_t Serv2;
};

pid_t start_process(const char *name, const vector<string> &args);
void terminate_children(int signal);
pair<string, string> pipe_create(const string &name);
void exit_processes();
void signal_2_handler(int signal);
string sm_create();
string semaphore_create();
processData data;
void usr1_handler(int signum);


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "zadanie <port1> <port2>" << endl;
        exit(EXIT_FAILURE);
    }

    string port1 = argv[1];
    string port2 = argv[2];

    signal(SIGUSR1, usr1_handler);
    signal(SIGUSR2, signal_2_handler);

    pair<string, string> pipe1 = pipe_create("pipe1");
    pair<string, string> pipe2 = pipe_create("pipe2");

    string sem_T_descriptor = semaphore_create();
    string sem_D_descriptor = semaphore_create();

    string sm_t = sm_create();
    string sm_d = sm_create();

    cout << "Zadanie: Starting processes..." << endl;

    data.Serv1 = start_process("./proc_serv1", {port1, port2});
    if (data.Serv1 < 0)
    {
        cerr << "Zadanie: Exec serv1 error" << endl;
        exit(EXIT_FAILURE);
    }

    data.Serv2 = start_process("./udp_receiver", {port1, port2});
    if (data.Serv2 < 0)
    {
        cerr << "Zadanie: Exec udp_receiver error" << endl;
        exit(EXIT_FAILURE);
    }

    data.D = start_process("./shm_to_socket", {sm_d, sem_D_descriptor, port1});
    if (data.D < 0)
    {
        cerr << "Zadanie: Exec shm_to_socket error" << endl;
        exit(EXIT_FAILURE);
    }

    data.P1 = start_process("./proc_reader_p1", {pipe1.second});
    if (data.P1 < 0)
    {
        cerr << "Zadanie: Exec proc_reader_p1 error" << endl;
        exit(EXIT_FAILURE);
    }

    data.P2 = start_process("./proc_reader_p2", {pipe1.second});
    if (data.P2 < 0)
    {
        cerr << "Zadanie: Exec proc_reader_p1 error" << endl;
        exit(EXIT_FAILURE);
    }

    data.PR = start_process("./proc_pr", {to_string(data.P1), to_string(data.P2), pipe1.first, pipe2.second});
    if (data.PR < 0)
    {
        cerr << "Zadanie: Exec PR error" << endl;
        exit(EXIT_FAILURE);
    }

    data.T = start_process("./pipe_to_shm", {pipe2.first, sm_t, sem_T_descriptor});
    if (data.T < 0)
    {
        cerr << "Zadanie: Exec pipe_to_shm error" << endl;
        exit(EXIT_FAILURE);
    }

    data.S = start_process("./proc_s", {sm_t, sem_T_descriptor, sm_d, sem_D_descriptor});
    if (data.S < 0)
    {
        cerr << "Zadanie: Exec S error" << endl;
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        if(serv2_ended){
            break;
        }
    }

    exit_processes();

    semctl(stoi(sem_D_descriptor), 2, IPC_RMID);
    semctl(stoi(sem_T_descriptor), 2, IPC_RMID);

    shmctl(stoi(sm_d), 2, NULL);
    shmctl(stoi(sm_t), 2, NULL);

    cout << "All processes finished. Exiting." << endl;
    return 0;
}

void exit_processes(){
    kill(data.P1, SIGUSR2);
    kill(data.P2, SIGUSR2);
    kill(data.PR, SIGTERM);
    kill(data.T, SIGUSR2);
    kill(data.D, SIGUSR2);
    kill(data.S, SIGTERM);
    kill(data.Serv1, SIGTERM);
    kill(data.Serv2, SIGUSR2);
}

void signal_2_handler(int signal)
{
    serv2_ended = 1;
}

void usr1_handler(int signum){
    cout << "Zadanie: got SIGUSR1" << endl;
}

pair<string, string> pipe_create(const string &name)
{
    cout << "Zadanie: creation of pipe: " << name << endl;

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        cerr << "Zadanie: fail of creating pipe: " << name << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Pipe created: " << name << endl;

    pair<string, string> result;
    result.first = to_string(pipe_fd[0]);
    result.second = to_string(pipe_fd[1]);

    return result;
}

pid_t start_process(const char *name, const vector<string> &args)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        vector<const char *> exec_args;
        exec_args.push_back(name);

        for (const string &arg : args)
        {
            exec_args.push_back(arg.c_str());
        }

        exec_args.push_back(nullptr);

        if (execve(name, const_cast<char *const *>(exec_args.data()), nullptr) == -1)
        {
            fprintf(stderr, "execve failed process: %s\n", name);
            perror("Error details");
            exit(EXIT_FAILURE);
        }
    }

    else if (pid == -1)
    {
        perror("Process can`t be created");
        exit(EXIT_FAILURE);
    }

    else
    {
        printf("%s proccess begin\n", name);
        pause();
    }

    return pid;
}

string semaphore_create()
{
    int descriptor = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
    if (descriptor == -1)
    {
        cerr << "Semaphore creation failed" << endl;
        exit(EXIT_FAILURE);
    }

    semctl(descriptor, 0, SETVAL, 0);
    semctl(descriptor, 1, SETVAL, 0);

    cout << "Zadanie: semaphore created!" << endl;
    string descriptor_string = to_string(descriptor);
    return descriptor_string;
}

string sm_create()
{
    cout << "Zadanie: shared memory creating success" << endl;

    int shared_memory_descriptor = shmget(IPC_PRIVATE, 256, IPC_CREAT | 0666);
    if (shared_memory_descriptor == -1)
    {
        cerr << "Shared memory creation failed" << endl;
        exit(EXIT_FAILURE);
    }
    
    cout << "Shared memory creation success" << endl;
    string shared_memory_descriptor_string = to_string(shared_memory_descriptor);
    return shared_memory_descriptor_string;
}
