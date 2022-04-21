#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdarg.h>

char *device = "/dev/ttyUSB0";
int b_rate = 115200;

void error(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	fprintf(stderr, "Error(%d)\n", errno);
	vfprintf(stderr, msg, args);
	fprintf(stderr, "%s\nAborting\n", strerror(errno));
	exit(-1);
}

int ttywrite(int fd, const char *msg)
{
	return write(fd, msg, strlen(msg));
}

int ttyread(int fd, char *buff, size_t size)
{
	int read_bytes = 0;

	memset(buff, 0, size);

	// size-1 because last character needs to be '\0'
	for(int i = 0; i < size-1; i++){

		int n = read(fd, &buff[i], 1);

		if (n < 0)
			return n;

		read_bytes += n;

		if(buff[i] == '\r')
			break;
	}
	return read_bytes;
}

int main()
{
	/* open device */
	int fd = open(device, O_RDWR | O_NOCTTY);

	if(fd < 0)
		error("Failed to open %s: ", device);

	/* configure serial port */
	struct termios tty;
	memset(&tty, 0, sizeof(tty));

	if(tcgetattr(fd, &tty))
		error("Tcgetattr: ");

	/* set baud rate */
	cfsetspeed(&tty, b_rate);

	/* Setting other Port Stuff */
	tty.c_cflag     &=  ~PARENB;            // Make 8n1
	tty.c_cflag     &=  ~CSTOPB;
	tty.c_cflag     &=  ~CSIZE;
	tty.c_cflag     |=  CS8;

	tty.c_cflag     &=  ~CRTSCTS;           // no flow control
	tty.c_cc[VMIN]   =  1;                  // read doesn't block
	tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
	tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	/* Make raw */
	cfmakeraw(&tty);

	tcflush(fd, TCIFLUSH);
	if ( tcsetattr (fd, TCSANOW, &tty ) != 0)
		error("Tcsetattr: ");

	char buffer[1024];

	while(1){
		ttyread(fd, buffer, sizeof(buffer));
		printf("%s", buffer);
		ttywrite(fd, "Hello!!!\n\r");
	}


	return 0;
}
