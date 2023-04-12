#include "hw1.h"

unsigned char quit = 0;
extern int mode;
int time_cnt = 0;

int dev_fds[NUM_DEVICES];

int switch_suc[9];
int suc_erase = 0;
int all_erase = 0;
unsigned char switch_keyboard[9][3] = {
	{'.', 'Q', 'Z'},
	{'A', 'B', 'C'},
	{'D', 'E', 'F'},
	{'G', 'H', 'I'},
	{'J', 'K', 'L'},
	{'M', 'N', 'O'},
	{'P', 'R', 'S'},
	{'T', 'U', 'V'},
	{'W', 'X', 'Y'}
};

unsigned char switch_num[9] = {'1','2','3','4','5','6','7','8','9'};

// 0 for key, 1 for value
int put_mode = 0;
// 0 for key
int get_mode = 0;
// 0 for english, 1 for number
int switch_eng_num = 1;

int fnd_cur = 0;
unsigned char lcd_string[32];
int led_status = 0;

void user_signal1(int sig) 
{
	quit = 1;
}

int open_devices(void){
	// open required device drivers
        dev_fds[0] = open(DIP_SWITCH, O_RDWR);
        if (dev_fds[0] < 0){
                printf("err dip switch");
                close(dev_fds[0]);
                return -1;
        }
        dev_fds[1] = open(FND_DEVICE, O_RDWR);
        if (dev_fds[1] < 0){
                printf("err fnd");
                close(dev_fds[1]);
                return -1;
        }
        dev_fds[2] = open(LED_DEVICE, O_RDWR);
        if (dev_fds[2] < 0){
                printf("err led");
                close(dev_fds[2]);
                return -1;
        }
        dev_fds[3] = open(PUSH_SWITCH, O_RDWR);
        if (dev_fds[3] < 0){
                printf("err push switch");
                close(dev_fds[3]);
                return -1;
        }
        dev_fds[4] = open(MOTOR_DEVICE, O_RDWR);
        if (dev_fds[4] < 0){
                printf("err motor");
                close(dev_fds[4]);
                return -1;
        }
        dev_fds[5] = open(LCD_DEVICE, O_RDWR);
        if (dev_fds[5] < 0){
                printf("err lcd");
                close(dev_fds[5]);
                return -1;
        }
        dev_fds[6] = open(KEY_EVENT, O_RDONLY|O_NONBLOCK);
        if (dev_fds[6] < 0){
                printf("err key event");
                close(dev_fds[6]);
                return -1;
        }

}

int close_devices(void){
	int i = 0;
	for(i=0; i<NUM_DEVICES; i++){
		close(dev_fds[i]);
	}
}

unsigned char in_switch(void){
	unsigned char switch_buf[MAX_BUTTON];
	int buf_size = sizeof(switch_buf);
	memset(switch_buf, 0, sizeof(switch_buf));

	int retval = 0;
	retval = read(dev_fds[3], &switch_buf, buf_size);
	if (retval < 0){
		printf("read error");
	}
	all_erase = 0;
	suc_erase = 0;
	if (switch_buf[1] && switch_buf[2]){
		// key or value init
		// lcd clear
		all_erase = 1;
		// switch_suc clear
		memset(switch_suc, -1, sizeof(switch_suc));
		return 0;
	}
	else if (switch_buf[4] && switch_buf[5]){
		// english <-> number toggle
		switch_eng_num = 1 - switch_eng_num;
		// switch_suc clear
		memset(switch_suc, -1, sizeof(switch_suc));
		return 0;
	}
	else if (switch_buf[7] && switch_buf[8]){
		// save : request PUT to main
		// lcd clear
		all_erase = 1;
		// switch_suc clear
		memset(switch_suc, -1, sizeof(switch_suc));
		
		return 0;
	}
	
	int i;
	for(i=0; i<9; i++){
		 if (switch_buf[i]){
			 // number case
			 if (switch_eng_num){
				 return switch_num[i];
			 }
			 if (switch_suc[i] == -1){
				memset(switch_suc, -1, sizeof(switch_suc));
				suc_erase = 0;
			 }
			 else {
				suc_erase = 1;
			 }
			 switch_suc[i] = (switch_suc[i] + 1) % 3;
			 return switch_keyboard[i][switch_suc[i]];
		 }
	}
	return 0;
}

void in_reset(){
	unsigned char dip_buf = 0;
	int retval = read(dev_fds[0], &dip_buf, 1);
	if (dip_buf){
		
	}
	if (retval < 0){
		printf("reset button err");
		return;
	}
	
}

void in_event(){
	return;
}

void out_lcd(unsigned char ch){
	int len = strlen(lcd_string);
	if (all_erase) {
		memset(lcd_string, 0, sizeof(lcd_string));
	}
	if (len == 31) return;
	if (suc_erase) {
		len--;
	}
	lcd_string[len] = ch;
	lcd_string[len+1] = '\0';

	int retval = write(dev_fds[5], lcd_string, 32);
	if (retval < 0){
		fprintf(stderr, "err lcd write");
	}
	return;
}

void out_led(){
	unsigned long *fpga_addr = 0;
	unsigned long *led_addr = 0;
	if ((time_cnt&3)){
		
	}
	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED){
		printf("mmap error!");
		close(dev_fds[2]);
		exit(1);
	}

	led_addr=(unsigned char*)((void *)fpga_addr+LED_ADDR);
	*led_addr = led_status;

	munmap(led_addr, 4096);
	return;
}

void out_fnd(char ch){
	return;
}

void out_motor(){
	return;
}

void io(){
	int open_ret;
	open_ret = open_devices();
	if (open_ret < 0){
		printf("open error");
		return;
	}
	
	(void)signal(SIGINT, user_signal1);
	
	time_cnt = 0;
	memset(lcd_string, 0, sizeof(lcd_string));
	memset(switch_suc, -1, sizeof(switch_suc));
	while(!quit){
		usleep(500000);
		time_cnt++;
		unsigned char ch = 0;
		ch = in_switch();
		in_reset();
		in_event();

		out_led();
		if (ch != 0){
			fprintf(stderr, "%d", ch);
			out_lcd(ch);
			out_fnd(ch);
		}
		out_motor();
	}

	int close_ret = close_devices();
	if (close_ret < 0){
		printf("close error");
		return;
	}
	return;
}
