CC = g++
CFLAGS = -Wall -Wformat -std=c++11

all: ipc_launcher proc_reader_p1 proc_reader_p2 pipe_to_shm shm_to_socket udp_receiver proc_s proc_pr

ipc_launcher: ipc_launcher.cpp
	$(CC) $(CFLAGS) ipc_launcher.cpp -o ipc_launcher

proc_reader_p1: proc_reader_p1.cpp
	$(CC) $(CFLAGS) proc_reader_p1.cpp -o proc_reader_p1

proc_reader_p2: proc_reader_p2.cpp
	$(CC) $(CFLAGS) proc_reader_p2.cpp -o proc_reader_p2

pipe_to_shm: pipe_to_shm.cpp
	$(CC) $(CFLAGS) pipe_to_shm.cpp -o pipe_to_shm

shm_to_socket: shm_to_socket.cpp
	$(CC) $(CFLAGS) shm_to_socket.cpp -o shm_to_socket

udp_receiver: udp_receiver.cpp
	$(CC) $(CFLAGS) udp_receiver.cpp -o udp_receiver


clean:
	rm -f ipc_launcher proc_reader_p1 proc_reader_p2 pipe_to_shm shm_to_socket udp_receiver proc_s proc_pr
