#include<stdio.h>
#include<unistd.h>
#include<syscall.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<string.h>
#include<linux/input.h>
#include<sys/mman.h>
#include<dirent.h>


#define DIP_SWITCH "/dev/fpga_dip_switch"
#define FND_MAX_DIGIT 4
#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/mem"
#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16
#define PUSH_SWITCH "/dev/fpga_push_switch"
#define MAX_BUTTON 9
#define MOTOR_DEVICE "/dev/fpga_step_motor"
#define LCD_MAX_BUFF 32
#define LCD_LINE_BUFF 16
#define LCD_DEVICE "/dev/fpga_text_lcd"
#define KEY_BUFF 65
#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_EVENT "/dev/input/event0"
#define EVENT_BUF_SIZE 64

#define NUM_DEVICES 7
#define NUM_DEV_DIP_SWITCH 0
#define NUM_DEV_FND 1
#define NUM_DEV_LED 2
#define NUM_DEV_PUSH_SWITCH 3
#define NUM_DEV_MOTOR 4
#define NUM_LCD 5
#define NUM_KEY_EVENT 6

#define SHM_KEY_FLAGS (key_t) 0x40
// define SHM_KEY_DATA (key_t) 0x40
// define SHM_KEY_REQUEST (key_t) 0x50
// define SHM_KEY_MODE (key_t) 0x60
// define SHM_KEY_ST (key_t) 0x70
#define SEMA_KEY (key_t) 0x80

// mode - shared memory
// put 0 get 1 merge 2
int* mode;
// number of storage table elements.
// merges when 3
int* st_num;
// quit status : exit when 1 
int* quit;

typedef struct {
	int mode;
	int st_num;
	int quit;
	int request;
} SHM_FLAGS;

SHM_FLAGS *flags;
int shm_flags_id;
int shm_mode_id;
int shm_st_num_id;
int shm_exit_id;

int sem_id;

void io();
void merge();
