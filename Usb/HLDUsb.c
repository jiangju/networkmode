/*
 * HLDUsb.c
 *
 *  Created on: 2016��9��18��
 *      Author: j
 */
#include "HLDUsb.h"
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include "log3762.h"
#include <unistd.h>
#include "CommLib.h"
#include "HLDWatchDog.h"
#include "SysPara.h"
#include <string.h>
#include <stdlib.h>

/*
 * ��������:�����ļ����Ϸ����ж�
 * */
int update_name_inspect(char *str)
{
	if(0 != strncmp(str, NET_MODE_V, LL_LL))
		return -1;
	if(*(str + LL_LL) > '9' || *(str + LL_LL) < '0')
		return -1;
	if(*(str + LL_LL + 1) != '.')
		return-1;
	if(*(str + LL_LL + 2) > '9' || *(str + LL_LL + 2) < '0')
		return -1;
	if(*(str + LL_LL + 3) != '.')
		return-1;
	if(*(str + LL_LL + 4) > '9' || *(str + LL_LL + 4) < '0')
		return -1;
	if(*(str + LL_LL + 5) > '9' || *(str + LL_LL + 5) < '0')
		return -1;
	return 0;
}

/*
 * ��������:�����ļ�CRC�Ϸ����ж�
 * */
int update_crc_inspect(char *str)
{
	if(*(str) < '0' || (*(str) > '9' && (*str) < 'A') || *(str) > 'F')
		return -1;
	if(*(str + 1) < '0' || (*(str + 1) > '9' && *(str + 1) < 'A') || *(str + 1) > 'F')
		return -1;
	if(*(str + 2) < '0' || (*(str + 2) > '9' && *(str + 2) < 'A') || *(str + 2) > 'F')
		return -1;
	if(*(str + 3) < '0' || (*(str + 3) > '9' && *(str + 3) < 'A') || *(str + 3) > 'F')
		return -1;
	return 0;
}

/*
 * ��������:��ȡ�����ļ�����CRC
 * ����:	name	�ļ���
 * 		crc		crc
 * ����ֵ 0 �ɹ� -1 ʧ��
 * */
int get_updatefile_nameandcrc(char *dname, char *name, unsigned short *crc)
{
	FILE *fp = NULL;
	//�����������ļ�
	int i = 3;
	int j = 0;
	while(i--)
	{
		fp = fopen(dname, "r");
		if(NULL == fp)
		{
			continue;
		}
		i--;
		break;
	}

	if(NULL == fp)
		return -1;
	//��ȡ�ļ�����
	char buf[100] = {0};
	fread(buf, 1, 100, fp);
	char *str1 = "name:";
	char *str2 = "crc:";
	//��������ļ���
	i = 0;
	while(i < (100 - 5))
	{
		if(0 == strncmp(str1, (buf + i), 5))
		{
			break;
		}
		i++;
	}

	if(95 == i)
		return -1;
	j = i;
	j += 5;
	if((j + LL_LL + V_LL) > 100)
		return -1;
	if(buf[j + LL_LL + V_LL] != ';')
		return -1;
	if(0 != update_name_inspect(buf + j))
		return -1;
	memcpy(name, buf + j, (LL_LL + V_LL));
	i = j + LL_LL + V_LL;

	while(i < (100 - 4))
	{
		if(0 == strncmp(str2, (buf + i), 4))
		{
			break;
		}
		i++;
	}
	if(96 == i)
		return -1;
	j = i;
	j += 4;
	if((j + 4) > 100)
		return -1;
	if(buf[j + 4] != ';')
		return -1;
	if(0 != update_crc_inspect(buf + j))
		return -1;

	*crc = 0;

	*crc = ((*crc) * 16) + chars_to_char(buf[j]);
	*crc = ((*crc) * 16) + chars_to_char(buf[j+1]);
	*crc = ((*crc) * 16) + chars_to_char(buf[j+2]);
	*crc = ((*crc) * 16) + chars_to_char(buf[j+3]);

	return 0;
}

/*
 * ��������:���³���������ļ�
 * */
int update_config_file(char *fname)
{
	FILE *fp = NULL;
	char str[100] = {0};
	fp = fopen("/opt/act_config1", "w");
	if(NULL == fp)
	{
		return -1;
	}
	//д���ļ���
	sprintf(str, "name:%s;\n",fname);
	int ret = fwrite(str, 1, strlen(str),fp);
	if(ret != strlen(str))
		return -1;

	//д��csУ��
	memset(str, 0, 100);
	unsigned char cs = Func_CS(fname, strlen(fname));
	sprintf(str,"cs:%02x;",cs);
	ret = fwrite(str, 1, strlen(str),fp);
	if(ret != strlen(str))
		return -1;

	fclose(fp);
	return 0;
}

/*
 * ��������:���U���Ƿ����
 * ����ֵ: 0 ���� -1 δ����
 * */
int is_inserted_usb(void)
{
	if(0 == access("/proc/scsi/usb-storage", F_OK))
		return 0;
	else
		return -1;
}

/*
 * ��������:��ȡU�̹��ص��ļ���
 * ����: dname	�ļ���
 * ����ֵ: 0 �ɹ� -1 ʧ��
 * */
int get_usb_dname(char *dname)
{
	DIR *dp = NULL;
	dp = opendir("/media");
	if(NULL == dp)
	{
		perror("open dir :");
		return -1;
	}
	struct dirent *d = NULL;
	while(1)
	{
		usleep(1);
		d = readdir(dp);
		if(NULL == d)
			break;
		if(d->d_name[0] == 's' && d->d_name[1] == 'd')
			break;
	}

	if(d == NULL)
	{
		printf("readdir err or read end\n");
		return -1;
	}
	strcpy(dname, d->d_name);
	return 0;
}

/*
 * ��������:usb�߳�
 * */
void *pthread_usb(void *arg)
{
	//���뿴�Ź�
	int wdt_id = *(int *)arg;
	//
	feed_watch_dog(wdt_id);	//ι��

	char fname[20] = {0};		//�ļ���
	unsigned short fcrc1 = 0;	//CRCУ����
	unsigned short fcrc2 = 0;	//CRCУ����
	char str1[100] = {0};		//���������ļ�·��
	char str2[100] = {0};		//U�������ļ�·��
	char str3[100] = {0};		//U�������ļ�����·��

	char dname[10] = {0};		//U���ļ���

	char dir1[100] = {0};		//U�̴��log376.2���ļ���
	char dir2[100] = {0};		//U�̴�������ļ����ļ���

	while(1)
	{
		sleep(1);	//����1��
		feed_watch_dog(wdt_id);	//ι��

		//�ж�U���Ƿ����
		if(0 == is_inserted_usb())
		{
			sleep(5);
			feed_watch_dog(wdt_id);

			//��ȡU���ļ���
			if(0 == get_usb_dname(dname))
			{
				sprintf(dir1, "/media/%s/log",dname);
				sprintf(dir2, "/media/%s/update",dname);
				if(0 == access(dir1, F_OK))
				{
					//��ͣ376.2���ļ�¼
					feed_watch_dog(wdt_id);
					log_3762_task_stop(&_log_3762_task);
					memset(dir1, 0, 100);
					sprintf(dir1, "/media/%s/log/log1_1.txt",dname);
					HLDFileCopy(LOG_3762_FILE0, dir1);
					memset(dir1, 0, 100);
					sprintf(dir1, "/media/%s/log/log1_2.txt",dname);
					HLDFileCopy(LOG_3762_FILE1, dir1);
					memset(dir1, 0, 100);
					sprintf(dir1, "/media/%s/log/log1_3.txt",dname);
					HLDFileCopy(LOG_3762_FILE2, dir1);
					memset(dir1, 0, 100);
					sprintf(dir1, "/media/%s/log/log1_4.txt",dname);
					HLDFileCopy(LOG_3762_FILE3, dir1);
					feed_watch_dog(wdt_id);
					log_3762_task_start(&_log_3762_task);
				}

				if(0 == access(dir2, F_OK))
				{
					//��ȡ�����ļ�����·��
					sprintf(str3, "%s/config.txt",dir2);
					if(0 == get_updatefile_nameandcrc(str3, fname, &fcrc1))
					{
						sprintf(str1, "/opt/%s",fname);
						sprintf(str2,"%s/%s",dir2,fname);
						if(0 == HLDFileCopy(str2, str1))
						{
							if(0 == get_file_crc(str1, &fcrc2))
							{
								if(fcrc2 == fcrc1)
								{
									update_config_file(fname);
									//�ı������ļ�Ȩ��
									memset(str1, 0, 100);
									sprintf(str1, "chmod +x /opt/%s", fname);
									system(str1);
									while(1);	//�����ɹ�  ����ϵͳ
								}
							}
						}
					}
					else
					{
						printf("get update file name and crc erro\n");
					}
				}
			}

			while(0 == is_inserted_usb())	//�ȴ�U�̰γ�
			{
				sleep(1);
				feed_watch_dog(wdt_id);	//ι��
			}
		}
	}

	pthread_exit(NULL);
}
