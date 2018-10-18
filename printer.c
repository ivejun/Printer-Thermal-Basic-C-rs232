#include <termios.h>                                                         
#include <stdio.h>
#include <stdlib.h>	
#include <string.h>
#include <unistd.h>                                                          
#include <fcntl.h>                                                                                                               
#include <sys/types.h> 
#include <stdint.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>
#include <stropts.h>
#include <poll.h>	
#include <wiringPi.h>
#include <errno.h>
#include <ctype.h>

#define BAUDRATE B19200                                                      
#define MODEMDEVICE "/dev/ttyS0"
#define left 1
#define center 2
#define right 3

int fd;                                                             
struct pollfd fds[1];

int ret, res;
int flag = 0;
int qrflag = 0;

////////////////////////////barcode Setting
//Enable Bold mode

//Printing data Directly CCCCCCCCCuy
void printdata(char * text){
	int len = strlen(text);
	write(fd, text, len);
	}

//Bold Setting
void bold(int state){
	char _offBold[] = {0x1B, 0x45, 0x00};
	char _onBold[] = {0x1B, 0x45, 0x01};

	switch(state){
		case 0:
		write(fd, _offBold, 3);
		break;
		case 1:
		write(fd, _onBold, 3);
		break;
		default:
		printf("Bold Hanya 1 dan 0(ON dan OFF)");
		break;
	}
}
//Enable Underline mode
void underline(int state){
	char _offU[] = {0x1B, 0x2D, 0x00};
	char _onU[] = {0x1B, 0x2D, 0x01};
	char _onU2[] = {0x1B, 0x2D, 0x02};

	switch(state){
		case 0:
		write(fd, _offU, 3);
		break;
		case 1:
		write(fd, _onU, 3);
		case 2:
		write(fd, _onU2, 3);
		break;
		default:
		printf("Underline Hanya 1 dan 0(ON dan OFF)");
		break;
	}
}
//Text Align Setting
void justify(int state){
	char _left[] = {0x1B, 0x61, 0};
	char _center[] = {0x1B, 0x61, 1};
	char _right[] = {0x1B, 0x61, 2};

	switch(state){
		case 1:
		write(fd, _left, 3);
		break;
		case 2:
		write(fd, _center, 3);
		break;
		case 3:
		write(fd, _right, 3);
		break;
		default:
		printf("Pilih 1 sampai 3 aja\n 1: Left\n 2: Center\n 3: Right");
		break;
	}
}
//size of Font Heigh
void doubleheight(int state){
	char _offHeight[] = {0x1B, 0x21, 0};
	char _onHeight[] = {0x1B, 0x21, 16};

	switch(state){
		case 0:
		write(fd, _offHeight, 3);
		break;
		case 1:
		write(fd, _onHeight, 3);
		break;
		default:
		printf("Height Hanya 1 dan 0(ON dan OFF)");
		break;
	}
}


void reversetext(int state){
	char _offreverse[] = {0x1D, 0x42, 0x00};
	char _onreverse[] = {0x1D, 0x42, 0x01};

	switch(state){
		case 0:
		write(fd, _offreverse, 3);
		break;
		case 1:
		write(fd, _onreverse, 3);
		break;
		default:
		printf("Height Hanya 1 dan 0(ON dan OFF)");
		break;
	}
}

//Rotate text to 90 Degree Kayak Tongkat
void rotatetext(int state){
	char _offrotate[] = {0x1B, 0x56, 0x00};
	char _onrotate[] = {0x1B, 0x56, 0x01};

	switch(state){
		case 0:
		write(fd, _offrotate, 3);
		break;
		case 1:
		write(fd, _onrotate, 3);
		break;
		default:
		printf("Height Hanya 1 dan 0(ON dan OFF)");
		break;
	}
}
//Cut The Page
void cut(){
	char _cut[] = {0x1B, 0x69};
	write(fd, _cut, 2);
	}

//Dipanggil Untuk Membersihkan buffer, Ketika Kita Mau print Bitmap, Barcode, QrCode. DLL.
void feed(){
	char feed[] = {0x0A};
	write(fd, feed, 1);
	}
	
//adjust Size Of Text
void sizetexts(int panjang, int lebar){
	char sizes[3] = {0x1D,0x21,0x00};

	if     (panjang == 1 && lebar == 1) sizes[2] = 0x00;
	else if(panjang == 1 && lebar == 2) sizes[2] = 0x01;
	else if(panjang == 2 && lebar == 2) sizes[2] = 0x11;
	else if(panjang == 2 && lebar == 1) sizes[2] = 0x10;
	else if(panjang == 1 && lebar == 3) sizes[2] = 0x02;
	else if(panjang == 2 && lebar == 3) sizes[2] = 0x21;
	else if(panjang == 3 && lebar == 3) sizes[2] = 0x22;
	else if(panjang == 3 && lebar == 2) sizes[2] = 0x12;
	else if(panjang == 3 && lebar == 1) sizes[2] = 0x20;
	
	else {
		printf("Wrong Value Size, Auto Set To 1");
		sizes[2] = 0x00;
		}

	write(fd, sizes, 3);
}

void reset(){
		char res[3] = {0x1B,0x40};
		write(fd, res, 3);
	}

////////////////////////////End Of Barcode Setting





int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf("error %d setting term attributes", errno);
}

void openprinter(){
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd == 0)
	{
		perror(MODEMDEVICE);
		printf("Failed to open MODEMDEVICE \"/dev/ttyAMA0\"\n");
		exit(-1); 
	}

	set_interface_attribs (fd, BAUDRATE, 0);  // set speed to 19200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	/* Open STREAMS device. */
	fds[0].fd = fd;
	fds[0].events = POLLRDNORM;
}

void bacadata(){
		char buf[255];  
		ret = poll(fds, 1, 1000);		//wait for response
		
		if (ret > 0)
		{
			/* An event on one of the fds has occurred. */
			if (fds[0].revents & POLLHUP)
			{
				printf("Hangup\n");
			}
			if (fds[0].revents & POLLRDNORM)
			{
					res = read(fd,buf,255);
					buf[res]=0;
					printf("Received %s\n",buf);
			}
		}
	}

void printbarcode(char *text){
		feed();
		//variable Pengaturan Barcode
		char barcode1[100] = {0x1D, 0x6B, 0x04, 0x30, 0x30, 0x30, 0x30, 0x00};
		char barcodeHeight[] = { 0x1D, 0x68, 50};
		char barcodeWidth[] = { 0x1D, 0x77, 3};
		char barcodeNumber[] = { 0x1D, 0x48, 2};
		
		//Check Status Data yang masuk		
		printf("Received %s\n",text);
		int len = strlen(text);
		printf("len %i\n",len);
		
		int i = 0;
		//Proses Convert Digit yang masuk
		while(i < len){
			//Point by reference
			barcode1[i+3] = text[i];
			int logic = 0;
			//Check apakah digit yang masuk ada Huruf Kecil, 
			logic = text[i] - '0';
			for(int finder = 49; finder<= 74; finder ++){
					if(logic == (finder))barcode1[i+3] = toupper(text[i]);//pass by reference huruf kecil -> huruf besar
					}
			i++;
			}
			//menambah akhiran untuk print barcode
			barcode1[len+4] = 0x00;
			//Setting Barcode Di Awal hanya di eksekusi 1X, tidak bolek lebih. akan mengakibatkan error jika lebih dari 1x
			if(!flag){
				res = write(fd, barcodeHeight, (3));
				res = write(fd, barcodeWidth, (3));
				res = write(fd, barcodeNumber, (3));
				flag = 1;
			}
			//Print Barcode Data
			res = write(fd, barcode1, (4+len));
}

void printqrcode(char *text, char version,char size){
		if(version <= 0 || version > 40){printf("Range Value Error");return;}
		if(size <= 0 || size > 40){printf("Range Value Error");return;}
		feed();
		//variable Pengaturan QR-CODE
		const int _Length = 5;
		int len = strlen(text);
		int i = 0;
		
		//QR setting Module
		char qr1command[] = { 0x13, 0x50, 0x48, 0x01, version};
		char qr2command[] = { 0x13, 0x50, 0x48, 0x02, 1};
		char qr3command[] = { 0x13, 0x50, 0x48, 0x03, size};
		char qr4command[] = { 0x13, 0x50, 0x48, 0x04, 2};
		char qr5command[] = { 0x00, 0x13, 0x50, 0x48, 0x05};
		char data[100];
		
		//Data Inside
		while(i != len){
			data[i] = text[i];
			i++;
		}
		
		//Writing State
		res = write(fd, qr1command, _Length);
		res = write(fd, qr2command, _Length);
		res = write(fd, qr3command, _Length);
		res = write(fd, qr4command, _Length);
		res = write(fd, data, len);
		res = write(fd, qr5command, _Length);
}


int main(void)
{
	int Counter = 1;
	char * counter = "11";

	openprinter();
	justify(center);
	for (;;)		// forever
	{
		justify(center);
		printdata("Selamat Datang Di PT Delameta Bilano");
		printdata("No. Telepon 000-00000");
		printdata("\n");
		justify(left);
		
		bold(1);
		printdata("Antrian Anda Adalah:");
		printdata(counter);
		bold(0);
		
		underline(1);
		justify(center);
		printdata("\n");
		printdata("Test Page");
		printdata("\n");
		underline(0);	
		
		printbarcode("Vincent");
		
		printdata("\n");
		printqrcode("Jajaja",1,8);
	
		printdata("\n");
		sizetexts(3, 3);
		printdata("OH MY GODDDDDD");
		printdata("\n");
		
		//cut();
		Counter +=1;
		reset();
		delay(100000);
	}
}

/*
 * 		
	
*/
