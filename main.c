#include "hw1.h"

extern int mode;

int main(void){
	
	key_t key;
	int shmid;
	key = ftok("myfile", 1);
	shmid = shmget(key, 1024, IPC_CREAT|0644);
	if (shmid == -1){
		perror("shmget");
		exit(1);
	}

	pid_t pid1;
	pid1 = fork();
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
			printf("merge process fork failed");
			return 1;
		}
		else if (pid2 == 0){
			merge();
		}
		else {
			while(1){
				/*
				if (mode == 1){
						
				}
				else if (mode == 2){
					handle_get();
				}
				else if (mode == 3){
					handle_merge();
				}
				*/
			}
		}
	}
	
}
