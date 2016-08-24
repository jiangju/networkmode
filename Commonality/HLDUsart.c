/*
 * Usart.c
 *
 *  Created on: 2016年6月24日
 *      Author: j
 */
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <HLDUsart.h>
#include <termios.h>
#include <limits.h>

/*
 * 函数功:获得设置需要的波特率
 * */
int get_baud_rate(unsigned long int baud_rate)
{
	switch (baud_rate)
	{
		case 0:
			return B0;
		case 50:
			return B50;
		case 75:
			return B75;
		case 110:
			return B110;
		case 134:
			return B134;
		case 150:
			return B150;
		case 200:
			return B200;
		case 300:
			return B300;
		case 600:
			return B600;
		case 1200:
			return B1200;
		case 1800:
			return B1800;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		case 230400:
			return B230400;

		default:
			return -1;
	}
}


/*
 *函数功能:设置串口
 *参数:	num	串口号
 *		attribute	串口属性
 *返回值:0 成功 1 失败
 **/
char SetUsart(unsigned char num, UsartAttribute *attribute)
{
	int fd;
	int baud_rate = 0;
	char status = 0;
	struct termios new_opt;
	char path[30] = {0};
	sprintf(path, "/dev/ttySP%d", num);
	fd = open(path, O_RDWR | O_NOCTTY);
	if(fd < 0)
	{
		printf("set usart open dev erro\n");
		return -1;
	}

	//get the current config -> new_opt
	tcgetattr(fd,&new_opt);

	bzero( &new_opt, sizeof(new_opt));

	//convert baud rate -> baud_flag
	baud_rate=get_baud_rate(attribute->BaudRate);

	tcflush(fd, TCIOFLUSH);
	//setup input/output baudrate
	cfsetispeed(&new_opt,baud_rate);

	cfsetospeed(&new_opt,baud_rate);

	status = tcsetattr(fd, TCSANOW, &new_opt);
	if (status != 0)
	{
		perror("tcsetattr::set baud rate failed\n");
		close(fd);
		return -1;
	}

	//修改控制模式，保证程序不会占用串口？
	new_opt.c_cflag |= CLOCAL;

	//修改控制模式，使得能够从串口读取输入数据
	new_opt.c_cflag |= CREAD;

	new_opt.c_cflag |= HUPCL;

	//setup control flow
	switch(attribute->FlowCtl)
	{
		case '0':
			//no control-flow
			new_opt.c_cflag &=~CRTSCTS;
			break;
		case '1':
			//hardware control-flow
			new_opt.c_cflag |=CRTSCTS;
			break;
		case '2':
			new_opt.c_iflag |= IXON | IXOFF | IXANY;
			break;
	}

	new_opt.c_cflag &=~CSIZE;
	switch(attribute->DataLen)
	{
		case '5':
			new_opt.c_cflag |=CS5;
			break;
		case '6':
			new_opt.c_cflag |=CS6;
			break;
		case '7':
			new_opt.c_cflag |=CS7;

			break;
		case '8':
			new_opt.c_cflag |=CS8;
			break;
		default:
			new_opt.c_cflag |=CS8;
	}

	//setup parity
	switch(attribute->isParity)
	{
		case 0:
			new_opt.c_cflag &= ~PARENB;   /* Clear parity enable */
			new_opt.c_iflag &= ~INPCK;     /* Enable parity checking */
			break;

		case 1:
			new_opt.c_cflag |= (PARODD | PARENB);	/* 设置为奇效验*/
			new_opt.c_iflag |= INPCK;				/* Disable parity checking */
			break;

		case 2:
			new_opt.c_cflag |= PARENB;		/* Enable parity */
			new_opt.c_cflag &= ~PARODD;		/* 转换为偶效验*/
			new_opt.c_iflag |= INPCK;       /* Disable parity checking */
			break;

		case 3:  /*as no parity*/
			new_opt.c_cflag &= ~PARENB;
			new_opt.c_cflag &= ~CSTOPB;
			break;

		default:
			fprintf(stderr,"Unsupported parity\n");
			close(fd);
			return -1;
	}

	//setup stop-bit
	if(attribute->isStop==2)
	{
		new_opt.c_cflag |=CSTOPB;
	}
	else
	{
		new_opt.c_cflag &=~CSTOPB;
	}

	/* Set input parity option */
	if (attribute->isParity != 0)
	{
		new_opt.c_iflag |= INPCK;
	}

	//修改输出模式：原始数据输出(raw 模式)
	new_opt.c_lflag &= ~(ICANON | ECHO | ISIG);				/*Input*/
	new_opt.c_oflag &= ~OPOST;								/*Output*/

	//修改控制字符：读取字符的最少个数为1 ？？？
	new_opt.c_cc[VMIN]=1;

	//修改控制字符：读取第一个字符的超时时间为1×100ms
	new_opt.c_cc[VTIME]=1;

	//如果发生数据溢出，接收数据，但是不再读取
	tcflush(fd,TCIFLUSH);

	status = tcsetattr(fd,TCSANOW,&new_opt);
	if(status != 0)
	{
		perror("Cannot set the serial port parameters");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

/*
 * 函数功能:串口发送
 * 参数:	fd		串口设备文件描述符
 * 		outbuff	需发送的内容
 * 		outlen	发送长度
 * 返回值: 无
 * */
void UsartSend(int fd, unsigned char *outbuff, unsigned short outlen)
{
//	int i = 0;
//	for(i = 0; i < outlen; i++)
//		printf(" %02x",outbuff[i] );
//	printf("\n");
	write(fd, outbuff, outlen);
}
