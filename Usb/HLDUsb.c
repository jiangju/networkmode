/*
 * HLDUsb.c
 *
 *  Created on: 2016年9月18日
 *      Author: j
 */
#include "HLDUsb.h"
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

static struct hld_usb_s _usb_status;

/*
 * 函数功能:初始化usb状态
 * */
void init_usb_status()
{
	_usb_status.status.s_in = 0;
	_usb_status.status.s_log = 0;
	_usb_status.status.s_update = 0;
	pthread_mutex_init(&_usb_status.mutex, NULL);
}

/*
 * 函数功能:设置usb状态
 * 参数:		in		状态
 * 返回值;
 * */
void set_usb_status(struct usb_status in)
{
	pthread_mutex_lock(&_usb_status.mutex);

	memcpy(&_usb_status.status, &in, sizeof(struct usb_status));

	pthread_mutex_unlock(&_usb_status.mutex);
}

/*
 * 函数功能:获得USB状态
 * 参数:		out		输出状态
 * 返回值:
 * */
void get_usb_status(struct usb_status *out)
{
	memset(out, 0, sizeof(struct usb_status));
	pthread_mutex_lock(&_usb_status.mutex);
	memcpy(&out, &_usb_status.status, sizeof(struct usb_status));
	pthread_mutex_unlock(&_usb_status.mutex);
}

/*
 * 函数功能:升级文件名合法性判断
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
 * 函数功能:升级文件CRC合法性判断
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
 * 函数功能:获取升级文件名及CRC
 * 参数:	name	文件名
 * 		crc		crc
 * 返回值 0 成功 -1 失败
 * */
int get_updatefile_nameandcrc(char *dname, char *name, unsigned short *crc)
{
	FILE *fp = NULL;
	//打开升级配置文件
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
	//读取文件内容
	char buf[100] = {0};
	fread(buf, 1, 100, fp);
	char *str1 = "name:";
	char *str2 = "crc:";
	//解析获得文件名
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
	{
		fclose(fp);
		return -1;
	}
	j = i;
	j += 5;
	if((j + LL_LL + V_LL) > 100)
	{
		fclose(fp);
		return -1;
	}
	if(buf[j + LL_LL + V_LL] != ';')
	{
		fclose(fp);
		return -1;
	}
	if(0 != update_name_inspect(buf + j))
	{
		fclose(fp);
		return -1;
	}
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
	{
		fclose(fp);
		return -1;
	}
	j = i;
	j += 4;
	if((j + 4) > 100)
	{
		fclose(fp);
		return -1;
	}
	if(buf[j + 4] != ';')
	{
		fclose(fp);
		return -1;
	}
	if(0 != update_crc_inspect(buf + j))
	{
		fclose(fp);
		return -1;
	}

	*crc = 0;

	*crc = ((*crc) * 16) + chars_to_char(buf[j]);
	*crc = ((*crc) * 16) + chars_to_char(buf[j+1]);
	*crc = ((*crc) * 16) + chars_to_char(buf[j+2]);
	*crc = ((*crc) * 16) + chars_to_char(buf[j+3]);

	fclose(fp);
	return 0;
}

/*
 * 函数功能:更新程序的配置文件
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
	//写入文件名
	sprintf(str, "name:%s;\n",fname);
	int ret = fwrite(str, 1, strlen(str),fp);
	if(ret != strlen(str))
	{
		fclose(fp);
		return -1;
	}

	//写入cs校验
	memset(str, 0, 100);
	unsigned char cs = Func_CS(fname, strlen(fname));
	sprintf(str,"cs:%02x;",cs);
	ret = fwrite(str, 1, strlen(str),fp);
	if(ret != strlen(str))
	{
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

/*
 * 函数功能:检测U盘是否插入
 * 返回值: 0 插入 -1 未插入
 * */
int is_inserted_usb(void)
{
	if(0 == access("/proc/scsi/usb-storage", F_OK))
		return 0;
	else
		return -1;
}

/*
 * 函数功能:获取U盘挂载的文件名
 * 参数: dname	文件名
 * 返回值: 0 成功 -1 失败
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
		closedir(dp);
		return -1;
	}
	strcpy(dname, d->d_name);
	closedir(dp);
	return 0;
}

/*
 * 函数功能:usb线程
 * */
void *pthread_usb(void *arg)
{
	int usb_flag = 0;	//usb 插入标志
	struct usb_status temp_status;
	//申请看门狗
	int wdt_id = *(int *)arg;
	//
	feed_watch_dog(wdt_id);	//喂狗
	printf("USB _WDT ID %d\n", wdt_id);

	char fname[20] = {0};		//文件名
	unsigned short fcrc1 = 0;	//CRC校验码
	unsigned short fcrc2 = 0;	//CRC校验码
	char str1[100] = {0};		//本地升级文件路径
	char str2[100] = {0};		//U盘升级文件路径
	char str3[100] = {0};		//U盘升级文件配置路径

	char dname[10] = {0};		//U盘文件名

	char dir1[100] = {0};		//U盘存放log376.2的文件夹
	char dir2[100] = {0};		//U盘存放升级文件的文件夹

	while(1)
	{
		init_usb_status();
		sleep(1);	//休眠1秒
		feed_watch_dog(wdt_id);	//喂狗

		//判断U盘是否插入
		if(0 == is_inserted_usb())
		{
			get_usb_status(&temp_status);
			temp_status.s_in = 1;
			set_usb_status(temp_status);

			sleep(5);
			feed_watch_dog(wdt_id);

			//获取U盘文件名
			if(0 == get_usb_dname(dname))
			{
				usb_flag = 1;
				sprintf(dir1, "/media/%s/log",dname);
				sprintf(dir2, "/media/%s/update",dname);
				if(0 == access(dir1, F_OK))
				{
					get_usb_status(&temp_status);
					temp_status.s_log = 1;
					set_usb_status(temp_status);

					//暂停376.2报文记录
					feed_watch_dog(wdt_id);
					log_3762_task_stop(&_log_3762_task);
					memset(dir1, 0, 100);
					sprintf(dir1, "/media/%s/log/log1_1.txt",dname);
					feed_watch_dog(wdt_id);
					if(0 != HLDFileCopy(LOG_3762_FILE0, dir1))
					{
						get_usb_status(&temp_status);
						temp_status.s_log |= 0x10;
						set_usb_status(temp_status);
					}

					memset(dir1, 0, 100);
					sprintf(dir1, "/media/%s/log/log1_2.txt",dname);
					feed_watch_dog(wdt_id);
					if(0 != HLDFileCopy(LOG_3762_FILE1, dir1))
					{
						get_usb_status(&temp_status);
						temp_status.s_log |= 0x20;
						set_usb_status(temp_status);
					}

					memset(dir1, 0, 100);
					sprintf(dir1, "/media/%s/log/log1_3.txt",dname);
					feed_watch_dog(wdt_id);
					if(0 != HLDFileCopy(LOG_3762_FILE2, dir1))
					{
						get_usb_status(&temp_status);
						temp_status.s_log |= 0x40;
						set_usb_status(temp_status);
					}

					memset(dir1, 0, 100);
					sprintf(dir1, "/media/%s/log/log1_4.txt",dname);
					feed_watch_dog(wdt_id);
					if(0 != HLDFileCopy(LOG_3762_FILE3, dir1))
					{
						get_usb_status(&temp_status);
						temp_status.s_log |= 0x80;
						set_usb_status(temp_status);
					}

					get_usb_status(&temp_status);
					temp_status.s_log &= 0xF0;
					temp_status.s_log |= 0x02;
					set_usb_status(temp_status);

					feed_watch_dog(wdt_id);
					log_3762_task_start(&_log_3762_task);
				}
				if(0 == access(dir2, F_OK))
				{
					get_usb_status(&temp_status);
					temp_status.s_update = 1;
					set_usb_status(temp_status);

					//获取升级文件配置路径
					printf("update...\n");
					sprintf(str3, "%s/config.txt",dir2);
					feed_watch_dog(wdt_id);
					if(0 == get_updatefile_nameandcrc(str3, fname, &fcrc1))
					{
						sprintf(str1, "/opt/%s",fname);
						sprintf(str2,"%s/%s",dir2,fname);
						feed_watch_dog(wdt_id);
						if(0 == HLDFileCopy(str2, str1))
						{
							feed_watch_dog(wdt_id);
							if(0 == get_file_crc(str1, &fcrc2))
							{
								if(fcrc2 == fcrc1)
								{
									feed_watch_dog(wdt_id);
									update_config_file(fname);
									//改变升级文件权限
									memset(str1, 0, 100);
									sprintf(str1, "chmod +x /opt/%s", fname);
									system(str1);
									system("reboot");	//升级成功  重启系统
								}
								else
								{
									get_usb_status(&temp_status);
									temp_status.s_update = 6;
									set_usb_status(temp_status);
								}
							}
							else
							{
								get_usb_status(&temp_status);
								temp_status.s_update = 5;
								set_usb_status(temp_status);
							}
						}
						else
						{
							get_usb_status(&temp_status);
							temp_status.s_update = 4;
							set_usb_status(temp_status);
						}
					}
					else
					{
						get_usb_status(&temp_status);
						temp_status.s_update = 3;
						set_usb_status(temp_status);
					}
				}
			}
			else
			{
				usb_flag = 0;
			}

			while(0 == is_inserted_usb() && usb_flag == 1)	//等待U盘拔出
			{
				if(0 != access(dir2, F_OK) && 0 != access(dir1, F_OK))
				{
					usb_flag = 0;
				}
				sleep(1);
				printf("ssssssssss\n");
				feed_watch_dog(wdt_id);	//喂狗
			}
		}
	}

	pthread_exit(NULL);
}
