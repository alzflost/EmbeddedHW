#include "hw1.h"

// unsigned char quit = 0;
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


// 1 if user is inserting something.
int input_start = 0;
// 0 for key, 1 for value
int put_mode = 0;
// 0 for key
int get_mode = 0;
// 0 for english, 1 for number
int switch_eng_num = 1;

// current number on FND
int fnd_cur = 0;
// current string on LCD
unsigned char lcd_string[32];

// current status of LED
int led_status = 0;
// if blinking, which number is on?
int led_low_high = 0;
unsigned char fnd_status[4];

// time cnt for motor ON
int motor_time_cnt;

struct input_event ev[EVENT_BUF_SIZE];
int size = sizeof(struct input_event);

/*
void user_signal1(int sig) 
{
	quit = 1;
}
*/

int open_devices(void){
	// open required device drivers
	
	// dip switch (reset button)
        dev_fds[0] = open(DIP_SWITCH, O_RDWR|O_NONBLOCK);
        if (dev_fds[0] < 0){
                printf("err dip switch");
                close(dev_fds[0]);
                return -1;
        }
	// fnd
        dev_fds[1] = open(FND_DEVICE, O_RDWR);
        if (dev_fds[1] < 0){
                printf("err fnd");
                close(dev_fds[1]);
                return -1;
        }
	// led
        dev_fds[2] = open(LED_DEVICE, O_RDWR);
        if (dev_fds[2] < 0){
                printf("err led");
                close(dev_fds[2]);
                return -1;
        }
	// switch
        dev_fds[3] = open(PUSH_SWITCH, O_RDWR);
        if (dev_fds[3] < 0){
                printf("err push switch");
                close(dev_fds[3]);
                return -1;
        }
	// motor
        dev_fds[4] = open(MOTOR_DEVICE, O_RDWR);
        if (dev_fds[4] < 0){
                printf("err motor");
                close(dev_fds[4]);
                return -1;
        }
	// lcd
        dev_fds[5] = open(LCD_DEVICE, O_RDWR);
        if (dev_fds[5] < 0){
                printf("err lcd");
                close(dev_fds[5]);
                return -1;
        }
	// prog back vol+ vol-
        dev_fds[6] = open(KEY_EVENT, O_RDONLY|O_NONBLOCK);
        if (dev_fds[6] < 0){
                printf("err key event");
                close(dev_fds[6]);
                return -1;
        }
	return 0;
}

// close devices
int close_devices(void){
	int i = 0;
	for(i=0; i<NUM_DEVICES; i++){
		if (close(dev_fds[i]) != 0){
			return -1;
		}
	}
	return 0;
}

void io_put(void){
	// all LED ON
	// write in shared memory	
}

void io_get(void){
	// all LED ON
	// write in shared memory
}

void io_merge(void){
	// write in shared memory
}


// switch button handler
unsigned char in_switch(void){
	unsigned char switch_buf[MAX_BUTTON];
	int buf_size = sizeof(switch_buf);
	memset(switch_buf, 0, sizeof(switch_buf));

	int retval = 0;
	// read from device
	retval = read(dev_fds[3], &switch_buf, buf_size);
	if (retval < 0){
		printf("read error");
	}
	all_erase = 0;
	suc_erase = 0;


	if (flags->mode == 0 && switch_buf[1] && switch_buf[2]){
		// key or value reset
		// LED ->  only #1 ON
		input_start = 0;
		// lcd clear or fnd clear
		all_erase = 1;
		// switch_suc clear
		memset(switch_suc, -1, sizeof(switch_suc));
		return 0;
	}
	else if (flags->mode == 0 && switch_buf[4] && switch_buf[5]){
		// english <-> number toggle
		switch_eng_num = 1 - switch_eng_num;
		// switch_suc clear
		memset(switch_suc, -1, sizeof(switch_suc));
		return 0;
	}
	else if (flags->mode == 0 && switch_buf[7] && switch_buf[8]){
		// lcd clear
		all_erase = 1;
		// switch_suc clear
		memset(switch_suc, -1, sizeof(switch_suc));
		// save : request PUT to main
		io_put();
		return 0;
	}
	
	int i;
	for(i=0; i<9; i++){
		 if (switch_buf[i]){
			 input_start = 1;
			 // number case
			 if (!put_mode || switch_eng_num){
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
	unsigned char dip_buf;
	int retval = read(dev_fds[0], &dip_buf, 1);
	if (dip_buf == 0){
		// put_mode
		if (flags->mode == 0){
			input_start = 0;
			put_mode = 1 - put_mode;
			fprintf(stderr, "put mode:%d\n0 is key\n1 is value\n", put_mode);
			// PUT : toggle key-value
		}
		else if (flags->mode == 1){
			input_start = 0;
			io_get();
			// GET : request GET
		}
		else if (flags->mode == 2){
			input_start = 0;
			io_merge();
			// Merge : request MERGE
		}
	}
	if (retval < 0){
		printf("reset read err\n");
	}
	
}

void in_event(){
	int rd = read(dev_fds[6], ev, size * EVENT_BUF_SIZE);
	int value = ev[0].value;
	if (ev[0].type == 1 && ev[0].value == 1){
		// VOL+ key
		if (ev[0].code == 115){
			input_start = 0;
			flags->mode = (flags->mode+1) % 3;
			fprintf(stderr, "mode:%d\n", mode);
		}
		// VOL- Key
		else if (ev[0].code == 114){
			input_start = 0;
			flags->mode = (flags->mode-1) % 3;
			fprintf(stderr, "mode:%d\n", mode);
		}
		// BACK key
		else if (ev[0].code == 158){
			input_start = 0;
			exit(-1);
		}
	}
	return;
}

void out_lcd(unsigned char ch){
	int len = strlen(lcd_string);
	// merge mode : no usage
	if (flags->mode == 2){
		return;
	}
	else if (flags->mode == 1){
		// later
		return;
	}
	else if (flags->mode == 0){
		if (!put_mode) return;
		if (put_mode && all_erase){
			memset(lcd_string, 0, sizeof(lcd_string));
			write(dev_fds[5], lcd_string, 32);
			all_erase = 0;
			return;
		}
		if (ch == 0) return;
		if (len == 31) return;
		if (suc_erase){
			len--;
		}
		lcd_string[len] = ch;
		lcd_string[len+1] = '\0';
	}

	int retval = write(dev_fds[5], lcd_string, 32);
	if (retval < 0){
		fprintf(stderr, "err lcd write");
	}
	return;
}

void out_led(){
	// merge mode : no usage
	if (flags->mode == 2){
		return;
	}

	unsigned long *fpga_addr = 0;
	unsigned char *led_addr = 0;

	// not exact second (ex 3.75sec, 5.5sec) or not inserting anything yet
	// no need for update : return
	if ((time_cnt&3) || !input_start){
		return;
	}
	// put mode and inserting something
	else if (flags->mode == 0){
		// inserting key : LED 3&4 alternately blink
		if (put_mode == 0){
			led_low_high = 1 - led_low_high;
			led_status = 1 << (4+led_low_high);
		}
		// inserting value : LED 7&8 alternately blink
		else {
			led_low_high = 1 - led_low_high;
			led_status = 1 << led_low_high;
		}
	}
	// get mode and inserting something
	else if (flags->mode == 1){
		led_low_high = 1 - led_low_high;
		led_status = 1 << (4+led_low_high);
	}

	// mmap
	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fds[2], FPGA_BASE_ADDRESS);
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

void out_fnd(unsigned char ch){
	// merge mode : no usage
	if (flags->mode == 2) {
		return;
	}
	// put mode
	if (flags->mode == 0) {
		if (put_mode == 1){
			return;
		}
		// inserting key and pressed reset
		else if (put_mode == 0 && all_erase){
			memset(fnd_status, 0, sizeof(fnd_status));
			fnd_cur = 0;
			write(dev_fds[1], fnd_status, 4);
			all_erase = 0;
			return;
		}
	}
	if (ch == 0) return;
	fnd_status[fnd_cur] = ch - 0x30;
	fnd_cur = (fnd_cur+1)%4;

	int retval = write(dev_fds[1], &fnd_status, 4);
	if (retval < 0){
		fprintf(stderr, "err fnd write");
	}
	return;
}

void out_motor(){
	if (flags->mode == 2){
		if (flags->request == 2){
			// motor ON
		}
	}
	return;
}

void io(){
	flags = (SHM_FLAGS *)shmat(shm_flags_id, (SHM_FLAGS*)NULL, 0);

	int open_ret;
	open_ret = open_devices();
	if (open_ret < 0){
		printf("open error");
		return;
	}
	
	//(void)signal(SIGINT, user_signal1);
	
	time_cnt = 0;
	memset(lcd_string, 0, sizeof(lcd_string));
	memset(switch_suc, -1, sizeof(switch_suc));
	memset(fnd_status, 0, sizeof(fnd_status));

	while(!quit){
		usleep(250000);
		time_cnt++;
		unsigned char ch = 0;
		ch = in_switch();
		in_reset();
		in_event();
		
		out_led();
		out_fnd(ch);
		out_lcd(ch);
		out_motor();
	}

	if (close_devices() < 0){
		printf("close error");
		return;
	}
	return;
}
