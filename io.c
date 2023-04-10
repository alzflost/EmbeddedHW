#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/stat.h>
#include<sys/types.h>
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



unsigned char quit = 0;

void user_signal1(int sig) 
{
	quit = 1;
}

int main(){

	// open required device drivers
	int dev_dip_switch = open(DIP_SWITCH, O_RDWR);
	if (dev_dip_switch < 0){
		printf("err");
		close(dev_dip_switch);
		return -1;
	}
	int dev_fnd = open(FND_DEVICE, O_RDWR);
	if (dev_fnd < 0){
		printf("err");
                close(dev_fnd);
                return -1;
	}
	int dev_led = open(LED_DEVICE, O_RDWR);
	if (dev_led < 0){
		printf("err led");
                close(dev_led);
                return -1;
	}
	int dev_push_switch = open(PUSH_SWITCH, O_RDWR);
        if (dev_push_switch < 0){
                printf("err push swi");
                close(dev_push_switch);
                return -1;
        }
	int dev_motor = open(MOTOR_DEVICE, O_RDWR);
        if (dev_motor < 0){
                printf("err motor");
                close(dev_motor);
                return -1;
        }
	int dev_lcd = open(LCD_DEVICE, O_RDWR);
        if (dev_lcd < 0){
                printf("err lcd");
                close(dev_lcd);
                return -1;
        }
	int dev_event = open(KEY_EVENT, O_RDWR);
        if (dev_event < 0){
                printf("err");
                close(dev_event);
                return -1;
        }
	
	unsigned char push_sw_buff[MAX_BUTTON];
	int buff_size;
	int i;
	unsigned char data[4];
	for (i=0; i<4; i++){
		data[i] = 0;
	}

	unsigned char retval;
	buff_size = sizeof(push_sw_buff);
	int number = 0;
	(void)signal(SIGINT, user_signal1);
	
	while(!quit){
		usleep(400000);
		memset(data,0,sizeof(data));
		retval = read(dev_push_switch, &push_sw_buff, buff_size);
		if (retval < 0){
			printf("read err\n");
			return -1;
		}
		for (i=0; i<9; i++){
			if (push_sw_buff[i]){
				data[0] = (i+1);
			}
		}
		retval = write(dev_fnd, &data, 4);
		if (retval < 0){
			printf("write err\n");
			return -1;
		}
	}
	close(dev_dip_switch);
	close(dev_fnd);
	close(dev_led);
	close(dev_push_switch);
	close(dev_motor);
	close(dev_lcd);
	close(dev_event);
	return 0;
}
