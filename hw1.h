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

#define DIP_SWITCH "/dev/fpga_dip_switch"
#define FND_MAX_DIGIT 4
#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/fpga_led"
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

#define NUM_DEVICES 7
#define NUM_DEV_DIP_SWITCH 0
#define NUM_DEV_FND 1
#define NUM_DEV_LED 2
#define NUM_DEV_PUSH_SWITCH 3
#define NUM_DEV_MOTOR 4
#define NUM_LCD 5
#define NUM_KEY_EVENT 6

int mode;

void io();
void merge();
