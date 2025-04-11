# Interprocess Communication System (Linux | C++ | IPC)

This project demonstrates a complex **multi-process architecture** using various Linux **Interprocess Communication (IPC)** mechanisms.

It simulates data flow through several custom processes using:
- unnamed pipes
- shared memory
- semaphores
- signals
- TCP and UDP sockets

All processes are launched and managed by a central launcher process.

---

## Features

-  Real-time interaction between multiple child processes
-  Custom process synchronization via `SIGUSR1` / `SIGUSR2`
-  Data flow through:
  - **pipes** → **shared memory** → **network sockets**
-  UDP receiver that writes incoming data to a file
-  Complete lifecycle management (creation & cleanup)

---

## 🛠 Technologies Used

| Tool          | Usage                                      |
|---------------|--------------------------------------------|
| **C++**       | Language for all components                |
| **UNIX IPC**  | Pipes, Shared Memory, Semaphores, Signals  |
| **POSIX**     | System calls (`fork`, `execve`, etc.)      |
| **Sockets**   | TCP (client), UDP (receiver)               |
| **Makefile**  | For compiling all components               |

---

| Process           | Description                                                                 |
|-------------------|-----------------------------------------------------------------------------|
| `ipc_launcher`     | Spawns and synchronizes all subprocesses                                   |
| `proc_reader_p1`   | Reads lines from `p1.txt`, sends via pipe                                   |
| `proc_reader_p2`   | Reads from `p2.txt`                                                         |
| `proc_pr`          | Merges two readers' output into a new pipe                                  |
| `pipe_to_shm`      | Reads from pipe → writes into shared memory (process T)                     |
| `proc_s`           | Moves/transforms data between two shared memory blocks                      |
| `shm_to_socket`    | Sends shared memory data via TCP (process D)                                |
| `udp_receiver`     | Listens on UDP port, writes to file `serv2.txt`


---

# Launch all processes
./ipc_launcher <tcp_port> <udp_port>

# Output
You will see console logs from each process (e.g., proc_reader_p1, proc_s, etc.), and serv2.txt will receive final UDP messages.


              

