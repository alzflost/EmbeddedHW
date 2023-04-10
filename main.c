#include<unistd.h>
#include<stdio.h>
#include<syscall.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<signal.h>

int mode = 1;

int io(void){

}

int merge(void){

}

int main(void){

	key_t key;
	int shmid;
	key = ftok("myfile", 1);
	shmid = shmget(key, 1024, IPC_CREAT|0644);
	if (shmid == -1){
		perror("shmget");
		exit(1);
	}

	pid_t pid;
	pid = fork();
	if (pid1 < 0){
		printf("I/O process fork failed");
		return 1;
	}
	else if (pid1 == 0){
		io();
	}
	else {
		pid_t pid2;
		pid2 = fork();
		if (pid2 < 0){
			print("merge process fork failed");
			return 1;
		}
		else if (pid2 == 0){
			merge();
		}
		else {
			while(1){
				if (mode == 1){
					handle_put();	
				}
				else if (mode == 2){
					handle_get();
				}
				else if (mode == 3){
					handle_merge();
				}
		}
	}
	
}
