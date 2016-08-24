/*
 * Usart.c
 *
 *  Created on: 2016��6��24��
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
 * ������:���������Ҫ�Ĳ�����
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
 *��������:���ô���
 *����:	num	���ں�
 *		attribute	��������
 *����ֵ:0 �ɹ� 1 ʧ��
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

	//�޸Ŀ���ģʽ����֤���򲻻�ռ�ô��ڣ�
	new_opt.c_cflag |= CLOCAL;

	//�޸Ŀ���ģʽ��ʹ���ܹ��Ӵ��ڶ�ȡ��������
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
			new_opt.c_cflag |= (PARODD | PARENB);	/* ����Ϊ��Ч��*/
			new_opt.c_iflag |= INPCK;				/* Disable parity checking */
			break;

		case 2:
			new_opt.c_cflag |= PARENB;		/* Enable parity */
			new_opt.c_cflag &= ~PARODD;		/* ת��ΪżЧ��*/
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

	//�޸����ģʽ��ԭʼ�������(raw ģʽ)
	new_opt.c_lflag &= ~(ICANON | ECHO | ISIG);				/*Input*/
	new_opt.c_oflag &= ~OPOST;								/*Output*/

	//�޸Ŀ����ַ�����ȡ�ַ������ٸ���Ϊ1 ������
	new_opt.c_cc[VMIN]=1;

	//�޸Ŀ����ַ�����ȡ��һ���ַ��ĳ�ʱʱ��Ϊ1��100ms
	new_opt.c_cc[VTIME]=1;

	//�����������������������ݣ����ǲ��ٶ�ȡ
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
 * ��������:���ڷ���
 * ����:	fd		�����豸�ļ�������
 * 		outbuff	�跢�͵�����
 * 		outlen	���ͳ���
 * ����ֵ: ��
 * */
void UsartSend(int fd, unsigned char *outbuff, unsigned short outlen)
{
//	int i = 0;
//	for(i = 0; i < outlen; i++)
//		printf(" %02x",outbuff[i] );
//	printf("\n");
	write(fd, outbuff, outlen);
}
