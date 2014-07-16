/*
* canif.cpp
* can�豸�ӿڵķ�װ
*
*  Created on: 2009-8-3
*      Author: Owner
*/
#include <Windows.h>
#include <iostream>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include "canif.h"
#include "ControlCan.h"
#include "canidcmd.h"
#include "caution.h"
#include <QMap>
#include "DB.h"
#include "tech.h"
#include "CMotor.h"
#include "canidcmd.h"

//���ݿ�
extern DB mydb;

extern tech* ptech;
//A4N���
extern CKSMCA4 *ksmc;

//ͨѶ����
com pubcom;
int GetParaRow(const QString& groupname);

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);
extern double f_follow;

canif::canif()
{
	succeedopenandinit = false;
	nDeviceType = VCI_PCI9820; //�豸����
	nDeviceInd = 0; //�豸��������װ�ж��can��ʱ��ʶ��ֻ��
	nReserved = 0; //����PCI9820������������
	nCANInd = 0; //can������Ķ˿ں�
}

canif::~canif()
{
	close();
}

bool canif::open()
{

	devmutex.lock();

	//ȡ�ò���
	bool ok;
	QDomNode para = GetParaByName("dev", "can����");
	speed = para.firstChildElement("value").text().toInt(&ok);

	para = GetParaByName("dev", "can��ѡ��");
	nCANInd = para.firstChildElement("value").text().toInt(&ok);


	VCI_INIT_CONFIG vic; //��ʼ�����ݽṹ
	DWORD dwRel;


	vic.AccCode = PCNODEID;
	vic.AccMask = PCRECVMASK;
	//vic.AccCode = 0x00000000;
	//vic.AccMask = 0xffffffff;

	vic.Filter = 0;

	//���������趨��ʱ��
	switch(speed) {
		case 10:
			vic.Timing0 = 0x31;
			vic.Timing1 = 0x1c;
			break;

		case 20:
			vic.Timing0 = 0x18;
			vic.Timing1 = 0x1c;
			break;

		case 50:
			vic.Timing0 = 0x09;
			vic.Timing1 = 0x1c;
			break;

		case 100:
			vic.Timing0 = 0x04;
			vic.Timing1 = 0x1c;
			break;

		case 125:
			vic.Timing0 = 0x03;
			vic.Timing1 = 0x1c;
			break;

		case 250:
			vic.Timing0 = 0x01;
			vic.Timing1 = 0x1c;
			break;

		case 500:
			vic.Timing0 = 0x00;
			vic.Timing1 = 0x1c;
			break;

		case 800:
			vic.Timing0 = 0x00;
			vic.Timing1 = 0x16;
			break;

		case 1000:
			vic.Timing0 = 0x00;
			vic.Timing1 = 0x14;
			break;

		default:
			vic.Timing0 = 0x01;
			vic.Timing1 = 0x1c;
			break;

	}

	vic.Mode = 0;

#ifdef HAVECANCARD

	//��CAN��
	dwRel = VCI_OpenDevice(nDeviceType, nDeviceInd, nReserved);

	if (dwRel != STATUS_OK)
	{
		publiccaution.addevent("CAN","CANͨѶ����ʧ��","CANͨѶ����ʧ��");
		devmutex.unlock();
		return false;
	}
	else
	{
	}

#endif

#ifdef HAVECANCARD
	//��ʼ��CAN��
	dwRel = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd, &vic);
	if (dwRel != STATUS_OK)
	{
		publiccaution.addevent("CAN","CAN����ʼ��ʧ��","CAN����ʼ��ʧ��");
		devmutex.unlock();
    	return false;
	}
	else
	{
	}
#endif

#ifdef HAVECANCARD
	//����CAN��
	dwRel = VCI_StartCAN(nDeviceType, nDeviceInd, nCANInd);
	if (dwRel != STATUS_OK)
	{
		VCI_ERR_INFO pErrInfo;
		dwRel = VCI_ReadErrInfo(nDeviceType, nDeviceInd, nCANInd, &pErrInfo);
		if (dwRel != STATUS_OK)
		{
			publiccaution.addevent("CAN","CAN������ʧ��","CAN������ʧ��");
			publiccaution.adddbginfo(QString("ReadErrInfo failed ,start can failed,err:") + QString::number(pErrInfo.ErrCode) );
		}
		devmutex.unlock();
		return false;
	}
	else
	{
#endif
		succeedopenandinit = true;
#ifdef HAVECANCARD
	}
#endif
	devmutex.unlock();
	return true;
}

bool canif::close()
{
	devmutex.lock();
#ifdef HAVECANCARD
	VCI_CloseDevice(nDeviceType, nDeviceInd);
#endif
	mutex.lock();
	RecvBuff.clear();
	mutex.unlock();

	succeedopenandinit = false;
	devmutex.unlock();
	return true;
}

//ˢ�½��ܻ���֡�е����ݵ������
bool canif::refresh()
{
	devmutex.lock();

#ifdef HAVECANCARD
	if(succeedopenandinit == true)
	{

		DWORD dwRel = 0;
		ULONG lRet = 0;
		VCI_CAN_OBJ* vco;

		dwRel = VCI_GetReceiveNum(nDeviceType, nDeviceInd, nCANInd);

		if (dwRel >= 1)
		{

			vco = (VCI_CAN_OBJ* )malloc(sizeof(VCI_CAN_OBJ)*dwRel );

			//ȡ����
			lRet = VCI_Receive(nDeviceType, nDeviceInd, nCANInd, vco, dwRel, 400);

			if (lRet == 0xFFFFFFFF || lRet != dwRel)
			{
				publiccaution.adddbginfo("VCI_Receive ����");
				devmutex.unlock();
				return false;
			}

			//���ݷ����������
			for (unsigned int i = 0; i < dwRel; i++)
			{
				tcmd tempcmd;
				tempcmd.cmd = vco[i].Data[3];
				tempcmd.id = vco[i].Data[2];
				tempcmd.para1 = vco[i].Data[1];
				tempcmd.para2 = vco[i].Data[0];
				tempcmd.timecnt = 0;

				mutex.lock();
				RecvBuff.append(tempcmd);
				mutex.unlock();
			}
			free(vco);
		}

#endif
		devmutex.unlock();
		return true;
#ifdef HAVECANCARD
	}
	else
	{
		devmutex.unlock();
		return false;
	}
#endif

}

//����һ������
bool canif::sendcmd(unsigned long fid, tcmd* cmd)
{
	if(succeedopenandinit == true)
	{
		if((cmd->cmd == 0x55) 
			&& (cmd->para2 == 0x07))
		{
			return false;
		}
		VCI_CAN_OBJ vco;
		ZeroMemory(&vco, sizeof(VCI_CAN_OBJ));
		vco.ID = fid;
		vco.SendType = 0; //0�������ͣ��Է�������2
		vco.RemoteFlag = 0;
		vco.ExternFlag = 0;
		vco.DataLen = 4; //���ݳ���
		ULONG lRet;


		//����˳��һ�£��ߵ�����
		vco.Data[0] = cmd->para2;
		vco.Data[1] = cmd->para1;
		vco.Data[2] = cmd->id;
		vco.Data[3] = cmd->cmd;
#ifdef HAVECANCARD
		//��������
		devmutex.lock();
		lRet = VCI_Transmit(nDeviceType, nDeviceInd, nCANInd, &vco, 1);
		devmutex.unlock();

		if (lRet != 1)
		{
			publiccaution.adddbginfo(QString("ʧ�ܷ���CAN���� CANID��CMD��PARA1��PARA2��PARA3��") + QString::number(fid,16) + QString(",") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
			return false;
		}
#endif
		publiccaution.adddbginfo(QString("�ɹ�����CAN���� CANID��CMD��PARA1��PARA2��PARA3��") + QString::number(fid,16) + QString(",") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
		return true;
#ifdef HAVECANCARD
	}
	else
	{
		//publiccaution.adddbginfo(QString("ʧ�ܷ���CAN���� CANID��CMD��PARA1��PARA2��PARA3��") + QString::number(fid,16) + QString(",") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
		return false;
#endif
	}
}

//�鿴�������Ƿ��а���id����,����еĻ����Ͱ�����ȡ��cmd��ȥ
bool canif::getidcmd(unsigned char id, tcmd* cmd)
{

	mutex.lock();

#ifdef HAVECANCARD

	//����������Ұ���id
	QLinkedList<tcmd>::iterator i;
	QLinkedList<tcmd>::iterator j;
	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i + 1;
		if ((*i).id == id)
		{
			cmd->cmd = (*i).cmd;
			cmd->id = (*i).id;
			cmd->para1 = (*i).para1;
			cmd->para2 = (*i).para2;
			publiccaution.adddbginfo(QString("�ɹ�ȡ��CAN���� CMD��PARA1��PARA2��PARA3��") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
			RecvBuff.erase(i);
			mutex.unlock();
            return true;
		}
		i = j;
	}
#endif
	mutex.unlock();
	return false;
}

//�鿴�������Ƿ���fid�� cmd���������еĻ�����ȡ��cmd��ȥ
bool canif::getcmd(unsigned char id, unsigned char cmdid, tcmd* cmd)
{
	mutex.lock();

#ifdef HAVECANCARD

	//����������Ұ���id
	QLinkedList<tcmd>::iterator i;
	QLinkedList<tcmd>::iterator j;
	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i + 1;
		if ((*i).id == id && (*i).cmd == cmdid)
		{
			cmd->cmd = (*i).cmd;
			cmd->id = (*i).id;
			cmd->para1 = (*i).para1;
			cmd->para2 = (*i).para2;
			publiccaution.adddbginfo(QString("�ɹ�ȡ��CAN���� CMD��PARA1��PARA2��PARA3��") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
			RecvBuff.erase(i);
			mutex.unlock();
			return true;
		}
		i = j;
	}
#endif
	mutex.unlock();

	return false;
}

//�鿴�������Ƿ��а���id�� cmdid�������Ϊpara1,����еĻ�����ȡ��cmd��ȥ
bool canif::getcmd(unsigned char id, unsigned char cmdid, unsigned char para1,
				  tcmd* cmd)
{
	mutex.lock();

#ifdef HAVECANCARD

	//����������Ұ���id
	QLinkedList<tcmd>::iterator i;
	QLinkedList<tcmd>::iterator j;
	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i + 1;
		if ((*i).id == id && (*i).cmd == cmdid && (*i).para1 == para1)
		{

			cmd->cmd = (*i).cmd;
			cmd->id = (*i).id;
			cmd->para1 = (*i).para1;
			cmd->para2 = (*i).para2;
			publiccaution.adddbginfo(QString("�ɹ�ȡ��CAN���� CMD��PARA1��PARA2��PARA3��") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
			RecvBuff.erase(i);
			mutex.unlock();
			return true;
		}
		i = j;
	}
#endif
	mutex.unlock();

	return false;
}

//�鿴�������Ƿ��а���id�� cmdid�������Ϊpara1,para2,����еĻ�����ȡ��cmd��ȥ
bool canif::getcmd(unsigned char id, unsigned char cmdid, unsigned char para1,
				  unsigned char para2, tcmd* cmd)
{
	mutex.lock();

#ifdef HAVECANCARD

	//����������Ұ���id
	QLinkedList<tcmd>::iterator i;
	QLinkedList<tcmd>::iterator j;
	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i +1;
		if ((*i).id == id && (*i).cmd == cmdid && (*i).para1 == para1
			&& (*i).para2 == para2)
		{
			cmd->cmd = (*i).cmd;
			cmd->id = (*i).id;
			cmd->para1 = (*i).para1;
			cmd->para2 = (*i).para2;
			publiccaution.adddbginfo(QString("�ɹ�ȡ��CAN���� CMD��PARA1��PARA2��PARA3��") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
			RecvBuff.erase(i);
			mutex.unlock();
            return true;
		}
		i = j;
	}
#endif
	mutex.unlock();
	return false;
}

//�鿴�������Ƿ��а���id�� cmdid�������Ϊpara1,����еĻ�����ȡ��cmd��ȥ,����������
int canif::getcmd(unsigned char id, unsigned char cmdid, unsigned char para1,
				  QList<tcmd>& cmdlist)
{

	unsigned int count = 0;
	mutex.lock();

#ifdef HAVECANCARD

	//����������Ұ���id
	QLinkedList<tcmd>::iterator i ;
	QLinkedList<tcmd>::iterator j ;

	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i+1;
		if ((*i).id == id && (*i).cmd == cmdid && (*i).para1 == para1)
		{
			tcmd cmd;
			cmd.cmd = (*i).cmd;
			cmd.id = (*i).id;
			cmd.para1 = (*i).para1;
			cmd.para2 = (*i).para2;
			publiccaution.adddbginfo(QString("�ɹ�ȡ��CAN���� CMD��PARA1��PARA2��PARA3��") + QString::number(cmd.cmd,16) + QString(",")+ QString::number(cmd.id,16)+ QString(",") + QString::number(cmd.para1,16) + QString(",")+ QString::number(cmd.para2,16));
			RecvBuff.erase(i);
			cmdlist.append(cmd);
			count++;
		}
		i = j;
	}
#endif
	mutex.unlock();
	return count;
}

//�鿴�������Ƿ��а���id�� cmdid����,����еĻ�����ȡ��cmdlist��ȥ,����������
int canif::getcmd(unsigned char id,unsigned char cmdid,QList<tcmd>& cmdlist)
{
	unsigned int count = 0;
	mutex.lock();
#ifdef HAVECANCARD

	//����������Ұ���id
	QLinkedList<tcmd>::iterator i ;
	QLinkedList<tcmd>::iterator j ;

	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i+1;
		if ((*i).id == id && (*i).cmd == cmdid )
		{
			tcmd cmd;
			cmd.cmd = (*i).cmd;
			cmd.id = (*i).id;
			cmd.para1 = (*i).para1;
			cmd.para2 = (*i).para2;
			publiccaution.adddbginfo(QString("�ɹ�ȡ��CAN���� CMD��PARA1��PARA2��PARA3��") + QString::number(cmd.cmd,16) + QString(",")+ QString::number(cmd.id,16)+ QString(",") + QString::number(cmd.para1,16) + QString(",")+ QString::number(cmd.para2,16));
			cmdlist.append(cmd);
			RecvBuff.erase(i);
			count++;
		}
		i = j;
	}

#endif
	mutex.unlock();
    return count;
}

//�鿴�������Ƿ��а���id�� cmdid�������Ϊpara1,para2 ����еĻ�����ȡ��cmdlist��ȥ,����������
int canif::getcmd(unsigned char id, unsigned char cmdid, unsigned char para1,
				  unsigned char para2, QList<tcmd>& cmdlist)
{
	unsigned int count = 0;
	mutex.lock();
#ifdef HAVECANCARD

	//����������Ұ���id
	QLinkedList<tcmd>::iterator i ;
	QLinkedList<tcmd>::iterator j ;

	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i+1;
		if ((*i).id == id && (*i).cmd == cmdid && (*i).para1 == para1
			&& (*i).para2 == para2)
		{
			tcmd cmd;
			cmd.cmd = (*i).cmd;
			cmd.id = (*i).id;
			cmd.para1 = (*i).para1;
			cmd.para2 = (*i).para2;
			publiccaution.adddbginfo(QString("�ɹ�ȡ��CAN���� CMD��PARA1��PARA2��PARA3��") + QString::number(cmd.cmd,16) + QString(",")+ QString::number(cmd.id,16)+ QString(",") + QString::number(cmd.para1,16) + QString(",")+ QString::number(cmd.para2,16));
			RecvBuff.erase(i);
			cmdlist.append(cmd);
			count++;
		}
		i = j;
	}
#endif
	mutex.unlock();
    return count;
}



//ȡ�ý��ջ����е���������
int canif::getcmdcount()
{
	int count = 0;
	mutex.lock();
	count = RecvBuff.count();
	mutex.unlock();
	return count;
}

//��������е�����
int canif::clearcmdbuff()
{
	mutex.lock();
	RecvBuff.clear();
	mutex.unlock();
	return true;
}

//ˢ�»����е�ʱ������������������ʱ�Ļ�����ɾ������
void canif::refreshtime()
{
	mutex.lock();
#ifdef HAVECANCARD

	//����������Ұ���id
	QLinkedList<tcmd>::iterator i ;
	QLinkedList<tcmd>::iterator j ;

	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i+1;

		if ((*i).timecnt > CANOUTTIME)
		{
			publiccaution.adddbginfo(QString("ɾ��CAN���� CMD��PARA1��PARA2��PARA3��") + QString::number((*i).cmd,16) + QString(",")+ QString::number((*i).id,16)+ QString(",") + QString::number((*i).para1,16) + QString(",")+ QString::number((*i).para2,16));
			RecvBuff.erase(i);
		}
		else
		{
			(*i).timecnt ++;
		}

		i = j;
	}
#endif
	mutex.unlock();
}

//ȫ�ֱ�����ʼ��
void com::init()
{

	//�����б��ʼ��,������������ʼ����������ͷ����������ʱ����
	//bdlist.append(PCCANBDID);
	bdlist.append(BUMPIOBDID);
	bdlist.append(MAGICIOBDID);

	//������ͷ����������ͷ����
	for(int i = 0+ PRINT1BDID;i < pubcom.PrintNum + PRINT1BDID; i++)
	{
		bdlist.append(i);

		//��ͷѡ��
		QString paraname = QString("��ͷ") + QString::number(i+1) + QString("״̬");

		//��ͷ�Ƿ񼤻�
		QDomNode paranode = GetParaByName("print", paraname);
		bool ok;
		paranode.firstChildElement("value").text().toInt(&ok);
		if (paranode.firstChildElement("value").text().toInt(
			&ok) != 1)
		{
			//���õ�ǰ�������ͷ����
			activeprintlist.append(i);
		}
	}

	QList<int>::iterator ii;
	for(ii = bdlist.begin();ii != bdlist.end();++ ii)
	{
		nodeonlinecountmap.insert(*ii,0);
		nodedupidmap.insert(*ii,0);
	}
	for(int i = 0;i < 16; ++i)
	{
		zerostatus[i] = 1;
		MagicOnFlushStep[i] = 0;
		MagicOnFlushTimer[i] = 0;
	}

	//����canidӳ��
	bdnodemap.insert(PCCANBDID, PCNODEID);
	bdnodemap.insert(BUMPIOBDID, BUMPIONODEID);
	bdnodemap.insert(MAGICIOBDID, MAGICIONODEID);

	bdnodemap.insert(PRINT1BDID, PRINT1NODEID);
	bdnodemap.insert(PRINT2BDID, PRINT2NODEID);
	bdnodemap.insert(PRINT3BDID, PRINT3NODEID);
	bdnodemap.insert(PRINT4BDID, PRINT4NODEID);
	bdnodemap.insert(PRINT5BDID, PRINT5NODEID);
	bdnodemap.insert(PRINT6BDID, PRINT6NODEID);
	bdnodemap.insert(PRINT7BDID, PRINT7NODEID);
	bdnodemap.insert(PRINT8BDID, PRINT8NODEID);
	bdnodemap.insert(PRINT9BDID, PRINT9NODEID);
	bdnodemap.insert(PRINT10BDID, PRINT10NODEID);
	bdnodemap.insert(PRINT11BDID, PRINT11NODEID);
	bdnodemap.insert(PRINT12BDID, PRINT12NODEID);
	bdnodemap.insert(PRINT13BDID, PRINT13NODEID);
	bdnodemap.insert(PRINT14BDID, PRINT14NODEID);
	bdnodemap.insert(PRINT15BDID, PRINT15NODEID);
	bdnodemap.insert(PRINT16BDID, PRINT16NODEID);

	//�����������
	nodeonlinemap.insert(PCCANBDID, true);
	nodeonlinemap.insert(BUMPIOBDID, true);
	nodeonlinemap.insert(MAGICIOBDID, true);

	nodeonlinemap.insert(PRINT1BDID, true);
	nodeonlinemap.insert(PRINT2BDID, true);
	nodeonlinemap.insert(PRINT3BDID, true);
	nodeonlinemap.insert(PRINT4BDID, true);
	nodeonlinemap.insert(PRINT5BDID, true);
	nodeonlinemap.insert(PRINT6BDID, true);
	nodeonlinemap.insert(PRINT7BDID, true);
	nodeonlinemap.insert(PRINT8BDID, true);
	nodeonlinemap.insert(PRINT9BDID, true);
	nodeonlinemap.insert(PRINT10BDID, true);
	nodeonlinemap.insert(PRINT11BDID, true);
	nodeonlinemap.insert(PRINT12BDID, true);
	nodeonlinemap.insert(PRINT13BDID, true);
	nodeonlinemap.insert(PRINT14BDID, true);
	nodeonlinemap.insert(PRINT15BDID, true);
	nodeonlinemap.insert(PRINT16BDID, true);

	//����id�͹���λ��
	bdcautionmap.insert(PCCANBDID, CANCAUTIONLOCATION);
	bdcautionmap.insert(BUMPIOBDID, BUMPIOLOCATION);
	bdcautionmap.insert(MAGICIOBDID, MAGICIOLOCATION);

	bdcautionmap.insert(PRINT1BDID, PRINT1LOCATION);
	bdcautionmap.insert(PRINT2BDID, PRINT2LOCATION);
	bdcautionmap.insert(PRINT3BDID, PRINT3LOCATION);
	bdcautionmap.insert(PRINT4BDID, PRINT4LOCATION);
	bdcautionmap.insert(PRINT5BDID, PRINT5LOCATION);
	bdcautionmap.insert(PRINT6BDID, PRINT6LOCATION);
	bdcautionmap.insert(PRINT7BDID, PRINT7LOCATION);
	bdcautionmap.insert(PRINT8BDID, PRINT8LOCATION);
	bdcautionmap.insert(PRINT9BDID, PRINT9LOCATION);
	bdcautionmap.insert(PRINT10BDID, PRINT10LOCATION);
	bdcautionmap.insert(PRINT11BDID, PRINT11LOCATION);
	bdcautionmap.insert(PRINT12BDID, PRINT12LOCATION);
	bdcautionmap.insert(PRINT13BDID, PRINT13LOCATION);
	bdcautionmap.insert(PRINT14BDID, PRINT14LOCATION);
	bdcautionmap.insert(PRINT15BDID, PRINT15LOCATION);
	bdcautionmap.insert(PRINT16BDID, PRINT16LOCATION);

	//�����
	cmdlist.append(COMTESTCMD);//ͨѶ����
	cmdlist.append(VERTESTCMD);//�汾У��
	cmdlist.append(SETBUMPNOCMD);//���ý��ñ��
	cmdlist.append(SETPRINTPARACMD);//������ͷ����
	cmdlist.append(SETBUMPIONOCMD);//���ý���IO���
	cmdlist.append(SETMAGICIONOCMD);//��ƴ�̨IO���
	cmdlist.append(SETMAGICANANOCMD);//��ƴ�̨ģ�������
	cmdlist.append(CHGSPEEDCMD);//������������Ӧ��
	cmdlist.append(ZEROCMD);//��������Ӧ��
	cmdlist.append(PRINTWORKSTATUSCMD);//��ͷ����״̬����
	cmdlist.append(PRINTSWITCHSTATUSCMD);//��ͷ����״̬����
	cmdlist.append(PRINTXSTATUSCMD);//��ͷ����λ������
	cmdlist.append(PRINTYSTATUSCMD);//��ͷб��λ������
	cmdlist.append(PRINTZSTATUSCMD);//��ͷ�߶�λ������
	cmdlist.append(PRINTOPCMD);//��ͷ��������
	cmdlist.append(STARTWORKCMD);//������������
	cmdlist.append(QUERYSPEEDCMD);//��ѯ�����ٶ�����
	cmdlist.append(QUERYMACHSTATUSCMD);//��ѯ����״̬����
	cmdlist.append(QUERYXSTATUSCMD);//��ѯ���ػ����ֵ�Xλ��
	cmdlist.append(QUERYYSTATUSCMD);//��ѯ���ػ����ֵ�Yλ��
	cmdlist.append(QUERYZSTATUSCMD);//��ѯ���ػ����ֵ�Zλ��
	cmdlist.append(PRINTCAUTIONCMD);//��ͷ��������Ӧ��
	cmdlist.append(ZEROIOCAUTIONCMD);//����IO���������Ӧ��
	cmdlist.append(OUTIOCAUTIONCMD);//����IO���������Ӧ��
	cmdlist.append(BUMPIOCAUTIONCMD);//����IO���������Ӧ��
	cmdlist.append(MAGICIOCAUTIONCMD); //��̨IO���������
	cmdlist.append(ENMACHWORKCMD); //���������������
	cmdlist.append(CHGMACHSPEEDCMD);//��������������
	cmdlist.append(ERRSTOPCMD);//����������ͣ������
	cmdlist.append(CAUTIONCMD);//�������澯
	cmdlist.append(MACHWORKACKCMD);//������������Ӧ��
	cmdlist.append(MACHCAUTIONACKCMD);//��������Ӧ��
	cmdlist.append(MACHERRSTOPACKCMD);//��������Ӧ��
	cmdlist.append(MACHSTOPACKCMD);//����ͣ��Ӧ��
	cmdlist.append(MAGICFULLCMD);//��̨���������
	cmdlist.append(MAGICRSPCMD);//��̨״̬�ϱ��������Ӧ��
	cmdlist.append(BUMPRSPCMD);//����״̬�ϱ��������Ӧ��

	bdonlinestep.insert(BUMPIOBDID,0);
	bdonlinestep.insert(MAGICIOBDID,0);

	for(int i=PRINT1BDID;i <= PRINT16BDID;i++)
	{
		//�����Ƿ�������ͷ�Ĳ������������ʼ��
		uicmdtoprintmap.insert(i,NOCMD);
		//��������ͷ��������Ĳ����ʼ��
		uicmdtoprintstepmap.insert(i,0);
		//��������ͷ��������ļ�ʱ��
		uicmdtoprinttimermap.insert(i,0);

		QDomNode para;
		para = GetParaByName("print", QString("��ͷ") +  QString::number(i+1) + QString("״̬"));
		bool ok;
		presetprintstatus[i] = para.firstChildElement("value").text().toInt(&ok);

		printstatus[i] = presetprintstatus[i];

		//���㲽���ʼ��
		actionzerostep.insert(i,0);

		//����Ի�
		movedirectstatus.insert(i,0);
		movedirecttimer.insert(i,0);
		bdonlinestep.insert(i,0);
	}

	ismainuicmd = false;
	somemainuicmdfailed = false;

	isplccmd = false;
	someplccmdfailed = false;

	motorspeed = 0;
	MotorSpeedChgFlag = false;

	//�ٶ�����û����Ч��
	speedcmdstep = 0;
	speedcmdcount = 0;
	speedcmdtimer = 0;

	//������������
	heartbeatstep = 1;

	//���ý��水����ʶ
	applyflag = false;

	cardok = false;

	//�Ƿ�ˢ��Ӧ�ã�ȫ�������·�
	refreshapply = false;

	//����Ӧ���Ƿ��ڽ���
	isapply = false;

	//����Ӧ�õĲ���
	applystep = 0;

	//������Ӧ�õĶ�ʱ��
	applytimer = 0;

	printchgspeedansflag = false;
	printchgspeedstep = 0;

	singleprintchgspeedstep = 0;

	printchgspeedfailed = false;
	printchgspeedsucceed = false;

	printactiveflag = false;

	iocautonflag = 0;

	cautionlocationidstrmap.insert(PCCAUTIONLOCATION,QString("���ػ�"));
	cautionlocationidstrmap.insert(A4NCAUTIONLOCATION,QString("�˶���"));
	cautionlocationidstrmap.insert(CANCAUTIONLOCATION,QString("CAN"));
	cautionlocationidstrmap.insert(PLCLOCATION,QString("�ⲿ�豸"));
	//cautionlocationidstrmap.insert(OUTIOLOCATION,QString("����IO��"));
	//cautionlocationidstrmap.insert(ZEROIOLOCATION,QString("����IO��"));
	cautionlocationidstrmap.insert(MAGICIOLOCATION,QString("��̨IO��"));
	cautionlocationidstrmap.insert(BUMPIOLOCATION,QString("����IO��"));
	cautionlocationidstrmap.insert(PRINT1LOCATION,QString("��ͷ1"));
	cautionlocationidstrmap.insert(PRINT2LOCATION,QString("��ͷ2"));
	cautionlocationidstrmap.insert(PRINT3LOCATION,QString("��ͷ3"));
	cautionlocationidstrmap.insert(PRINT4LOCATION,QString("��ͷ4"));
	cautionlocationidstrmap.insert(PRINT5LOCATION,QString("��ͷ5"));
	cautionlocationidstrmap.insert(PRINT6LOCATION,QString("��ͷ6"));
	cautionlocationidstrmap.insert(PRINT7LOCATION,QString("��ͷ7"));
	cautionlocationidstrmap.insert(PRINT8LOCATION,QString("��ͷ8"));
	cautionlocationidstrmap.insert(PRINT9LOCATION,QString("��ͷ9"));
	cautionlocationidstrmap.insert(PRINT10LOCATION,QString("��ͷ10"));
	cautionlocationidstrmap.insert(PRINT11LOCATION,QString("��ͷ11"));
	cautionlocationidstrmap.insert(PRINT12LOCATION,QString("��ͷ12"));
	cautionlocationidstrmap.insert(PRINT13LOCATION,QString("��ͷ13"));
	cautionlocationidstrmap.insert(PRINT14LOCATION,QString("��ͷ14"));
	cautionlocationidstrmap.insert(PRINT15LOCATION,QString("��ͷ15"));
	cautionlocationidstrmap.insert(PRINT16LOCATION,QString("��ͷ16"));
	//cautionlocationidstrmap.insert(A4N,QString("A4Nϵͳ����"));
	//cautionlocationidstrmap.insert(CANNETWORK,QString("CAN����"));
	cautionlocationidstrmap.insert(SERIAL,QString("����"));

	cautionidstrmap.insert(CAUTION,QString("����"));
	cautionidstrmap.insert(COMCAUTION,QString("����"));
	cautionidstrmap.insert(VERCAUTION,QString("�汾��ƥ��"));
	cautionidstrmap.insert(PARCAUTION,QString("��������"));
	cautionidstrmap.insert(ZEROCAUTION,QString("�������"));
	cautionidstrmap.insert(PRINTBUMPCAUTION,QString("��ͷ���ù���"));
	cautionidstrmap.insert(PRINTMOTORCAUTION,QString("�ŷ�����"));
	cautionidstrmap.insert(PLCCAUTION,QString("����"));
	cautionidstrmap.insert(PLCHALTCATION,QString("��ͣ����"));
	cautionidstrmap.insert(PCNOA4NCAUTION,QString("û�а�װ�˶���"));
	cautionidstrmap.insert(PCNOCANCATION,QString("û�а�װCAN��"));
	cautionidstrmap.insert(PCSYSCATION,QString("PC��ϵͳ����"));
	cautionidstrmap.insert(PCERRCANCMD,QString("�����CAN����"));

	cautionidstrmap.insert(ZEROANSOUTTIME,QString("����IO��Ӧ��ʱ"));
	cautionidstrmap.insert(MAGICANSOUTTIME,QString("��̨���ư�Ӧ��ʱ"));
	cautionidstrmap.insert(BUMPANSOUTTIME,QString("���ÿ��ư�Ӧ��ʱ"));
	cautionidstrmap.insert(ZEROSIGNOUTTIME,QString("��λ�źų�ʱ"));
	cautionidstrmap.insert(XMOVMAX,QString("����Ի�����λ"));
	cautionidstrmap.insert(XMOVMIN,QString("����Ի�����λ"));
	cautionidstrmap.insert(YMOVMAX,QString("б��Ի�����λ"));
	cautionidstrmap.insert(YMOVMIN,QString("б��Ի�����λ"));
	cautionidstrmap.insert(XMOTORBOOTERR,QString("��������������"));
	cautionidstrmap.insert(YMOTORBOOTERR,QString("б������������"));
	cautionidstrmap.insert(EPRROMERR,QString("EEPROM��д����"));
	cautionidstrmap.insert(A4NANSOUTTIME,QString("�˶����ƿ�Ӧ��ʱ"));
	cautionidstrmap.insert(REPIDCAUTION,QString("�ظ��İ���CANID"));
	cautionidstrmap.insert(SERIALNOEXIST,QString("������"));
	cautionidstrmap.insert(SERIALLOST,QString("����"));
	cautionidstrmap.insert(CANNETCAUTION,QString("�������"));

	eventidstrmap.insert(SENDCMDFAILEVENT,QString("��������ʧ��"));
	eventidstrmap.insert(WAITANSOUTTIMEEVENT,QString("���ܳ�ʱ"));
	eventidstrmap.insert(BDONLINEEVENT,QString("�����¼�"));
	eventidstrmap.insert(BDOFFLINEEVENT,QString("�����¼�"));
	eventidstrmap.insert(UNKNOWCMDEVENT,QString("����ʶ����"));
	eventidstrmap.insert(TOOMUCHUNUSEDCMDEVENT,QString("����δ���������"));


	//����
	heartbeatstep = 0;
	heartbeattimer = 0;
	heartbeatcount = 0;

	//����
	plcreqstartstep = 0;
	plcreqstarttimer = 0;
	plcreqstartcount = 0;
	printok = false;

	//ֹͣ
	plcreqstopstep = 0;
	plcreqstopcount = 0;
	plcreqstoptimer = 0;

	//����������
	mainuicmdstep = 0;
	mainuicmdcount = 0;
	mainuidcmdtimer = 0;
	mainuidcmd = 0;

	//����PLC������
	plccmdstep = 0;
	plccmdcount = 0;
	plcdcmdtimer = 0;
	plcdcmd = 0;

	//������������
	opuicmdflag = 0;
	opuicmdstep = 0;
	opuicmdtimer = 0;
	opuicmdprintno = 0;
	opuicmdcmd = 0;
	opuicmdcount = 0;

	//�ָ���ͷλ�������ʼ��
	for(int i= 0;i<16;i++)
	{
		restoreTask[i].id = i;
		restoreTask[i].timer = 0;
		restoreTask[i].step = 0;
	}

	//������ͷ��ǰλ�������ʼ��
	for(int i= 0;i<16;i++)
	{
		savelocTask[i].id = i;
		savelocTask[i].timer = 0;
		savelocTask[i].step = 0;
	}

	//��ͷ����
	printchgspeedstep = 0;
	printchgspeedcount = 0;
	printchgspeedtimer = 0;

	enuichgspdbt = false;

	statuschgstep = 0;

	//IO�˿��ϱ�����״̬
	IOCautionStatus = false;
	IOCautionStatusstep = 0;
	IOCautionStatuscount = 0;
	IOCautionStatustimer = 0;

	//IO�˿��ϱ�����״̬
	IOErrStatus = false;
	IOErrStatusstep = 0;
	IOErrStatuscount = 0;
	IOErrStatustimer = 0;

	//IO�˿��ϱ���ͣ״̬
	IOHaltStatus = false;
	IOHaltStatusstep = 0;
	IOHaltStatuscount = 0;
	IOHaltStatustimer = 0;

	//ˢ����������
	FlushDataReqStep = 0;
	FlushDataReqTimer = 0;

	quitthread = false;

	PowerOff = false;
	
	QDomNode para;
	para = GetParaByName("prod", "�趨ӡ������ͣ��");
	bool ok;
	NeedStopAtWorkfinished = para.firstChildElement("value").text().toInt(&ok);

	//���㴫�����źſ��
	para = GetParaByName("dev", "���㴫�����źſ��");
	ZeroDetectWidth = para.firstChildElement("value").text().toInt(&ok);

	//������ٶο��
	para = GetParaByName("dev", "������ٶο��");
	ZeroHighSpeedWidth = para.firstChildElement("value").text().toInt(&ok);

	//������ٶο��
	para = GetParaByName("dev", "������ٶο��");
	ZeroLowSpeedWidth = para.firstChildElement("value").text().toInt(&ok);

	//�Ƿ��¼����
	QString paraname = QString("��¼����");

	QDomNode paranode = GetParaByName("system", paraname);
	paranode.firstChildElement("value").text().toInt(&ok);
	if (paranode.firstChildElement("value").text().toInt(
		&ok) == 1)
	{
		bloprecflag = true;
	}
	else
	{
		bloprecflag = false;
	}

	//�Ƿ��¼�쳣
	paraname = QString("��¼�쳣");
	paranode = GetParaByName("system", paraname);
	paranode.firstChildElement("value").text().toInt(&ok);
	if (paranode.firstChildElement("value").text().toInt(
		&ok) == 1)
	{
		blexceptrecflag = true;
	}
	else
	{
		blexceptrecflag = false;
	}

	//�Ƿ��¼������Ϣ
	paraname = QString("�Ƿ��¼������Ϣ");
	paranode = GetParaByName("dev", paraname);
	paranode.firstChildElement("value").text().toInt(&ok);
	if (paranode.firstChildElement("value").text().toInt(
		&ok) == 1)
	{
		bldbginforecflag = true;
	}
	else
	{
		bldbginforecflag = false;
	}

	//�������߱�ʶ
	OfflineFlag = false;

	//����������
	PlcStartUpFlag = false;

	//ֹͣ������
	PlcStopUpFlag = false;

	//����������
	PlcErrUpFlag = false;

	//�����½���
	PlcErrDownFlag = false;

	//�澯������
	PlcCauUpFlag = false;

	//�澯�½���
	PlcCauDownFlag = false;

	//��������������
	PlcJinbuyunxinUpFlag = false;

	//���������½���
	PlcJinbuyunxinDownFlag = false;

	//ӡ����������������
	PlcYinhuadaodaiyunxinUpFlag = false;

	//ӡ�����������½���
	PlcYinhuadaodaiyunxinDownFlag = false;

	//�淿����1����������
	PlcHongfangwangdai1yunxinUpFlag = false;

	//�淿����1�����½���
	PlcHongfangwangdai1yunxinDownFlag = false;

	//�淿����2����������
	PlcHongfangwangdai2yunxinUpFlag = false;

	//�淿����2�����½���
	PlcHongfangwangdai2yunxinDownFlag = false;

	//�䲼����������
	PlcLuobuyunxinUpFlag = false;

	//�䲼�����½���
	PlcLuobuyunxinDownFlag = false;

	//������������������
	PlcGongjiangcilikaiguanUpFlag = false;

	//�������������½���
	PlcGongjiangcilikaiguanDownFlag = false;

	//������С�ı��ʶ
	PlcGongjiangcilidaxiaogaibianFlag = false;

	Shangjiangcitaistep = 0;
	Shangjiangcitaicount = 0;
	Shangjiangcitaitimer = 0;
	Shangjiangcitaiflag = false;

	iHelpViewLocation = 0;
	iHelpPageLocation = 0;
	bHelpCrossPage = false;
}

com::com()
{

}

com::~com()
{

}

bool com::chkver(unsigned char para1, unsigned char para2)
{
	if (para2 == 0x81)
		return true;
	else
		return false;
}


//���ý����Ƿ�������ͷ�������������
int com::setcmdtoprint(int bdid,bool flag)
{
	uicmdtoprintmap.insert(bdid,flag);
	return true;
}

//�Ƿ�������
bool com::iscmd(unsigned char cmd)
{
	return cmdlist.contains(cmd);
}

//�Ƿ��а�������
bool com::hasbdonline()
{
	QMap<int,int>::iterator it;
	for (it = bdonlinestep.begin();it != bdonlinestep.end(); ++it)
	{
		if (it.value() > 1)
		{
			return true;
		}
	}
	return false;
}

//�Ĵ�����ʼ���ӿ�
void initRegs(struct _regs* regs)
{
	//��0
	memset(regs->readarray,0,100);
	memset(regs->hisreadarray,0,100);
	memset(regs->writearray,0,100);
	regs->heartbeatcnt = 0;
}

//regs���modbus�Ľӿ�
void ReadRegs(struct _regs* regs,unsigned int index,unsigned char* ch)
{
	*ch = regs->writearray[index];
}

//[0]�ֽڣ����͸�PC�ĸ�8λIO,[1]�ֽڣ����͸�PC�ĵ�8λIO,[2-3]�����͸�PC�Ĵ�̨���� [5]:���͸�PC���µ�8λIO [4]:���͸�PC���¸�8λIO
void WriteRegs(struct _regs* regs,unsigned int index,unsigned char ch)
{
	regs->readarray[index] = ch;

	//�Ƿ�IO�ı䣬IO�ı�͵���Hook����
	if (index == 0)
	{
		if ( ((regs->hisreadarray[index] >> (regs->PlcLuobuyunxinBitIndex-8)) & 0x01) == 0x00
			&& ((regs->readarray[index] >> (regs->PlcLuobuyunxinBitIndex-8)) & 0x01) == 0x01 )
		{
			//������
			regs->PlcLuobuyunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> (regs->PlcLuobuyunxinBitIndex-8)) & 0x01) == 0x01
			&& ((regs->readarray[index] >> (regs->PlcLuobuyunxinBitIndex-8)) & 0x01) == 0x00 )
		{
			//�½���
			regs->PlcLuobuyunxinFp(false);
		}
		if ( ((regs->hisreadarray[index] >> (regs->PlcGongjiangcilikaiguanBitIndex-8)) & 0x01) == 0x00
			&& ((regs->readarray[index] >> (regs->PlcGongjiangcilikaiguanBitIndex-8)) & 0x01) == 0x01 )
		{
			//������
			regs->PlcGongjiangcilikaiguanFp(true);
		}
		if ( ((regs->hisreadarray[index] >> (regs->PlcGongjiangcilikaiguanBitIndex-8)) & 0x01) == 0x01
			&& ((regs->readarray[index] >> (regs->PlcGongjiangcilikaiguanBitIndex-8)) & 0x01) == 0x00 )
		{
			//�½���
			regs->PlcGongjiangcilikaiguanFp(false);
		}
	}
	else if (index == 1)
	{
		if ( ((regs->hisreadarray[index] >> regs->PlcStartReqBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcStartReqBitIndex) & 0x01) == 0x01 )
		{
			//������
			regs->PlcStartReqFp(true);
			printf("start ������\n");
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcStopBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcStopBitIndex) & 0x01) == 0x01 )
		{
			//������
			regs->PlcStopFp(true);
			printf("stop ������\n");
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}

		if ( ((regs->hisreadarray[index] >> regs->PlcErrBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcErrBitIndex) & 0x01) == 0x01 )
		{
			//������
			regs->PlcErrFp(true);
			printf("err ������\n");
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcErrBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcErrBitIndex) & 0x01) == 0x00 )
		{
			//�½���
			regs->PlcErrFp(false);
			printf("err �½���\n");
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcCauBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcCauBitIndex) & 0x01) == 0x01 )
		{
			//������
			regs->PlcCauFp(true);
			printf("cau ������\n");
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcCauBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcCauBitIndex) & 0x01) == 0x00 )
		{
			//�½���
			regs->PlcCauFp(false);
			printf("cau �½���\n");
		}

		if ( ((regs->hisreadarray[index] >> regs->PlcJinbuyunxinBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcJinbuyunxinBitIndex) & 0x01) == 0x01 )
		{
			//������
			regs->PlcJinbuyunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcJinbuyunxinBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcJinbuyunxinBitIndex) & 0x01) == 0x00 )
		{
			//�½���
			regs->PlcJinbuyunxinFp(false);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcYinhuadaodaiyunxinBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcYinhuadaodaiyunxinBitIndex) & 0x01) == 0x01 )
		{
			//������
			regs->PlcYinhuadaodaiyunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcYinhuadaodaiyunxinBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcYinhuadaodaiyunxinBitIndex) & 0x01) == 0x00 )
		{
			//�½���
			regs->PlcYinhuadaodaiyunxinFp(false);
		}

		if ( ((regs->hisreadarray[index] >> regs->PlcHongfangwangdai1yunxinBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcHongfangwangdai1yunxinBitIndex) & 0x01) == 0x01 )
		{
			//������
			regs->PlcHongfangwangdai1yunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcHongfangwangdai1yunxinBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcHongfangwangdai1yunxinBitIndex) & 0x01) == 0x00 )
		{
			//�½���
			regs->PlcHongfangwangdai1yunxinFp(false);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcHongfangwangdai2yunxinBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcHongfangwangdai2yunxinBitIndex) & 0x01) == 0x01 )
		{
			//������
			regs->PlcHongfangwangdai2yunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcHongfangwangdai2yunxinBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcHongfangwangdai2yunxinBitIndex) & 0x01) == 0x00 )
		{
			//�½���
			regs->PlcHongfangwangdai2yunxinFp(false);
		}
	}//������С�ı�([3]�ֽڵ�8λ [2]�ֽڸ�8λ)�������̨�����ı�(��̨�����ٷֱ�)���͸�ÿ����ͷ
	else if(index == 3)
	{
		if((regs->hisreadarray[index] != regs->readarray[index]) )
		{
			//��̨�����޸�
			//interfaceevent* ptempevent = new interfaceevent();

			//ptempevent->cmd = 0x12;//��ͷ��������
			//ptempevent->status = 0x00;//����״̬
			//ptempevent->data[0] = 0x58;//�����޸�
			//ptempevent->data[1] = regs->readarray[index];//������С

			//����һ���¼�����̨�߳�
			//QCoreApplication::postEvent(backendobject, ptempevent);
			//printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);

			//������С�ı��ʶ��������
            //regs->PlcGongjiangcilidaxiaogaibianFp(true,regs->readarray[index]);
		}
	}
	else if(index == 5)
	{
		if ( ((regs->hisreadarray[index] >> 0) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 0) & 0x01) == 0x01 )
		{
			//��ͷ�·�������
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x04;//С�����֣��·Ż�����̧

			//����һ���¼�����̨�߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> 1) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 1) & 0x01) == 0x01 )
		{
			//��ͷ̧��������
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x03;//С�����֣��·Ż�����̧

			//����һ���¼�����̨�߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> 2) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 2) & 0x01) == 0x01 )
		{
			//��̨�ϴ�������
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x07;//��̨����

			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> 3) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 3) & 0x01) == 0x01 )
		{
			//��̨ȥ��������
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x08;//��̨�ر�

			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> 4) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 4) & 0x01) == 0x01 )
		{
			//��ͷ����������
			//����״̬����ѹǷѹ����ͣ��������������
			if ((pubcom.machprintstatus != 2) && (publiccaution.hascaution(PLCLOCATION, PLCHALTCATION) == false) )
			{
				interfaceevent* ptempevent = new interfaceevent();

				ptempevent->cmd = 0x12;//��ͷ��������
				ptempevent->status = 0x00;//����״̬
				ptempevent->data[0] = 0x53;//����

				//����һ���¼�����̨�߳�
				QCoreApplication::postEvent(backendobject, ptempevent);
				printf("plc����\n");

			}
		}
		if ( ((regs->hisreadarray[index] >> 5) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 5) & 0x01) == 0x01 )
		{
			//��ͷ��ת������
			//����״̬����������ת��ֹͣ��ת���ⲿ��ͣ����״̬������������ת
			if ((pubcom.machprintstatus != 2) && ((publiccaution.hascaution(PLCLOCATION,PLCHALTCATION) == false) || ((ptech->circleflag) == true)) )
			{
				bool cancircle = true;
				QList<int>::iterator it;

				//���̱߳���
				pubcom.activeprintlistmutex.lockForRead();
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{

					//�����в����Է�ת
					if (pubcom.actionzerostep.value(*it) != 0)
					{
						cancircle = false;
						break;
					}
				}
				pubcom.activeprintlistmutex.unlock();

				if(cancircle == true)
				{
					interfaceevent* ptempevent = new interfaceevent();

					ptempevent->cmd = 0x12;//��ͷ��������
					ptempevent->status = 0x00;//����״̬
					ptempevent->data[0] = 0x54;//��ת

					//����һ���¼��������߳�
					QCoreApplication::postEvent(backendobject, ptempevent);
					ptech->circleflag = true;
				}
			}
		}
		if ( ((regs->hisreadarray[index] >> 6) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 6) & 0x01) == 0x01 )
		{
			//��������������
			{
				ptech->curworknumber = 0;
				ptech->sumcount = 0;
				mydb.SaveValue("curworknumber",ptech->curworknumber);
			}
		}
		if ( ((regs->hisreadarray[index] >> 5) & 0x01) == 0x01
			&& ((regs->readarray[index] >> 5) & 0x01) == 0x00 )
		{
			//��ͷ��ת�½���
			//����״̬��������ת
			if (pubcom.machprintstatus != 2)
			{
				bool cancircle = true;
				QList<int>::iterator it;

				//���̱߳���
				pubcom.activeprintlistmutex.lockForRead();
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{

					//�����в����Է�ת
					if (pubcom.actionzerostep.value(*it) != 0)
					{
						cancircle = false;
						break;
					}
				}
				pubcom.activeprintlistmutex.unlock();

				if(cancircle == true)
				{
					interfaceevent* ptempevent = new interfaceevent();

					ptempevent->cmd = 0x12;//��ͷ��������
					ptempevent->status = 0x00;//����״̬
					ptempevent->data[0] = 0x54;//��ת

					//����һ���¼��������߳�
					QCoreApplication::postEvent(backendobject, ptempevent);
					ptech->circleflag = false;
				}
			}
		}
	}
	else if(index == 4)
	{
		if ( ((regs->hisreadarray[index] >> 1) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 1) & 0x01) == 0x01 )
		{
			//���ÿ���
			printf("���ÿ���\n");
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x59;//�Զ�����

			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
		if ( ((regs->hisreadarray[index] >> 2) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 2) & 0x01) == 0x01 )
		{
			//����ֹͣ
			printf("����ֹͣ\n");
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x60;//�Զ��ر�

			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}

	else if((index == 6) 
		|| (index == 7))//��������
	{
		bool chgflag = false;
		if(regs->hisreadarray[6] != regs->readarray[6] )
		{
			regs->hisreadarray[6] = regs->readarray[6];
			chgflag = true;
		}
		if(regs->hisreadarray[7] != regs->readarray[7] )
		{
			regs->hisreadarray[7] = regs->readarray[7];
			chgflag = true;
		}
		if(chgflag == true)
		{
			ptech->setworknumber = (((unsigned short)regs->readarray[6])<< 8) +  regs->readarray[7];
			mydb.SaveValue("setworknumber",ptech->setworknumber);
		}
	}
	else if((index == 8) 
		|| (index == 9))//�ٲ�ٷֱ�
	{
		bool chgflag = false;
		if(regs->hisreadarray[8] != regs->readarray[8] )
		{
			regs->hisreadarray[8] = regs->readarray[8];
			chgflag = true;
		}
		if(regs->hisreadarray[9] != regs->readarray[9] )
		{
			regs->hisreadarray[9] = regs->readarray[9];
			chgflag = true;
		}

		short bilv = (((short)regs->readarray[8])<< 8) +  regs->readarray[9];
		if((f_follow) != ((double)bilv/100000))
		{
			pubcom.Follow = (double)bilv/100000;
			chgflag = true;
		}
	}
	regs->hisreadarray[index] = regs->readarray[index];
}

//���ػ���������λλ��
void SetPcEnableStartBitIndex(struct _regs* regs,int index)
{
	regs->PcEnableStartBitIndex = index;
}

//���ػ���������λλ��
void SetPcMotorAccBitIndex(struct _regs* regs,int index)
{
	regs->PcMotorAccBitIndex = index;
}

//���ػ���������λλ��
void SetPcMotorDecBitIndex(struct _regs* regs,int index)
{
	regs->PcMotorDecBitIndex = index;
}

//���ػ�����λλ��
void SetPcErrBitIndex(struct _regs* regs,int index)
{
	regs->PcErrBitIndex = index;
}

//���ػ�����λλ��
void SetPcCauBitIndex(struct _regs* regs,int index)
{
	regs->PcCauBitIndex = index;
}

//���ػ�����̺����λλ��
void SetPcMotorOnBitIndex(struct _regs* regs,int index)
{
	regs->PcMotorOnBitIndex = index;
}

//���ػ���ֹ̺ͣλλ��
void SetPcMotorOffBitIndex(struct _regs* regs,int index)
{
	regs->PcMotorOffBitIndex = index;
}

//���ػ���������λλ��
void SetPcWorkFinBitIndex(struct _regs* regs,int index)
{
	regs->PcWorkFinBitIndex = index;
}

//���ػ���������λλ��
void SetPcSetHeartBeatBitIndex(struct _regs* regs,int index)
{
	regs->PcSetHeartBeatBitIndex = index;
}

//���ػ�����ˢ������λλ��
void SetPcFlushDataReqBitIndex(struct _regs* regs,int index)
{
	regs->PcFlushDataReqBitIndex = index;
}

//PLC������������λλ��
void SetPlcStartReqBitIndex(struct _regs* regs,int index)
{
	regs->PlcStartReqBitIndex = index;
}

//PLC����ֹͣλλ��
void SetPlcStopBitIndex(struct _regs* regs,int index)
{
	regs->PlcStopBitIndex = index;
}

//PLC����λλ��
void SetPlcErrBitIndex(struct _regs* regs,int index)
{
	regs->PlcErrBitIndex = index;
}

//PLC����λλ��
void SetPlcCauBitIndex(struct _regs* regs,int index)
{
	regs->PlcCauBitIndex = index;
}

//PLC��������λλ��
void SetPlcJinbuyunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcJinbuyunxinBitIndex = index;
}

//PLCӡ����������λλ��
void SetPlcYinhuadaodaiyunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcYinhuadaodaiyunxinBitIndex = index;
}

//PLC�淿����1����λλ��
void SetPlcHongfangwangdai1yunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcHongfangwangdai1yunxinBitIndex = index;
}

//PLC�淿����2����λλ��
void SetPlcHongfangwangdai2yunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcHongfangwangdai2yunxinBitIndex = index;
}

//PLC�䲼����λλ��
void SetPlcLuobuyunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcLuobuyunxinBitIndex = index;
}

//PLC������������λλ��
void SetPlcGongjiangcilikaiguanBitIndex(struct _regs* regs,int index)
{
	regs->PlcGongjiangcilikaiguanBitIndex = index;
}


//���ػ����IO��λ��
void SetPcOutIOWordIndex(struct _regs* regs,int index)
{
	regs->PcOutIOWordIndex = index;
}

//���ػ�����˫��λ��
void SetPcWorkRecDWordIndex(struct _regs* regs,int index)
{
	regs->PcWorkRecDWordIndex = index;
}


//��������IO������ָ�루�ص��ӿڣ�
void SetPlcStartReqFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcStartReqFp = fp;
}
void SetPlcStopFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcStopFp = fp;
}
void SetPlcErrFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcErrFp = fp;
}
void SetPlcCauFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcCauFp = fp;
}

void SetPlcJinbuyunxinFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcJinbuyunxinFp = fp;
}
void SetPlcYinhuadaodaiyunxinFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcYinhuadaodaiyunxinFp = fp;
}
void SetPlcHongfangwangdai1yunxinFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcHongfangwangdai1yunxinFp = fp;
}
void SetPlcHongfangwangdai2yunxinFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcHongfangwangdai2yunxinFp = fp;
}
void SetPlcLuobuyunxinFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcLuobuyunxinFp = fp;
}
void SetPlcGongjiangcilikaiguanFp(struct _regs* regs,void (*fp)(bool))
{
	regs->PlcGongjiangcilikaiguanFp = fp;
}

void SetPlcGongjiangcilidaxiaogaibianFp(struct _regs* regs,void (*fp)(bool,unsigned char))
{
	regs->PlcGongjiangcilidaxiaogaibianFp = fp;
}

//��λ�ڲ��������Ƕ���ӿ�
void SetWriteRegsBit(struct _regs* regs,bool bit,int index)
{

	if (index >= 8)
	{
		if (bit == true)
		{
			regs->writearray[regs->PcOutIOWordIndex*2] = regs->writearray[regs->PcOutIOWordIndex*2] | (0x01 << (index-8));
		} 
		else
		{
			regs->writearray[regs->PcOutIOWordIndex*2] = regs->writearray[regs->PcOutIOWordIndex*2] & (~(0x01 << (index-8)));
		}
	}
	else
	{
		if (bit == true)
		{
			regs->writearray[regs->PcOutIOWordIndex*2 + 1] = regs->writearray[regs->PcOutIOWordIndex*2 + 1] | (0x01 << (index));
		} 
		else
		{
			regs->writearray[regs->PcOutIOWordIndex*2 + 1] = regs->writearray[regs->PcOutIOWordIndex*2 + 1] & (~(0x01 << (index)));
		}
	}
}

//��������
void WritePcEnableStartBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PlcStartReqBitIndex);
}

//��������
void WritePcMotorAccBit(struct _regs* regs,bool bit)
{

	SetWriteRegsBit(regs,bit,regs->PcMotorAccBitIndex);

}

//��������
void WritePcMotorDecBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcMotorDecBitIndex);

}

//���ػ�����
void WritePcErrBit(struct _regs* regs,bool bit)
{

	SetWriteRegsBit(regs,bit,regs->PcErrBitIndex);

}

//���ػ�����
void WritePcCauBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcCauBitIndex);

}

//�������
void WritePcMotorOnBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcMotorOnBitIndex);
	//printf("IO:%X",regs->writearray[regs->PcOutIOWordIndex*2 + 1]);

}

//���ֹͣ
void WritePcMotorOffBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcMotorOffBitIndex);
	//printf("IO:%X",regs->writearray[regs->PcOutIOWordIndex*2 + 1]);
}

//��������
void WritePcWorkFinBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcWorkFinBitIndex);

}

//д����λ
void WritePcSetHeartBeatBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcSetHeartBeatBitIndex);
}

//дˢ����������
void WritePcFlushDataReqBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcFlushDataReqBitIndex);

}

//д����ʱ��̨δ�����澯
void WriteMagicCauBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,10);
}

//д������
void WritePcWorkRecDWord(struct _regs* regs,int word)
{
	regs->writearray[regs->PcWorkRecDWordIndex * 2 ] = (unsigned char)(word >> 8);
	regs->writearray[regs->PcWorkRecDWordIndex * 2 + 1] = (unsigned char)(word);
	regs->writearray[regs->PcWorkRecDWordIndex * 2 + 2] = (unsigned char)(word >> 24);
	regs->writearray[regs->PcWorkRecDWordIndex * 2 + 3] = (unsigned char)(word>>16);
}


//ˢ�¶����������Ĵ���
void FlushRegsReadHeatbeat(struct _regs* regs)
{
	if((regs->heartbeatcnt) < 20)
	(regs->heartbeatcnt)++;
}

//����������������ֵ
void ReadRegsReadHeatbeat(struct _regs* regs,unsigned char* pch)
{
	*pch = regs->heartbeatcnt;
}


//����IO������
//��������
void PlcStartReqHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("start OK!\n");
		pubcom.PlcStartUpFlag = flag;
	}
}

//ֹͣ
void PlcStopHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("stop OK!\n");
		pubcom.PlcStopUpFlag = flag;
	}
}
//����
void PlcErrHook(bool flag)
{
	if (flag == true)
	{
		pubcom.PlcErrUpFlag = true;
	} 
	else
	{
		pubcom.PlcErrDownFlag = true;
	}
}
//�澯
void PlcCauHook(bool flag)
{
	if (flag == true)
	{
		pubcom.PlcCauUpFlag = true;
	} 
	else
	{
		pubcom.PlcCauDownFlag = true;
	}
}

//��������hook
void PlcJinbuyunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("��������\n");
		pubcom.PlcJinbuyunxinUpFlag = true;
		pubcom.PlcJinbuyunxinDownFlag = false;
	} 
	else
	{
		Dbgout("����ֹͣ\n");
		pubcom.PlcJinbuyunxinDownFlag = true;
		pubcom.PlcJinbuyunxinUpFlag = false;
	}
}

//ӡ����������hook
void PlcYinhuadaodaiyunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("ӡ����������\n");
		pubcom.PlcYinhuadaodaiyunxinUpFlag = true;
		pubcom.PlcYinhuadaodaiyunxinDownFlag = false;
	} 
	else
	{
		Dbgout("ӡ������ֹͣ\n");
		pubcom.PlcYinhuadaodaiyunxinDownFlag = true;
		pubcom.PlcYinhuadaodaiyunxinUpFlag = false;
	}
}

//�淿����1����hook
void PlcHongfangwangdai1yunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("�淿����1����\n");
		pubcom.PlcHongfangwangdai1yunxinUpFlag = true;
		pubcom.PlcHongfangwangdai1yunxinDownFlag = false;
	} 
	else
	{
		Dbgout("�淿����1ֹͣ\n");
		pubcom.PlcHongfangwangdai1yunxinDownFlag = true;
		pubcom.PlcHongfangwangdai1yunxinUpFlag = false;
	}
}

//�淿����2����hook
void PlcHongfangwangdai2yunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("�淿����2����\n");
		pubcom.PlcHongfangwangdai2yunxinUpFlag = true;
		pubcom.PlcHongfangwangdai2yunxinDownFlag = false;
	} 
	else
	{
		Dbgout("�淿����2ֹͣ\n");
		pubcom.PlcHongfangwangdai2yunxinDownFlag = true;
		pubcom.PlcHongfangwangdai2yunxinUpFlag = false;
	}
}

//�䲼����hook
void PlcLuobuyunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("�䲼����\n");
		pubcom.PlcLuobuyunxinUpFlag = true;
		pubcom.PlcLuobuyunxinDownFlag = false;
	} 
	else
	{
		Dbgout("�䲼ֹͣ\n");
		pubcom.PlcLuobuyunxinDownFlag = true;
		pubcom.PlcLuobuyunxinUpFlag = false;
	}
}

//������������hook
void PlcGongjiangcilikaiguanHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("����������\n");
		pubcom.PlcGongjiangcilikaiguanUpFlag = true;
	} 
	else
	{
		Dbgout("����������\n");
		pubcom.PlcGongjiangcilikaiguanDownFlag = true;
	}
}

//����������С�ı�hook
void PlcGongjiangcilidaxiaogaibianHook(bool flag,unsigned char percent)
{
	pubcom.PlcGongjiangcilidaxiao = percent;
	pubcom.PlcGongjiangcilidaxiaogaibianFlag = true;
}




