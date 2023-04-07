#include<unistd.h>
#include<stdio.h>
#include<syscall.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>

int mode = 1;

int main(void){
	pid_t pid;
	pid = fork();
	if (pid1 < 0){
		printf("I/O process fork failed");
		return 1;
	}
	else if (pid1 == 0){
		
	}
	else {
		pid_t pid2;
		pid2 = fork();
		if (pid2 < 0){
			print("merge process fork failed");
			return 1;
		}
	}
}
