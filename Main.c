/*
 * Main.c
 *
 *  Created on: 2016��6��20��
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
 * ��������:����Ĭ�����в���
 * ����ֵ:��
 * */
void SetTolerantRunPara(void)
{
	memset(&_RunPara, 0, sizeof(RunPara));

	//Ĭ�����ڵ��ַ
	_RunPara.AFN05_1.HostNode[0] = 0;
	_RunPara.AFN05_1.HostNode[1] = 0;
	_RunPara.AFN05_1.HostNode[2] = 0;
	_RunPara.AFN05_1.HostNode[3] = 0;
	_RunPara.AFN05_1.HostNode[4] = 0;
	_RunPara.AFN05_1.HostNode[5] = 0;

	//Ĭ�Ͻ�ֹ�¼��ϱ�
	_RunPara.AFN05_2.IsAppera = 0;

	//Ĭ�ϼ�شӽڵ㳬ʱΪ15��
	_RunPara.AFN05_4.TimeOut = 15;

	//��������
	//ip
	_RunPara.IpPort.Ip[0] = 192;
	_RunPara.IpPort.Ip[1] = 168;
	_RunPara.IpPort.Ip[2] = 0;
	_RunPara.IpPort.Ip[3] = 44;

	//��������
	_RunPara.IpPort.NetMask[0] = 255;
	_RunPara.IpPort.NetMask[1] = 255;
	_RunPara.IpPort.NetMask[2] = 255;
	_RunPara.IpPort.NetMask[3] = 0;

	//����
	_RunPara.IpPort.GwIp[0] = 192;
	_RunPara.IpPort.GwIp[1] = 168;
	_RunPara.IpPort.GwIp[2] = 0;
	_RunPara.IpPort.GwIp[3] = 100;

	//TCP Server �˿ں�
	_RunPara.IpPort.Port = 9876;

	//��ֵ�ն� Server �˿ں�
	_RunPara.IpPort.TopPort = 8899;

	//���ճɹ��ĳɹ���־
	_RunPara.CyFlag = 0xFE;
}

/*
 * ��������:������������в�����AFN05  01
 * ����:	fd	�����ļ����ļ�������
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
			//�����в��������ڵ��ַ�������ò��������ڵ��ַ
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
 * ��������:������������в�����AFN05  02
 * ����:	fd	�����ļ����ļ�������
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
			//Ĭ�Ͻ�ֹ�¼��ϱ�
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
		//Ĭ�Ͻ�ֹ�¼��ϱ�
		_RunPara.AFN05_2.IsAppera = 0;
	}
}

/*
 * ��������:������������в�����AFN05  04
 * ����:	fd	�����ļ����ļ�������
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
			//Ĭ�ϼ�شӽڵ㳬ʱΪ15��
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
		//Ĭ�ϼ�شӽڵ㳬ʱΪ15��
		_RunPara.AFN05_4.TimeOut = 15;
	}
}

/*
 * ��������:������������в�����Ip Port
 * ����:	fd	�����ļ����ļ�������
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
			//��������
			//ip
			_RunPara.IpPort.Ip[0] = 192;
			_RunPara.IpPort.Ip[1] = 168;
			_RunPara.IpPort.Ip[2] = 0;
			_RunPara.IpPort.Ip[3] = 44;

			//��������
			_RunPara.IpPort.NetMask[0] = 255;
			_RunPara.IpPort.NetMask[1] = 255;
			_RunPara.IpPort.NetMask[2] = 255;
			_RunPara.IpPort.NetMask[3] = 0;

			//����
			_RunPara.IpPort.GwIp[0] = 192;
			_RunPara.IpPort.GwIp[1] = 168;
			_RunPara.IpPort.GwIp[2] = 0;
			_RunPara.IpPort.GwIp[3] = 100;

			//TCP Server �˿ں�
			_RunPara.IpPort.Port = 9876;

			//��ֵ�ն� Server �˿ں�
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
		//��������
		//ip
		_RunPara.IpPort.Ip[0] = 192;
		_RunPara.IpPort.Ip[1] = 168;
		_RunPara.IpPort.Ip[2] = 0;
		_RunPara.IpPort.Ip[3] = 44;

		//��������
		_RunPara.IpPort.NetMask[0] = 255;
		_RunPara.IpPort.NetMask[1] = 255;
		_RunPara.IpPort.NetMask[2] = 255;
		_RunPara.IpPort.NetMask[3] = 0;

		//����
		_RunPara.IpPort.GwIp[0] = 192;
		_RunPara.IpPort.GwIp[1] = 168;
		_RunPara.IpPort.GwIp[2] = 0;
		_RunPara.IpPort.GwIp[3] = 100;

		//TCP Server �˿ں�
		_RunPara.IpPort.Port = 9876;

		//TCP Server �˿ں�
		_RunPara.IpPort.TopPort = 8899;
	}
}

/*
 * ��������:������������в����ĳ���ɹ���־
 * ����:	fd	�����ļ����ļ�������
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
			//���ճɹ��ĳɹ���־
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
		//���ճɹ��ĳɹ���־
		_RunPara.CyFlag = 0xFE;
	}
}

/*
 * ��������:������ʼ��
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

	//�ж��ļ��Ƿ����  �������в���
	ret = access(CONFIG_FILE, F_OK);
	if(0 == ret)
	{
		//�ļ���ʧ�ܺ�����
		while(i--)
		{
			fd = open(CONFIG_FILE, O_RDWR, 0666);
			if(fd >= 0)
				break;
		}

		if(fd >= 0)	//�ļ��ɹ���
		{
			//���ڵ��ַ
			StartUpdataRunParaAFN05_01(fd);

			//��ֹ/����ֹ�¼��ϱ�
			StartUpdataRunParaAFN05_02(fd);

			//�ӽڵ������ʱʱ��
			StartUpdataRunParaAFN05_04(fd);

			//��������
			StartUpdataRunParaIpPort(fd);

			//����ɹ��ı�־
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
		//�ļ���ʧ�ܺ�����
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

	//����ϵͳ����
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
 * ��������:��ȡ���̨��
 * */
void GetAmmStandBook(void)
{
	int ret = 0;
	int fd;
	int i = 3;
	int len = 0;
	AmmAttribute amm;
	StandNode node;

	//�ж��ļ��Ƿ����  �������в���
	ret = access(STAND_BOOK_FILE, F_OK);
	if(ret == 0)
	{
		//����и��ļ�����ͬ��̨�˵��ڴ�
		while(i--)
		{
			fd = open(STAND_BOOK_FILE, O_RDWR);
			if(fd >= 0)
				break;
		}
		if(fd >=0)
		{
			//��ȡ̨���ļ�����ͬ�����ڴ���
			for(i = 0; i < AMM_MAX_NUM; i++)
			{
				memset(&amm, 0, sizeof(AmmAttribute));
				len = read(fd, &amm, sizeof(AmmAttribute));
				if(len != sizeof(AmmAttribute))
					break;
				len = offsetof(AmmAttribute, CS);
				if(amm.CS == Func_CS(&amm, len))
				{
					//���ַ
					memcpy(node.Amm, amm.Amm, AMM_ADDR_LEN);
					//�ն˵�ַ
					memcpy(node.Ter, amm.Ter, TER_ADDR_LEN);
					//�����־
					node.cyFlag = amm.cyFlag;
					//���(������ļ��е�λ��)
					node.num = i;
					//��Լ����
					node.type = amm.type;

					//���̨�˵��ڴ�
					AddNodeStand(&node);
				}
				else
				{
					//���ļ�λ��Ϊ����
					SetStandFleSurplus(i);
				}
			}
			close(fd);
		}
		else
		{
			//̨���ļ�ȫ������
			StandFileAllSurplus();
		}
	}
	else
	{
		//���û�и��ļ��������̨���ļ�0xFF
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

		//̨���ļ�ȫ������
		StandFileAllSurplus();
	}
}

/*
 * ��������:ȫ�ֱ�����ʼ��
 * */
void GlobalVariableInit(void)
{
	//���в���
	memset(&_RunPara, 0, sizeof(RunPara));
	//���������̨�˽ڵ�
	memset(_SortNode, 0, sizeof(StandNode *) * AMM_MAX_NUM);
	//·�ɵ�һ���ڵ��ַ
	_FristNode = NULL;
	//������·�ɽڵ��һ���ڵ��ַ
	_FristRecycleNode = NULL;
	//��ֵ�ն˵�һ���ڵ��ַ
	_FristTop = NULL;
	//�����ճ�ֵ�ն˽ڵ��һ���ڵ��ַ
	_FristRecycleTopNode = NULL;
	//�̳߳ص�ַ
	_Threadpool = NULL;
	//��ʼ��̨�˽ڵ�����
	StandNodeNumInit();
	//��ʼ��̨���ļ�����λ�ñ�־
	StandFileAllSurplus();
}

/*
 * ��������:��̫��ģ���ʼ��
 * */
void NetworkModeInit(void)
{
	//ȫ�ֱ�����ʼ��
	GlobalVariableInit();

	//��ȡ���в��� ����ģ������
	RunParaInit();

	//��ȡ���̨��
	GetAmmStandBook();

	//��ʼ���̳߳�
	_Threadpool = threadpool_init(THREAD_POOL_SIZE, QUEUE_MAX_NUM);

	//��Ҫ���Ե��ź�
	signal(SIGPIPE, signalaction);
	signal(SIGIO, signalaction);
//	signal(SIGSEGV, signalaction);

	//��������ʼ��
	CollectorInit();

	//���Ź���ʼ��
	watch_dog_init();

	//�������ӡ��ʼ��
	hld_printf_init();
}

/*
 * ��������:�����������߳�
 * ����:	__start_routine	�̺߳���
 * 		id				���Ź�id
 * 		maxtime 		���ʱʱ��(���ÿ��Ź�ʹ��)
 * ����ֵ �߳̾��
 * */
pthread_t new_pthread(void *(*__start_routine) (void *),int *id, int maxtime)
{
	pthread_t pt;
	//���뿴�Ź�
	*id = apply_watch_dog();
	if(*id < 0)
	{
		system("reboot");
	}
	//���ÿ��Ź�
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
