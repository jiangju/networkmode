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
//		system("reboot");
	}
}

/*
 * ��������:����Ĭ�����в���
 * ����ֵ:��
 * */
void SetTolerantRunPara(void)
{
	pthread_mutex_lock(&_RunPara.mutex);
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

	//��̨���·�
	_RunPara.StandFlag = 0x55;

	//��Ȩ����˵�ַ���˿�
	_RunPara.server.ip[0] = 192;
	_RunPara.server.ip[1] = 168;
	_RunPara.server.ip[2] = 0;
	_RunPara.server.ip[3] = 30;

	_RunPara.server.port = 10010;

	pthread_mutex_unlock(&_RunPara.mutex);
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
			pthread_mutex_lock(&_RunPara.mutex);
			_RunPara.AFN05_1.HostNode[0] = 0;
			_RunPara.AFN05_1.HostNode[1] = 0;
			_RunPara.AFN05_1.HostNode[2] = 0;
			_RunPara.AFN05_1.HostNode[3] = 0;
			_RunPara.AFN05_1.HostNode[4] = 0;
			_RunPara.AFN05_1.HostNode[5] = 0;
			memcpy(&afn05_1.HostNode, &_RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);
			pthread_mutex_unlock(&_RunPara.mutex);
			afn05_1.CS = Func_CS((void*)&afn05_1, len);
			len = offsetof(tpConfiguration, AFN05_1);
			WriteFile(fd, len, (void*)&afn05_1, sizeof(tpAFN05_1));
		}
		else
		{
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.AFN05_1.HostNode, &afn05_1.HostNode, NODE_ADDR_LEN);
			pthread_mutex_unlock(&_RunPara.mutex);
		}
	}
	else
	{
		pthread_mutex_lock(&_RunPara.mutex);
		_RunPara.AFN05_1.HostNode[0] = 0;
		_RunPara.AFN05_1.HostNode[1] = 0;
		_RunPara.AFN05_1.HostNode[2] = 0;
		_RunPara.AFN05_1.HostNode[3] = 0;
		_RunPara.AFN05_1.HostNode[4] = 0;
		_RunPara.AFN05_1.HostNode[5] = 0;
		pthread_mutex_unlock(&_RunPara.mutex);
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
			pthread_mutex_lock(&_RunPara.mutex);
			_RunPara.AFN05_2.IsAppera = 0;

			memcpy(&afn05_2.IsAppera, &_RunPara.AFN05_2.IsAppera, 1);
			pthread_mutex_unlock(&_RunPara.mutex);
			afn05_2.CS = Func_CS((void*)&afn05_2, len);
			len = offsetof(tpConfiguration, AFN05_2);
			WriteFile(fd, len, (void*)&afn05_2, sizeof(tpAFN05_2));
		}
		else
		{
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.AFN05_2.IsAppera, &afn05_2.IsAppera, 1);
			pthread_mutex_unlock(&_RunPara.mutex);
		}
	}
	else
	{
		pthread_mutex_lock(&_RunPara.mutex);
		//Ĭ�Ͻ�ֹ�¼��ϱ�
		_RunPara.AFN05_2.IsAppera = 0;
		pthread_mutex_unlock(&_RunPara.mutex);
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
			pthread_mutex_lock(&_RunPara.mutex);
			_RunPara.AFN05_4.TimeOut = 15;

			memcpy(&afn05_4.TimeOut, &_RunPara.AFN05_4.TimeOut, 1);
			pthread_mutex_unlock(&_RunPara.mutex);
			afn05_4.CS = Func_CS((void*)&afn05_4, len);
			len = offsetof(tpConfiguration, AFN05_4);
			WriteFile(fd, len, (void*)&afn05_4, sizeof(tpAFN05_4));
		}
		else
		{
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.AFN05_4.TimeOut, &afn05_4.TimeOut, 1);
			pthread_mutex_unlock(&_RunPara.mutex);
		}
	}
	else
	{
		//Ĭ�ϼ�شӽڵ㳬ʱΪ15��
		pthread_mutex_lock(&_RunPara.mutex);
		_RunPara.AFN05_4.TimeOut = 15;
		pthread_mutex_unlock(&_RunPara.mutex);
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
			pthread_mutex_lock(&_RunPara.mutex);
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
			pthread_mutex_unlock(&_RunPara.mutex);
			ipPort.CS = Func_CS((void*)&ipPort, len);
			len = offsetof(tpConfiguration, IpPort);
			WriteFile(fd, len, (void*)&ipPort, sizeof(tpIpPort));
		}
		else
		{
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.IpPort, &ipPort, sizeof(tpIpPort));
			pthread_mutex_unlock(&_RunPara.mutex);
		}
	}
	else
	{
		//��������
		pthread_mutex_lock(&_RunPara.mutex);
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
		pthread_mutex_unlock(&_RunPara.mutex);
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
			pthread_mutex_lock(&_RunPara.mutex);
			_RunPara.CyFlag = 0xFE;

			memcpy(&cyFlag.flag, &_RunPara.CyFlag, 1);
			pthread_mutex_unlock(&_RunPara.mutex);
			cyFlag.CS = Func_CS((void*)&cyFlag, len);
			len = offsetof(tpConfiguration, cyFlag);
			WriteFile(fd, len, (void*)&cyFlag, sizeof(tpCyFlag));
		}
		else
		{
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.CyFlag, &cyFlag.flag, 1);
			pthread_mutex_unlock(&_RunPara.mutex);
		}
	}
	else
	{
		//���ճɹ��ĳɹ���־
		pthread_mutex_lock(&_RunPara.mutex);
		_RunPara.CyFlag = 0xFE;
		pthread_mutex_unlock(&_RunPara.mutex);
	}
}

/*
 * ��������:������������в�����̨���·���׼��
 * ����:	fd	�����ļ����ļ�������
 * */
void StartUpdataRunParaIsStand(int fd)
{
	tpIsStand stand;
	int len = 0;
	len = offsetof(tpConfiguration, StandFlag);
	if(0 == ReadFile(fd, len, (void *)(&stand), sizeof(tpIsStand)))
	{
		len = offsetof(tpIsStand, CS);
		if(stand.CS != Func_CS((void*)&stand, len))
		{
			pthread_mutex_lock(&_RunPara.mutex);
			//̨���·�״̬
			_RunPara.StandFlag = 0x55;

			memcpy(&stand.flag, &_RunPara.StandFlag, 1);
			pthread_mutex_unlock(&_RunPara.mutex);
			stand.CS = Func_CS((void*)&stand, len);
			len = offsetof(tpConfiguration, StandFlag);
			WriteFile(fd, len, (void*)&stand, sizeof(tpIsStand));
		}
		else
		{
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.StandFlag, &stand.flag, 1);
			pthread_mutex_unlock(&_RunPara.mutex);
		}
	}
	else
	{
		//̨���·�״̬ δ�·�
		pthread_mutex_lock(&_RunPara.mutex);
		_RunPara.StandFlag = 0x55;
		pthread_mutex_unlock(&_RunPara.mutex);
	}
}

/*
 * ��������:�������ʼ�������ip���˿�
 * ����:fd
 * */
void StartUpdataRunParaServer(int fd)
{
	tpServer s;
	memset(&s, 0, sizeof(tpServer));
	int len = 0;
	len = offsetof(tpConfiguration, server);
	if(0 == ReadFile(fd, len, (void *)(&s), sizeof(tpServer)))
	{
		len = offsetof(tpServer, CS);
		if(s.CS != Func_CS(&s, len))
		{
			pthread_mutex_lock(&_RunPara.mutex);
			_RunPara.server.ip[0] = 192;
			_RunPara.server.ip[1] = 168;
			_RunPara.server.ip[2] = 0;
			_RunPara.server.ip[3] = 30;
			_RunPara.server.port = 10010;
			memcpy(&s, &_RunPara.server, sizeof(tpServer));
			pthread_mutex_unlock(&_RunPara.mutex);
			s.CS = Func_CS(&s, len);
			len = offsetof(tpConfiguration, server);
			WriteFile(fd, len, (void*)&s, sizeof(tpServer));
		}
		else
		{
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.server, &s, sizeof(tpServer));
			pthread_mutex_unlock(&_RunPara.mutex);
		}
	}
	else
	{
		pthread_mutex_lock(&_RunPara.mutex);
		_RunPara.server.ip[0] = 192;
		_RunPara.server.ip[1] = 168;
		_RunPara.server.ip[2] = 0;
		_RunPara.server.ip[3] = 30;
		_RunPara.server.port = 10010;
		pthread_mutex_unlock(&_RunPara.mutex);
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

	pthread_mutex_init(&_RunPara.mutex, NULL);

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

			//�Ƿ����·�̨�˱�־
			StartUpdataRunParaIsStand(fd);

			//��Ȩ�����ip���˿�
			StartUpdataRunParaServer(fd);

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
	pthread_mutex_lock(&_RunPara.mutex);
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
	pthread_mutex_unlock(&_RunPara.mutex);
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
				pthread_mutex_lock(&_RunPara.mutex);
				if(amm.cyFlag == _RunPara.CyFlag)
				{
					amm.cyFlag = _RunPara.CyFlag + 1;
				}
				pthread_mutex_unlock(&_RunPara.mutex);
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
}

/*
 * ��������:��̫��ģ���ʼ��
 * */
void NetworkModeInit(void)
{
	//ȫ�ֱ�����ʼ��
	GlobalVariableInit();

	HldStandInit();
	//��ʼ��̨�˽ڵ�����
	StandNodeNumInit();
	//��ʼ��̨���ļ�����λ�ñ�־
	StandFileAllSurplus();

	//��ȡ���в��� ����ģ������
	RunParaInit();

	//��ȡ�����̨��
	GetAmmStandBook();

	//��ʼ���̳߳�
	_Threadpool = threadpool_init(THREAD_POOL_SIZE, QUEUE_MAX_NUM);

	//��Ҫ���Ե��ź�
	signal(SIGPIPE, signalaction);
	signal(SIGIO, signalaction);
	signal(SIGSEGV, signalaction);

	//��������ʼ��
	CollectorInit();

	//����̨�˳�ʼ��
	init_initiative_stand();

	//���Ź���ʼ��
	watch_dog_init();

	//��ʼ����Ȩģ��
	init_hld_ac_mode();
	open_hld_ac();
	//376.2��־�ļ�״̬��ʼ��
//	init_3762log();
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
		printf("apply watch dog erro\n");
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

	pthread_t pt[8];
	int id[7] = {0};

	if(0 == pthread_create(&pt[0], NULL, HLDWatchDog, NULL))
	{
		pt[1] = new_pthread(NetWork0, &id[0], 10);
		pt[2] = new_pthread(Infrared, &id[1], 10);
		pt[3] = new_pthread(Collector, &id[2], 30);
		pt[4] = new_pthread(Usart0, &id[3], 10);
		pt[5] = new_pthread(pthread_log_3762, &id[4], 10);
		pt[6] = new_pthread(pthread_usb, &id[5], 60);
		pt[7] = new_pthread(NetWork1, &id[6], 10);

		pthread_join(pt[7], NULL);
		pthread_join(pt[6], NULL);
		pthread_join(pt[5], NULL);
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
