/*
 * Main.c
 *
 *  Created on: 2016年6月20日
 *      Author: Administrator
 */

#include "Main.h"

void signalaction(int num) {
	if (SIGPIPE == num) {
		printf("\n network abnormal\n");
	} else if (SIGIO == num) {
		printf("\n IO abnormal \n");
	} else if (SIGSEGV == num) {
		printf("\n segment fault\n");
		system("reboot");
	}
}

/*
 * 函数功能:设置默认运行参数
 * 返回值:无
 * */
void SetTolerantRunPara(void)
{
	memset(&_RunPara, 0, sizeof(RunPara));

	//默认主节点地址
	_RunPara.AFN05_1.HostNode[0] = 0;
	_RunPara.AFN05_1.HostNode[1] = 0;
	_RunPara.AFN05_1.HostNode[2] = 0;
	_RunPara.AFN05_1.HostNode[3] = 0;
	_RunPara.AFN05_1.HostNode[4] = 0;
	_RunPara.AFN05_1.HostNode[5] = 0;

	//默认禁止事件上报
	_RunPara.AFN05_2.IsAppera = 0;

	//默认监控从节点超时为15秒
	_RunPara.AFN05_4.TimeOut = 15;

	//网络设置
	//ip
	_RunPara.IpPort.Ip[0] = 192;
	_RunPara.IpPort.Ip[1] = 168;
	_RunPara.IpPort.Ip[2] = 0;
	_RunPara.IpPort.Ip[3] = 44;

	//子网掩码
	_RunPara.IpPort.NetMask[0] = 255;
	_RunPara.IpPort.NetMask[1] = 255;
	_RunPara.IpPort.NetMask[2] = 255;
	_RunPara.IpPort.NetMask[3] = 0;

	//网关
	_RunPara.IpPort.GwIp[0] = 192;
	_RunPara.IpPort.GwIp[1] = 168;
	_RunPara.IpPort.GwIp[2] = 0;
	_RunPara.IpPort.GwIp[3] = 100;

	//TCP Server 端口号
	_RunPara.IpPort.Port = 9876;

	//充值终端 Server 端口号
	_RunPara.IpPort.TopPort = 8899;

	//抄收成功的成功标志
	_RunPara.CyFlag = 0xFE;
}

/*
 * 函数功能:开机后更新运行参数的AFN05  01
 * 参数:	fd	配置文件的文件描述符
 * */
void StartUpdataRunParaAFN05_01(int fd)
{
	tpAFN05_1 afn05_1;
	int len = 0;
	memset(&afn05_1, 0, sizeof(tpAFN05_1));

	len = offsetof(tpConfiguration, AFN05_1);
	if(0 == ReadFile(fd, len, (void *)(&afn05_1), sizeof(tpAFN05_1)))
	{
		len = offsetof(tpAFN05_1, CS);
		if(afn05_1.CS != Func_CS((void*)&afn05_1, len))
		{
			//将运行参数的主节点地址赋给配置参数的主节点地址
			_RunPara.AFN05_1.HostNode[0] = 0;
			_RunPara.AFN05_1.HostNode[1] = 0;
			_RunPara.AFN05_1.HostNode[2] = 0;
			_RunPara.AFN05_1.HostNode[3] = 0;
			_RunPara.AFN05_1.HostNode[4] = 0;
			_RunPara.AFN05_1.HostNode[5] = 0;
			memcpy(&afn05_1.HostNode, &_RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);
			afn05_1.CS = Func_CS((void*)&afn05_1, len);
			len = offsetof(tpConfiguration, AFN05_1);
			WriteFile(fd, len, (void*)&afn05_1, sizeof(tpAFN05_1));
		}
		else
		{
			memcpy(&_RunPara.AFN05_1.HostNode, &afn05_1.HostNode, NODE_ADDR_LEN);
		}
	}
	else
	{
		_RunPara.AFN05_1.HostNode[0] = 0;
		_RunPara.AFN05_1.HostNode[1] = 0;
		_RunPara.AFN05_1.HostNode[2] = 0;
		_RunPara.AFN05_1.HostNode[3] = 0;
		_RunPara.AFN05_1.HostNode[4] = 0;
		_RunPara.AFN05_1.HostNode[5] = 0;
	}
}

/*
 * 函数功能:开机后更新运行参数的AFN05  02
 * 参数:	fd	配置文件的文件描述符
 * */
void StartUpdataRunParaAFN05_02(int fd)
{
	tpAFN05_2 afn05_2;
	memset(&afn05_2, 0, sizeof(tpAFN05_2));
	int len = 0;

	len = offsetof(tpConfiguration, AFN05_2);
	if(0 == ReadFile(fd, len, (void *)(&afn05_2), sizeof(tpAFN05_2)))
	{
		len = offsetof(tpAFN05_2, CS);
		if(afn05_2.CS != Func_CS((void*)&afn05_2, len))
		{
			//默认禁止事件上报
			_RunPara.AFN05_2.IsAppera = 0;

			memcpy(&afn05_2.IsAppera, &_RunPara.AFN05_2.IsAppera, 1);
			afn05_2.CS = Func_CS((void*)&afn05_2, len);
			len = offsetof(tpConfiguration, AFN05_2);
			WriteFile(fd, len, (void*)&afn05_2, sizeof(tpAFN05_2));
		}
		else
		{
			memcpy(&_RunPara.AFN05_2.IsAppera, &afn05_2.IsAppera, 1);
		}
	}
	else
	{
		//默认禁止事件上报
		_RunPara.AFN05_2.IsAppera = 0;
	}
}

/*
 * 函数功能:开机后更新运行参数的AFN05  04
 * 参数:	fd	配置文件的文件描述符
 * */
void StartUpdataRunParaAFN05_04(int fd)
{
	tpAFN05_4 afn05_4;
	memset(&afn05_4, 0, sizeof(tpAFN05_4));

	int len = 0;

	len = offsetof(tpConfiguration, AFN05_4);
	if(0 == ReadFile(fd, len, (void *)(&afn05_4), sizeof(tpAFN05_4)))
	{
		len = offsetof(tpAFN05_4, CS);
		if(afn05_4.CS != Func_CS((void*)&afn05_4, len))
		{
			//默认监控从节点超时为15秒
			_RunPara.AFN05_4.TimeOut = 15;

			memcpy(&afn05_4.TimeOut, &_RunPara.AFN05_4.TimeOut, 1);
			afn05_4.CS = Func_CS((void*)&afn05_4, len);
			len = offsetof(tpConfiguration, AFN05_4);
			WriteFile(fd, len, (void*)&afn05_4, sizeof(tpAFN05_4));
		}
		else
		{
			memcpy(&_RunPara.AFN05_4.TimeOut, &afn05_4.TimeOut, 1);
		}
	}
	else
	{
		//默认监控从节点超时为15秒
		_RunPara.AFN05_4.TimeOut = 15;
	}
}

/*
 * 函数功能:开机后更新运行参数的Ip Port
 * 参数:	fd	配置文件的文件描述符
 * */
void StartUpdataRunParaIpPort(int fd)
{
	tpIpPort  ipPort;
	memset(&ipPort, 0, sizeof(tpIpPort));
	int len = 0;
	len = offsetof(tpConfiguration, IpPort);

	if(0 == ReadFile(fd, len, (void *)(&ipPort), sizeof(tpIpPort)))
	{
		len = offsetof(tpIpPort, CS);
		if(ipPort.CS != Func_CS((void*)&ipPort, len))
		{
			//网络设置
			//ip
			_RunPara.IpPort.Ip[0] = 192;
			_RunPara.IpPort.Ip[1] = 168;
			_RunPara.IpPort.Ip[2] = 0;
			_RunPara.IpPort.Ip[3] = 44;

			//子网掩码
			_RunPara.IpPort.NetMask[0] = 255;
			_RunPara.IpPort.NetMask[1] = 255;
			_RunPara.IpPort.NetMask[2] = 255;
			_RunPara.IpPort.NetMask[3] = 0;

			//网关
			_RunPara.IpPort.GwIp[0] = 192;
			_RunPara.IpPort.GwIp[1] = 168;
			_RunPara.IpPort.GwIp[2] = 0;
			_RunPara.IpPort.GwIp[3] = 100;

			//TCP Server 端口号
			_RunPara.IpPort.Port = 9876;

			//充值终端 Server 端口号
			_RunPara.IpPort.TopPort = 8899;

			memcpy(&ipPort, &_RunPara.IpPort, sizeof(tpIpPort));
			ipPort.CS = Func_CS((void*)&ipPort, len);
			len = offsetof(tpConfiguration, IpPort);
			WriteFile(fd, len, (void*)&ipPort, sizeof(tpIpPort));
		}
		else
		{
			memcpy(&_RunPara.IpPort, &ipPort, sizeof(tpIpPort));
		}
	}
	else
	{
		//网络设置
		//ip
		_RunPara.IpPort.Ip[0] = 192;
		_RunPara.IpPort.Ip[1] = 168;
		_RunPara.IpPort.Ip[2] = 0;
		_RunPara.IpPort.Ip[3] = 44;

		//子网掩码
		_RunPara.IpPort.NetMask[0] = 255;
		_RunPara.IpPort.NetMask[1] = 255;
		_RunPara.IpPort.NetMask[2] = 255;
		_RunPara.IpPort.NetMask[3] = 0;

		//网关
		_RunPara.IpPort.GwIp[0] = 192;
		_RunPara.IpPort.GwIp[1] = 168;
		_RunPara.IpPort.GwIp[2] = 0;
		_RunPara.IpPort.GwIp[3] = 100;

		//TCP Server 端口号
		_RunPara.IpPort.Port = 9876;

		//TCP Server 端口号
		_RunPara.IpPort.TopPort = 8899;
	}
}

/*
 * 函数功能:开机后更新运行参数的抄表成功标志
 * 参数:	fd	配置文件的文件描述符
 * */
void StartUpdataRunParaCyFlag(int fd)
{
	tpCyFlag  cyFlag;
	memset(&cyFlag, 0, sizeof(tpCyFlag));

	int len = 0;

	len = offsetof(tpConfiguration, cyFlag);
	if(0 == ReadFile(fd, len, (void *)(&cyFlag), sizeof(tpCyFlag)))
	{
		len = offsetof(tpCyFlag, CS);
		if(cyFlag.CS != Func_CS((void*)&cyFlag, len))
		{
			//抄收成功的成功标志
			_RunPara.CyFlag = 0xFE;

			memcpy(&cyFlag.flag, &_RunPara.CyFlag, 1);
			cyFlag.CS = Func_CS((void*)&cyFlag, len);
			len = offsetof(tpConfiguration, cyFlag);
			WriteFile(fd, len, (void*)&cyFlag, sizeof(tpCyFlag));
		}
		else
		{
			memcpy(&_RunPara.CyFlag, &cyFlag.flag, 1);
		}
	}
	else
	{
		//抄收成功的成功标志
		_RunPara.CyFlag = 0xFE;
	}
}

/*
 * 函数功能:参数初始化
 * */
void RunParaInit(void)
{
	int ret = 0;
	int fd;
	int i = 3;
	unsigned char eth[10] = { 0 };
	unsigned char ip[20] = { 0 };
	unsigned char netmask[20] = { 0 };
	unsigned char gwip[20] = { 0 };

	//判断文件是否存在  设置运行参数
	ret = access(CONFIG_FILE, F_OK);
	if(0 == ret)
	{
		//文件打开失败后重试
		while(i--)
		{
			fd = open(CONFIG_FILE, O_RDWR, 0666);
			if(fd >= 0)
				break;
		}

		if(fd >= 0)	//文件成功打开
		{
			//主节点地址
			StartUpdataRunParaAFN05_01(fd);

			//禁止/不禁止事件上报
			StartUpdataRunParaAFN05_02(fd);

			//从节点监控最大超时时间
			StartUpdataRunParaAFN05_04(fd);

			//网络设置
			StartUpdataRunParaIpPort(fd);

			//抄表成功的标志
			StartUpdataRunParaCyFlag(fd);

			close(fd);
		}
		else
		{
			SetTolerantRunPara();
		}
	}
	else
	{
		//文件打开失败后重试
		while(i--)
		{
			fd = open(CONFIG_FILE, O_RDWR | O_CREAT, 0666);
			if(fd >= 0)
				break;
		}
		if(fd >= 0)
		{
			tpConfiguration config;
			memset(&config, 0xFF, sizeof(tpConfiguration));
			WriteFile(fd, 0, (void*)&config, sizeof(tpConfiguration));
			close(fd);
		}
		SetTolerantRunPara();
	}

	//设置系统网络
	//system("pkill -9 udhcpc");
	sleep(1);
	sprintf((char *) eth, "eth0");
	sprintf((char *) ip, "%u.%u.%u.%u", _RunPara.IpPort.Ip[0],
			_RunPara.IpPort.Ip[1], _RunPara.IpPort.Ip[2],
			_RunPara.IpPort.Ip[3]);
	sprintf((char *) netmask, "%u.%u.%u.%u", _RunPara.IpPort.NetMask[0],
			_RunPara.IpPort.NetMask[1], _RunPara.IpPort.NetMask[2],
			_RunPara.IpPort.NetMask[3]);
	sprintf((char*) gwip, "%u.%u.%u.%u", _RunPara.IpPort.GwIp[0],
			_RunPara.IpPort.GwIp[1], _RunPara.IpPort.GwIp[2],
			_RunPara.IpPort.GwIp[3]);
	IfConfigEthnert((char*) eth, (char*) ip, (char*) netmask,
			(char*) gwip);
	sleep(3);
}

/*
 * 函数功能:获取电表台账
 * */
void GetAmmStandBook(void)
{
	int ret = 0;
	int fd;
	int i = 3;
	int len = 0;
	AmmAttribute amm;
	StandNode node;

	//判断文件是否存在  设置运行参数
	ret = access(STAND_BOOK_FILE, F_OK);
	if(ret == 0)
	{
		//如果有该文件，则同步台账到内存
		while(i--)
		{
			fd = open(STAND_BOOK_FILE, O_RDWR);
			if(fd >= 0)
				break;
		}
		if(fd >=0)
		{
			//读取台账文件内容同步到内存中
			for(i = 0; i < AMM_MAX_NUM; i++)
			{
				memset(&amm, 0, sizeof(AmmAttribute));
				len = read(fd, &amm, sizeof(AmmAttribute));
				if(len != sizeof(AmmAttribute))
					break;
				len = offsetof(AmmAttribute, CS);
				if(amm.CS == Func_CS(&amm, len))
				{
					//表地址
					memcpy(node.Amm, amm.Amm, AMM_ADDR_LEN);
					//终端地址
					memcpy(node.Ter, amm.Ter, TER_ADDR_LEN);
					//抄表标志
					node.cyFlag = amm.cyFlag;
					//序号(电表在文件中的位置)
					node.num = i;
					//规约类型
					node.type = amm.type;

					//添加台账到内存
					AddNodeStand(&node);
				}
				else
				{
					//该文件位置为空余
					SetStandFleSurplus(i);
				}
			}
			close(fd);
		}
		else
		{
			//台账文件全部空余
			StandFileAllSurplus();
		}
	}
	else
	{
		//如果没有该文件，则填充台账文件0xFF
		memset(&amm, 0xFF, sizeof(AmmAttribute));
		while(i--)
		{
			fd = open(STAND_BOOK_FILE, O_RDWR | O_CREAT, 0666);
			if(fd >= 0)
				break;
		}
		if(fd >=0)
		{
			for(i = 0; i < AMM_MAX_NUM; i++)
			{
				if(amm.cyFlag == _RunPara.CyFlag)
					amm.cyFlag = _RunPara.CyFlag + 1;
				write(fd, &amm, sizeof(AmmAttribute));
			}
		}
		close(fd);

		//台账文件全部空余
		StandFileAllSurplus();
	}
}

/*
 * 函数功能:全局变量初始化
 * */
void GlobalVariableInit(void)
{
	//运行参数
	memset(&_RunPara, 0, sizeof(RunPara));
	//存放排序后的台账节点
	memset(_SortNode, 0, sizeof(StandNode *) * AMM_MAX_NUM);
	//路由第一个节点地址
	_FristNode = NULL;
	//待回收路由节点第一个节点地址
	_FristRecycleNode = NULL;
	//充值终端第一个节点地址
	_FristTop = NULL;
	//待回收充值终端节点第一个节点地址
	_FristRecycleTopNode = NULL;
	//线程池地址
	_Threadpool = NULL;
	//初始化台账节点数量
	StandNodeNumInit();
	//初始化台账文件空余位置标志
	StandFileAllSurplus();
}

/*
 * 函数功能:以太网模块初始化
 * */
void NetworkModeInit(void)
{
	//全局变量初始化
	GlobalVariableInit();

	//获取运行参数 设置模块网络
	RunParaInit();

	//获取电表台账
	GetAmmStandBook();

	//初始化线程池
	_Threadpool = threadpool_init(THREAD_POOL_SIZE, QUEUE_MAX_NUM);

	//需要忽略的信号
	signal(SIGPIPE, signalaction);
	signal(SIGIO, signalaction);
//	signal(SIGSEGV, signalaction);

	//抄表器初始化
	CollectorInit();

	//看门狗初始化
	watch_dog_init();

	//华立达打印初始化
	hld_printf_init();
}

/*
 * 函数功能:创建华立达线程
 * 参数:	__start_routine	线程函数
 * 		id				看门狗id
 * 		maxtime 		最大超时时间(设置看门狗使用)
 * 返回值 线程句柄
 * */
pthread_t new_pthread(void *(*__start_routine) (void *),int *id, int maxtime)
{
	pthread_t pt;
	//申请看门狗
	*id = apply_watch_dog();
	if(*id < 0)
	{
		system("reboot");
	}
	//设置看门狗
	set_watch_dog(*id, maxtime);
	pthread_create(&pt, NULL, __start_routine, (void*)id);

	return pt;
}

int main(int argc, char *argv[])
{
	NetworkModeInit();

	pthread_t pt[6];
	int id[5] = {0};

	if(0 == pthread_create(&pt[0], NULL, HLDWatchDog, NULL))
	{
		pt[1] = new_pthread(NetWork0, &id[0], 6);
		pt[2] = new_pthread(Infrared, &id[1], 6);
		pt[3] = new_pthread(Collector, &id[2], 30);
		pt[4] = new_pthread(Usart0, &id[3], 6);

		pthread_join(pt[4], NULL);
		pthread_join(pt[3], NULL);
		pthread_join(pt[1], NULL);
		pthread_join(pt[2], NULL);
		pthread_join(pt[0], NULL);
	}
	else
	{
		return reboot(LINUX_REBOOT_CMD_RESTART);
	}
}
