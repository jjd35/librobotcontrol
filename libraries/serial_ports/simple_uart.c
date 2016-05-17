/*******************************************************************************
* simple_uart.c
*
* This is a collection of C functions to make interfacing with UART ports on 
* the BeagleBone easier. This could be used on other linux platforms too.
*
*******************************************************************************/

#include "../robotics_cape.h"
#include "../useful_includes.h"

#define MIN_BUS 0
#define MAX_BUS 5

/*******************************************************************************
* Local Global Variables
*******************************************************************************/
int initialized[MAX_BUS-MIN_BUS+1]; // keep track of if a bus is initialized
char *paths[6] = { \
	"/dev/ttyO0", \
	"/dev/ttyO1", \
	"/dev/ttyO2", \
	"/dev/ttyO3", \
	"/dev/ttyO4", \
	"/dev/ttyO5" };

int fd[6];


/*******************************************************************************
* int initialize_uart(int bus, int baudrate)
* 
* returns -1 for failure or 0 for success
*******************************************************************************/ 
int initialize_uart(int bus, int baudrate){

	struct termios config;
	speed_t speed; //baudrate
	
	// sanity checks
	if(bus<MIN_BUS || bus>MAX_BUS){
		printf("ERROR: uart bus must be between %d & %d\n", MIN_BUS, MAX_BUS);
		return -1;
	}
	
	switch(baudrate){
	case (230400): 
		speed=B230400;
		break;
	case (115200): 
		speed=B115200;
		break;
	case (57600): 
		speed=B57600;
		break;
	case (38400): 
		speed=B38400;
		break;
	case (19200): 
		speed=B19200;
		break;
	case (9600): 
		speed=B9600;
		break;
	case (4800): 
		speed=B4800;
		break;
	case (2400): 
		speed=B2400;
		break;
	case (1800): 
		speed=B1800;
		break;
	case (1200): 
		speed=B1200;
		break;
	case (600): 
		speed=B600;
		break;
	case (300): 
		speed=B300;
		break;
	case (200): 
		speed=B200;
		break;
	case (150): 
		speed=B150;
		break;
	case (134): 
		speed=B134;
		break;
	case (110): 
		speed=B110;
		break;
	case (75): 
		speed=B75;
		break;
	case (50): 
		speed=B50;
		break;
	default:
		printf("ERROR: invalid speed. Please use a standard baudrate\n");
		return -1;
	}
	
	// close the bus in case it was already open
	close_uart(bus);
	
	// open file descriptor for blocking reads
	if ((fd[bus] = open(paths[bus], O_RDWR | O_NOCTTY)) < 0) {
		printf("error opening uart%d in /dev/\n", bus);
		printf("device tree overlay probably isn't loaded\n");
		return -1;
	}
	
	// set up tc config
	memset(&config,0,sizeof(config));
	config.c_iflag=0;
	config.c_iflag=0;
    config.c_oflag=0;
    config.c_cflag= CS8|CREAD|CLOCAL;   // 8n1, see termios.h for more info
    config.c_lflag=0;
    config.c_cc[VTIME]=0; // no timeout condition
	config.c_cc[VMIN]=1;  // only return if something is in the buffer
	
	
	if(cfsetispeed(&config, speed) < 0) {
		printf("ERROR: cannot set uart%d baud rate\n", bus);
		return -1;
	}
	if(cfsetospeed(&config, speed) < 0) {
		printf("ERROR: cannot set uart%d baud rate\n", bus);
		return -1;
	}
	
	
	if(tcsetattr(fd[bus], TCSAFLUSH, &config) < 0) { 
		printf("cannot set uart%d attributes\n", bus);
		return -1;
	}
	
	initialized[bus] = 1;
	
	flush_uart(bus);
	return 0;
}


/*******************************************************************************
*	int close_uart(int bus)
*
* If the bus is open and has been initialized, close it and return 0.
* If the bus in uninitialized, just return right away.
* Return -1 if bus is out of bounds.
*******************************************************************************/
int close_uart(int bus){
	// sanity checks
	if(bus<MIN_BUS || bus>MAX_BUS){
		printf("ERROR: uart bus must be between %d & %d\n", MIN_BUS, MAX_BUS);
		return -1;
	}
	
	// if not initialized already, return
	if(initialized[bus]==0){
		return 0;
	}
	
	close(fd[bus]);
	initialized[bus]=0;
	return 0;
}




/*******************************************************************************
* int get_uart_fd(int bus)
*
* Returns the file descriptor for a uart bus once it has been initialized.
* use this if you want to do your own reading and writing to the bus instead
* of the basic functions defined here. If the bus has not been initialized, 
* return -1;
*******************************************************************************/
int get_uart_fd(int bus){
	// sanity checks
	if(bus<MIN_BUS || bus>MAX_BUS){
		printf("ERROR: uart bus must be between %d & %d\n", MIN_BUS, MAX_BUS);
		return -1;
	}
	
	if (initialized[bus]==0){
		printf("ERROR: uart%d not initialized yet\n", bus);
		return -1;
	}
	
	return fd[bus];
}

/*******************************************************************************
* int flush_uart(int bus)
*
* flushes (discards) any data received but not read.
*******************************************************************************/
int flush_uart(int bus){
	// sanity checks
	if(bus<MIN_BUS || bus>MAX_BUS){
		printf("ERROR: uart bus must be between %d & %d\n", MIN_BUS, MAX_BUS);
		return -1;
	}
	if(initialized[bus]==0){
		printf("ERROR: uart%d must be initialized first\n", bus);
		return -1;
	}
	return tcflush(fd[bus],TCIFLUSH);
}

/*******************************************************************************
*	int uart_send_bytes(int bus, int bytes, char* data);
*
* This is essentially a wrapper for the linux write() function with some sanity
* checks. Returns -1 on error, otherwise returns number of bytes sent.
*******************************************************************************/
int uart_send_bytes(int bus, int bytes, char* data){
	// sanity checks
	if(bus<MIN_BUS || bus>MAX_BUS){
		printf("ERROR: uart bus must be between %d & %d\n", MIN_BUS, MAX_BUS);
		return -1;
	}
	if(bytes<1){
		printf("ERROR: number of bytes to send must be >1\n");
		return -1;
	}
	if(initialized[bus]==0){
		printf("ERROR: uart%d must be initialized first\n", bus);
		return -1;
	}
	
	return write(fd[bus], data, bytes);
}

/*******************************************************************************
* int uart_send_byte(int bus, char data);
*
* This is essentially a wrapper for the linux write() function with some sanity
* checks. Returns -1 on error, otherwise returns number of bytes sent.
*******************************************************************************/
int uart_send_byte(int bus, char data){
	
	// sanity checks
	if(bus<MIN_BUS || bus>MAX_BUS){
		printf("ERROR: uart bus must be between %d & %d\n", MIN_BUS, MAX_BUS);
		return -1;
	}
	
	if(initialized[bus]==0){
		printf("ERROR: uart%d must be initialized first\n", bus);
		return -1;
	}
	
	return write(fd[bus], &data, 1);
}
		

/*******************************************************************************
* int uart_read_bytes(int bus, int bytes, char* buf, int timeout_ms)
*
* This is a blocking function call. It will only return once the desired number
* of bytes has been read from the buffer or if the global flow state defined
* in robotics_cape.h is set to EXITING.
*******************************************************************************/
int uart_read_bytes(int bus, int bytes, char* buf, int timeout_ms){
	int ret; // holder for return values
	fd_set set; // for select()
	struct timeval timeout;
	int bytes_read; // number of bytes read so far
	int bytes_left; // number of bytes still need to be read
	
	// sanity checks
	if(bus<MIN_BUS || bus>MAX_BUS){
		printf("ERROR: uart bus must be between %d & %d\n", MIN_BUS, MAX_BUS);
		return -1;
	}
	if(bytes<1){
		printf("ERROR: number of bytes to send must be >1\n");
		return -1;
	}
	if(initialized[bus]==0){
		printf("ERROR: uart%d must be initialized first\n", bus);
		return -1;
	}
	
	bytes_read = 0;
	bytes_left = bytes;
	
	// set up the timeout OUTSIDE of the read loop. We will likely be calling
	// select() multiple times and that will decrease the timeout struct each
	// time ensuring the TOTAL timeout requested by the user is honoured instead
	// of the timeout value compounding each loop.
	timeout.tv_sec = timeout_ms/1000;
	timeout.tv_usec = (timeout_ms % 1000) * 1000;
	
	// exit the read loop once enough bytes have been read
	// or the global flow state becomes EXITING. This prevents programs
	// getting stuck here and not exiting properly
	while((bytes_left>0)&&get_state()!=EXITING){
		FD_ZERO(&set); /* clear the set */
		FD_SET(fd[bus], &set); /* add our file descriptor to the set */
		ret = select(fd[bus] + 1, &set, NULL, NULL, &timeout);
		if(ret == -1){
			// select returned and error. EINTR means interrupted by SIGINT
			// aka ctrl-c. Don't print anything as this happens normally
			// in case of EINTR/Ctrl-C just return how many bytes got read up 
			// until then without raising alarms.
			if(errno!=EINTR){
				printf("uart select() error: %s\n", strerror(errno));
				return -1;
			}
			return bytes_read;  
		}
		else if(ret == 0){
			// timeout
			return bytes_read;
		}
		else{
			// There was data to read. Read up to the number of bytes left
			// and no more. This most likely will return fewer bytes than
			// bytes_left, but we will loop back to get the rest.
			ret=read(fd[bus], buf+bytes_read, bytes_left);
			if(ret<0){
				printf("ERROR: uart read() returned %d\n", ret);
				return -1;
			}
			else if(ret>0){
				// success, actually read something
				bytes_read += ret;
				bytes_left -= ret;
			}
		}
	}
	return bytes_read;
}
