/*
* canif.cpp
* can设备接口的封装
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

//数据库
extern DB mydb;

extern tech* ptech;
//A4N电机
extern CKSMCA4 *ksmc;

//通讯对象
com pubcom;
int GetParaRow(const QString& groupname);

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);
extern double f_follow;

canif::canif()
{
	succeedopenandinit = false;
	nDeviceType = VCI_PCI9820; //设备类型
	nDeviceInd = 0; //设备索引，安装有多个can卡时的识别只用
	nReserved = 0; //对于PCI9820保留，无意义
	nCANInd = 0; //can卡上面的端口号
}

canif::~canif()
{
	close();
}

bool canif::open()
{

	devmutex.lock();

	//取得参数
	bool ok;
	QDomNode para = GetParaByName("dev", "can速率");
	speed = para.firstChildElement("value").text().toInt(&ok);

	para = GetParaByName("dev", "can口选择");
	nCANInd = para.firstChildElement("value").text().toInt(&ok);


	VCI_INIT_CONFIG vic; //初始化数据结构
	DWORD dwRel;


	vic.AccCode = PCNODEID;
	vic.AccMask = PCRECVMASK;
	//vic.AccCode = 0x00000000;
	//vic.AccMask = 0xffffffff;

	vic.Filter = 0;

	//根据速率设定定时器
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

	//打开CAN卡
	dwRel = VCI_OpenDevice(nDeviceType, nDeviceInd, nReserved);

	if (dwRel != STATUS_OK)
	{
		publiccaution.addevent("CAN","CAN通讯卡打开失败","CAN通讯卡打开失败");
		devmutex.unlock();
		return false;
	}
	else
	{
	}

#endif

#ifdef HAVECANCARD
	//初始化CAN卡
	dwRel = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd, &vic);
	if (dwRel != STATUS_OK)
	{
		publiccaution.addevent("CAN","CAN卡初始化失败","CAN卡初始化失败");
		devmutex.unlock();
    	return false;
	}
	else
	{
	}
#endif

#ifdef HAVECANCARD
	//启动CAN卡
	dwRel = VCI_StartCAN(nDeviceType, nDeviceInd, nCANInd);
	if (dwRel != STATUS_OK)
	{
		VCI_ERR_INFO pErrInfo;
		dwRel = VCI_ReadErrInfo(nDeviceType, nDeviceInd, nCANInd, &pErrInfo);
		if (dwRel != STATUS_OK)
		{
			publiccaution.addevent("CAN","CAN卡启动失败","CAN卡启动失败");
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

//刷新接受缓存帧中的数据到命令缓存
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

			//取数据
			lRet = VCI_Receive(nDeviceType, nDeviceInd, nCANInd, vco, dwRel, 400);

			if (lRet == 0xFFFFFFFF || lRet != dwRel)
			{
				publiccaution.adddbginfo("VCI_Receive 错误");
				devmutex.unlock();
				return false;
			}

			//数据放在命令缓存中
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

//发送一个命令
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
		vco.SendType = 0; //0正常发送，自发自收是2
		vco.RemoteFlag = 0;
		vco.ExternFlag = 0;
		vco.DataLen = 4; //数据长度
		ULONG lRet;


		//数组顺序不一致，颠倒过来
		vco.Data[0] = cmd->para2;
		vco.Data[1] = cmd->para1;
		vco.Data[2] = cmd->id;
		vco.Data[3] = cmd->cmd;
#ifdef HAVECANCARD
		//发送数据
		devmutex.lock();
		lRet = VCI_Transmit(nDeviceType, nDeviceInd, nCANInd, &vco, 1);
		devmutex.unlock();

		if (lRet != 1)
		{
			publiccaution.adddbginfo(QString("失败发送CAN命令 CANID，CMD，PARA1，PARA2，PARA3：") + QString::number(fid,16) + QString(",") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
			return false;
		}
#endif
		publiccaution.adddbginfo(QString("成功发送CAN命令 CANID，CMD，PARA1，PARA2，PARA3：") + QString::number(fid,16) + QString(",") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
		return true;
#ifdef HAVECANCARD
	}
	else
	{
		//publiccaution.adddbginfo(QString("失败发送CAN命令 CANID，CMD，PARA1，PARA2，PARA3：") + QString::number(fid,16) + QString(",") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
		return false;
#endif
	}
}

//查看缓存中是否有板子id命令,如果有的话，就把命令取到cmd中去
bool canif::getidcmd(unsigned char id, tcmd* cmd)
{

	mutex.lock();

#ifdef HAVECANCARD

	//遍历命令，查找板子id
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
			publiccaution.adddbginfo(QString("成功取得CAN命令 CMD，PARA1，PARA2，PARA3：") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
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

//查看缓存中是否有fid和 cmd的命令，如果有的话，就取到cmd中去
bool canif::getcmd(unsigned char id, unsigned char cmdid, tcmd* cmd)
{
	mutex.lock();

#ifdef HAVECANCARD

	//遍历命令，查找板子id
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
			publiccaution.adddbginfo(QString("成功取得CAN命令 CMD，PARA1，PARA2，PARA3：") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
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

//查看缓存中是否有板子id的 cmdid命令，参数为para1,如果有的话，就取到cmd中去
bool canif::getcmd(unsigned char id, unsigned char cmdid, unsigned char para1,
				  tcmd* cmd)
{
	mutex.lock();

#ifdef HAVECANCARD

	//遍历命令，查找板子id
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
			publiccaution.adddbginfo(QString("成功取得CAN命令 CMD，PARA1，PARA2，PARA3：") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
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

//查看缓存中是否有板子id的 cmdid命令，参数为para1,para2,如果有的话，就取到cmd中去
bool canif::getcmd(unsigned char id, unsigned char cmdid, unsigned char para1,
				  unsigned char para2, tcmd* cmd)
{
	mutex.lock();

#ifdef HAVECANCARD

	//遍历命令，查找板子id
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
			publiccaution.adddbginfo(QString("成功取得CAN命令 CMD，PARA1，PARA2，PARA3：") + QString::number(cmd->cmd,16) + QString(",")+ QString::number(cmd->id,16)+ QString(",") + QString::number(cmd->para1,16) + QString(",")+ QString::number(cmd->para2,16));
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

//查看缓存中是否有板子id的 cmdid命令，参数为para1,如果有的话，就取到cmd中去,返回是数量
int canif::getcmd(unsigned char id, unsigned char cmdid, unsigned char para1,
				  QList<tcmd>& cmdlist)
{

	unsigned int count = 0;
	mutex.lock();

#ifdef HAVECANCARD

	//遍历命令，查找板子id
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
			publiccaution.adddbginfo(QString("成功取得CAN命令 CMD，PARA1，PARA2，PARA3：") + QString::number(cmd.cmd,16) + QString(",")+ QString::number(cmd.id,16)+ QString(",") + QString::number(cmd.para1,16) + QString(",")+ QString::number(cmd.para2,16));
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

//查看缓存中是否有板子id的 cmdid命令,如果有的话，就取到cmdlist中去,返回是数量
int canif::getcmd(unsigned char id,unsigned char cmdid,QList<tcmd>& cmdlist)
{
	unsigned int count = 0;
	mutex.lock();
#ifdef HAVECANCARD

	//遍历命令，查找板子id
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
			publiccaution.adddbginfo(QString("成功取得CAN命令 CMD，PARA1，PARA2，PARA3：") + QString::number(cmd.cmd,16) + QString(",")+ QString::number(cmd.id,16)+ QString(",") + QString::number(cmd.para1,16) + QString(",")+ QString::number(cmd.para2,16));
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

//查看缓存中是否有板子id的 cmdid命令，参数为para1,para2 如果有的话，就取到cmdlist中去,返回是数量
int canif::getcmd(unsigned char id, unsigned char cmdid, unsigned char para1,
				  unsigned char para2, QList<tcmd>& cmdlist)
{
	unsigned int count = 0;
	mutex.lock();
#ifdef HAVECANCARD

	//遍历命令，查找板子id
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
			publiccaution.adddbginfo(QString("成功取得CAN命令 CMD，PARA1，PARA2，PARA3：") + QString::number(cmd.cmd,16) + QString(",")+ QString::number(cmd.id,16)+ QString(",") + QString::number(cmd.para1,16) + QString(",")+ QString::number(cmd.para2,16));
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



//取得接收缓存中的命令数量
int canif::getcmdcount()
{
	int count = 0;
	mutex.lock();
	count = RecvBuff.count();
	mutex.unlock();
	return count;
}

//清除缓存中的命令
int canif::clearcmdbuff()
{
	mutex.lock();
	RecvBuff.clear();
	mutex.unlock();
	return true;
}

//刷新缓存中的时间戳，并且命令如果超时的话，就删除命令
void canif::refreshtime()
{
	mutex.lock();
#ifdef HAVECANCARD

	//遍历命令，查找板子id
	QLinkedList<tcmd>::iterator i ;
	QLinkedList<tcmd>::iterator j ;

	for(i = RecvBuff.begin();i != RecvBuff.end();)
	{
		j = i+1;

		if ((*i).timecnt > CANOUTTIME)
		{
			publiccaution.adddbginfo(QString("删除CAN命令 CMD，PARA1，PARA2，PARA3：") + QString::number((*i).cmd,16) + QString(",")+ QString::number((*i).id,16)+ QString(",") + QString::number((*i).para1,16) + QString(",")+ QString::number((*i).para2,16));
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

//全局变量初始化
void com::init()
{

	//板子列表初始化,根据配置来初始化，例如网头的数量，暂时不管
	//bdlist.append(PCCANBDID);
	bdlist.append(BUMPIOBDID);
	bdlist.append(MAGICIOBDID);

	//根据网头数量设置网头板子
	for(int i = 0+ PRINT1BDID;i < pubcom.PrintNum + PRINT1BDID; i++)
	{
		bdlist.append(i);

		//网头选择
		QString paraname = QString("网头") + QString::number(i+1) + QString("状态");

		//网头是否激活
		QDomNode paranode = GetParaByName("print", paraname);
		bool ok;
		paranode.firstChildElement("value").text().toInt(&ok);
		if (paranode.firstChildElement("value").text().toInt(
			&ok) != 1)
		{
			//设置当前激活的网头板子
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

	//板子canid映射
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

	//板子在线情况
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

	//板子id和故障位置
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

	//命令集合
	cmdlist.append(COMTESTCMD);//通讯测试
	cmdlist.append(VERTESTCMD);//版本校验
	cmdlist.append(SETBUMPNOCMD);//设置浆泵编号
	cmdlist.append(SETPRINTPARACMD);//设置网头参数
	cmdlist.append(SETBUMPIONOCMD);//设置浆泵IO编号
	cmdlist.append(SETMAGICIONOCMD);//设计磁台IO编号
	cmdlist.append(SETMAGICANANOCMD);//设计磁台模拟量编号
	cmdlist.append(CHGSPEEDCMD);//主机调速命令应答
	cmdlist.append(ZEROCMD);//找零命令应答
	cmdlist.append(PRINTWORKSTATUSCMD);//网头工作状态命令
	cmdlist.append(PRINTSWITCHSTATUSCMD);//网头开关状态命令
	cmdlist.append(PRINTXSTATUSCMD);//网头横向位置命令
	cmdlist.append(PRINTYSTATUSCMD);//网头斜向位置命令
	cmdlist.append(PRINTZSTATUSCMD);//网头高度位置命令
	cmdlist.append(PRINTOPCMD);//网头动作命令
	cmdlist.append(STARTWORKCMD);//主机启动命令
	cmdlist.append(QUERYSPEEDCMD);//查询主机速度命令
	cmdlist.append(QUERYMACHSTATUSCMD);//查询机器状态命令
	cmdlist.append(QUERYXSTATUSCMD);//查询工控机保持的X位置
	cmdlist.append(QUERYYSTATUSCMD);//查询工控机保持的Y位置
	cmdlist.append(QUERYZSTATUSCMD);//查询工控机保持的Z位置
	cmdlist.append(PRINTCAUTIONCMD);//网头故障命令应答
	cmdlist.append(ZEROIOCAUTIONCMD);//对零IO板故障命令应答
	cmdlist.append(OUTIOCAUTIONCMD);//对外IO板故障命令应答
	cmdlist.append(BUMPIOCAUTIONCMD);//浆泵IO板故障命令应答
	cmdlist.append(MAGICIOCAUTIONCMD); //磁台IO板故障命令
	cmdlist.append(ENMACHWORKCMD); //允许机器开机命令
	cmdlist.append(CHGMACHSPEEDCMD);//向主机调速命令
	cmdlist.append(ERRSTOPCMD);//向主机报错停机命令
	cmdlist.append(CAUTIONCMD);//向主机告警
	cmdlist.append(MACHWORKACKCMD);//主机开车请求应答
	cmdlist.append(MACHCAUTIONACKCMD);//主机报警应答
	cmdlist.append(MACHERRSTOPACKCMD);//主机报错应答
	cmdlist.append(MACHSTOPACKCMD);//主机停车应答
	cmdlist.append(MAGICFULLCMD);//磁台满输出命令
	cmdlist.append(MAGICRSPCMD);//磁台状态上报命令，无需应答
	cmdlist.append(BUMPRSPCMD);//浆泵状态上报命令，无需应答

	bdonlinestep.insert(BUMPIOBDID,0);
	bdonlinestep.insert(MAGICIOBDID,0);

	for(int i=PRINT1BDID;i <= PRINT16BDID;i++)
	{
		//界面是否有向网头的操作命令请求初始化
		uicmdtoprintmap.insert(i,NOCMD);
		//界面向网头发起命令的步骤初始化
		uicmdtoprintstepmap.insert(i,0);
		//界面向网头发起命令的计时器
		uicmdtoprinttimermap.insert(i,0);

		QDomNode para;
		para = GetParaByName("print", QString("网头") +  QString::number(i+1) + QString("状态"));
		bool ok;
		presetprintstatus[i] = para.firstChildElement("value").text().toInt(&ok);

		printstatus[i] = presetprintstatus[i];

		//对零步骤初始化
		actionzerostep.insert(i,0);

		//纵向对花
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

	//速度命令没有起效果
	speedcmdstep = 0;
	speedcmdcount = 0;
	speedcmdtimer = 0;

	//心跳测试启动
	heartbeatstep = 1;

	//设置界面按键标识
	applyflag = false;

	cardok = false;

	//是否刷新应用，全部参数下发
	refreshapply = false;

	//参数应用是否在进行
	isapply = false;

	//参数应用的步骤
	applystep = 0;

	//参数的应用的定时器
	applytimer = 0;

	printchgspeedansflag = false;
	printchgspeedstep = 0;

	singleprintchgspeedstep = 0;

	printchgspeedfailed = false;
	printchgspeedsucceed = false;

	printactiveflag = false;

	iocautonflag = 0;

	cautionlocationidstrmap.insert(PCCAUTIONLOCATION,QString("工控机"));
	cautionlocationidstrmap.insert(A4NCAUTIONLOCATION,QString("运动卡"));
	cautionlocationidstrmap.insert(CANCAUTIONLOCATION,QString("CAN"));
	cautionlocationidstrmap.insert(PLCLOCATION,QString("外部设备"));
	//cautionlocationidstrmap.insert(OUTIOLOCATION,QString("对外IO板"));
	//cautionlocationidstrmap.insert(ZEROIOLOCATION,QString("对零IO板"));
	cautionlocationidstrmap.insert(MAGICIOLOCATION,QString("磁台IO板"));
	cautionlocationidstrmap.insert(BUMPIOLOCATION,QString("浆泵IO板"));
	cautionlocationidstrmap.insert(PRINT1LOCATION,QString("网头1"));
	cautionlocationidstrmap.insert(PRINT2LOCATION,QString("网头2"));
	cautionlocationidstrmap.insert(PRINT3LOCATION,QString("网头3"));
	cautionlocationidstrmap.insert(PRINT4LOCATION,QString("网头4"));
	cautionlocationidstrmap.insert(PRINT5LOCATION,QString("网头5"));
	cautionlocationidstrmap.insert(PRINT6LOCATION,QString("网头6"));
	cautionlocationidstrmap.insert(PRINT7LOCATION,QString("网头7"));
	cautionlocationidstrmap.insert(PRINT8LOCATION,QString("网头8"));
	cautionlocationidstrmap.insert(PRINT9LOCATION,QString("网头9"));
	cautionlocationidstrmap.insert(PRINT10LOCATION,QString("网头10"));
	cautionlocationidstrmap.insert(PRINT11LOCATION,QString("网头11"));
	cautionlocationidstrmap.insert(PRINT12LOCATION,QString("网头12"));
	cautionlocationidstrmap.insert(PRINT13LOCATION,QString("网头13"));
	cautionlocationidstrmap.insert(PRINT14LOCATION,QString("网头14"));
	cautionlocationidstrmap.insert(PRINT15LOCATION,QString("网头15"));
	cautionlocationidstrmap.insert(PRINT16LOCATION,QString("网头16"));
	//cautionlocationidstrmap.insert(A4N,QString("A4N系统故障"));
	//cautionlocationidstrmap.insert(CANNETWORK,QString("CAN网络"));
	cautionlocationidstrmap.insert(SERIAL,QString("串口"));

	cautionidstrmap.insert(CAUTION,QString("故障"));
	cautionidstrmap.insert(COMCAUTION,QString("离线"));
	cautionidstrmap.insert(VERCAUTION,QString("版本不匹配"));
	cautionidstrmap.insert(PARCAUTION,QString("参数故障"));
	cautionidstrmap.insert(ZEROCAUTION,QString("对零故障"));
	cautionidstrmap.insert(PRINTBUMPCAUTION,QString("网头浆泵故障"));
	cautionidstrmap.insert(PRINTMOTORCAUTION,QString("伺服故障"));
	cautionidstrmap.insert(PLCCAUTION,QString("故障"));
	cautionidstrmap.insert(PLCHALTCATION,QString("急停故障"));
	cautionidstrmap.insert(PCNOA4NCAUTION,QString("没有安装运动卡"));
	cautionidstrmap.insert(PCNOCANCATION,QString("没有安装CAN卡"));
	cautionidstrmap.insert(PCSYSCATION,QString("PC机系统故障"));
	cautionidstrmap.insert(PCERRCANCMD,QString("错误的CAN命令"));

	cautionidstrmap.insert(ZEROANSOUTTIME,QString("对零IO板应答超时"));
	cautionidstrmap.insert(MAGICANSOUTTIME,QString("磁台控制板应答超时"));
	cautionidstrmap.insert(BUMPANSOUTTIME,QString("浆泵控制板应答超时"));
	cautionidstrmap.insert(ZEROSIGNOUTTIME,QString("零位信号超时"));
	cautionidstrmap.insert(XMOVMAX,QString("横向对花上限位"));
	cautionidstrmap.insert(XMOVMIN,QString("横向对花下限位"));
	cautionidstrmap.insert(YMOVMAX,QString("斜向对花上限位"));
	cautionidstrmap.insert(YMOVMIN,QString("斜向对花下限位"));
	cautionidstrmap.insert(XMOTORBOOTERR,QString("横向电机启动错误"));
	cautionidstrmap.insert(YMOTORBOOTERR,QString("斜向电机启动错误"));
	cautionidstrmap.insert(EPRROMERR,QString("EEPROM读写错误"));
	cautionidstrmap.insert(A4NANSOUTTIME,QString("运动控制卡应答超时"));
	cautionidstrmap.insert(REPIDCAUTION,QString("重复的板子CANID"));
	cautionidstrmap.insert(SERIALNOEXIST,QString("不存在"));
	cautionidstrmap.insert(SERIALLOST,QString("离线"));
	cautionidstrmap.insert(CANNETCAUTION,QString("网络故障"));

	eventidstrmap.insert(SENDCMDFAILEVENT,QString("发送命令失败"));
	eventidstrmap.insert(WAITANSOUTTIMEEVENT,QString("接受超时"));
	eventidstrmap.insert(BDONLINEEVENT,QString("上线事件"));
	eventidstrmap.insert(BDOFFLINEEVENT,QString("离线事件"));
	eventidstrmap.insert(UNKNOWCMDEVENT,QString("不认识命令"));
	eventidstrmap.insert(TOOMUCHUNUSEDCMDEVENT,QString("过多未处理的命令"));


	//心跳
	heartbeatstep = 0;
	heartbeattimer = 0;
	heartbeatcount = 0;

	//启动
	plcreqstartstep = 0;
	plcreqstarttimer = 0;
	plcreqstartcount = 0;
	printok = false;

	//停止
	plcreqstopstep = 0;
	plcreqstopcount = 0;
	plcreqstoptimer = 0;

	//主界面命令
	mainuicmdstep = 0;
	mainuicmdcount = 0;
	mainuidcmdtimer = 0;
	mainuidcmd = 0;

	//来自PLC的命令
	plccmdstep = 0;
	plccmdcount = 0;
	plcdcmdtimer = 0;
	plcdcmd = 0;

	//操作界面命令
	opuicmdflag = 0;
	opuicmdstep = 0;
	opuicmdtimer = 0;
	opuicmdprintno = 0;
	opuicmdcmd = 0;
	opuicmdcount = 0;

	//恢复网头位置任务初始化
	for(int i= 0;i<16;i++)
	{
		restoreTask[i].id = i;
		restoreTask[i].timer = 0;
		restoreTask[i].step = 0;
	}

	//保存网头当前位置任务初始化
	for(int i= 0;i<16;i++)
	{
		savelocTask[i].id = i;
		savelocTask[i].timer = 0;
		savelocTask[i].step = 0;
	}

	//网头调速
	printchgspeedstep = 0;
	printchgspeedcount = 0;
	printchgspeedtimer = 0;

	enuichgspdbt = false;

	statuschgstep = 0;

	//IO端口上报故障状态
	IOCautionStatus = false;
	IOCautionStatusstep = 0;
	IOCautionStatuscount = 0;
	IOCautionStatustimer = 0;

	//IO端口上报错误状态
	IOErrStatus = false;
	IOErrStatusstep = 0;
	IOErrStatuscount = 0;
	IOErrStatustimer = 0;

	//IO端口上报急停状态
	IOHaltStatus = false;
	IOHaltStatusstep = 0;
	IOHaltStatuscount = 0;
	IOHaltStatustimer = 0;

	//刷新数据请求
	FlushDataReqStep = 0;
	FlushDataReqTimer = 0;

	quitthread = false;

	PowerOff = false;
	
	QDomNode para;
	para = GetParaByName("prod", "设定印制米数停车");
	bool ok;
	NeedStopAtWorkfinished = para.firstChildElement("value").text().toInt(&ok);

	//对零传感器信号宽度
	para = GetParaByName("dev", "对零传感器信号宽度");
	ZeroDetectWidth = para.firstChildElement("value").text().toInt(&ok);

	//对零高速段宽度
	para = GetParaByName("dev", "对零高速段宽度");
	ZeroHighSpeedWidth = para.firstChildElement("value").text().toInt(&ok);

	//对零低速段宽度
	para = GetParaByName("dev", "对零低速段宽度");
	ZeroLowSpeedWidth = para.firstChildElement("value").text().toInt(&ok);

	//是否记录操作
	QString paraname = QString("记录操作");

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

	//是否记录异常
	paraname = QString("记录异常");
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

	//是否记录调试信息
	paraname = QString("是否记录调试信息");
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

	//串口离线标识
	OfflineFlag = false;

	//启动上升沿
	PlcStartUpFlag = false;

	//停止上升沿
	PlcStopUpFlag = false;

	//错误上升沿
	PlcErrUpFlag = false;

	//错误下降沿
	PlcErrDownFlag = false;

	//告警上升沿
	PlcCauUpFlag = false;

	//告警下降沿
	PlcCauDownFlag = false;

	//进布运行上升沿
	PlcJinbuyunxinUpFlag = false;

	//进布运行下降沿
	PlcJinbuyunxinDownFlag = false;

	//印花导带运行上升沿
	PlcYinhuadaodaiyunxinUpFlag = false;

	//印花导带运行下降沿
	PlcYinhuadaodaiyunxinDownFlag = false;

	//烘房网带1运行上升沿
	PlcHongfangwangdai1yunxinUpFlag = false;

	//烘房网带1运行下降沿
	PlcHongfangwangdai1yunxinDownFlag = false;

	//烘房网带2运行上升沿
	PlcHongfangwangdai2yunxinUpFlag = false;

	//烘房网带2运行下降沿
	PlcHongfangwangdai2yunxinDownFlag = false;

	//落布运行上升沿
	PlcLuobuyunxinUpFlag = false;

	//落布运行下降沿
	PlcLuobuyunxinDownFlag = false;

	//供浆磁力开关上升沿
	PlcGongjiangcilikaiguanUpFlag = false;

	//供浆磁力开关下降沿
	PlcGongjiangcilikaiguanDownFlag = false;

	//磁力大小改变标识
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


//设置界面是否有向网头发起的命令请求
int com::setcmdtoprint(int bdid,bool flag)
{
	uicmdtoprintmap.insert(bdid,flag);
	return true;
}

//是否是命令
bool com::iscmd(unsigned char cmd)
{
	return cmdlist.contains(cmd);
}

//是否有板子上线
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

//寄存器初始化接口
void initRegs(struct _regs* regs)
{
	//置0
	memset(regs->readarray,0,100);
	memset(regs->hisreadarray,0,100);
	memset(regs->writearray,0,100);
	regs->heartbeatcnt = 0;
}

//regs针对modbus的接口
void ReadRegs(struct _regs* regs,unsigned int index,unsigned char* ch)
{
	*ch = regs->writearray[index];
}

//[0]字节：发送给PC的高8位IO,[1]字节：发送给PC的低8位IO,[2-3]：发送给PC的磁台磁力 [5]:发送给PC的新低8位IO [4]:发送给PC的新高8位IO
void WriteRegs(struct _regs* regs,unsigned int index,unsigned char ch)
{
	regs->readarray[index] = ch;

	//是否IO改变，IO改变就调用Hook程序
	if (index == 0)
	{
		if ( ((regs->hisreadarray[index] >> (regs->PlcLuobuyunxinBitIndex-8)) & 0x01) == 0x00
			&& ((regs->readarray[index] >> (regs->PlcLuobuyunxinBitIndex-8)) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcLuobuyunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> (regs->PlcLuobuyunxinBitIndex-8)) & 0x01) == 0x01
			&& ((regs->readarray[index] >> (regs->PlcLuobuyunxinBitIndex-8)) & 0x01) == 0x00 )
		{
			//下降沿
			regs->PlcLuobuyunxinFp(false);
		}
		if ( ((regs->hisreadarray[index] >> (regs->PlcGongjiangcilikaiguanBitIndex-8)) & 0x01) == 0x00
			&& ((regs->readarray[index] >> (regs->PlcGongjiangcilikaiguanBitIndex-8)) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcGongjiangcilikaiguanFp(true);
		}
		if ( ((regs->hisreadarray[index] >> (regs->PlcGongjiangcilikaiguanBitIndex-8)) & 0x01) == 0x01
			&& ((regs->readarray[index] >> (regs->PlcGongjiangcilikaiguanBitIndex-8)) & 0x01) == 0x00 )
		{
			//下降沿
			regs->PlcGongjiangcilikaiguanFp(false);
		}
	}
	else if (index == 1)
	{
		if ( ((regs->hisreadarray[index] >> regs->PlcStartReqBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcStartReqBitIndex) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcStartReqFp(true);
			printf("start 上升沿\n");
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcStopBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcStopBitIndex) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcStopFp(true);
			printf("stop 上升沿\n");
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}

		if ( ((regs->hisreadarray[index] >> regs->PlcErrBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcErrBitIndex) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcErrFp(true);
			printf("err 上升沿\n");
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcErrBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcErrBitIndex) & 0x01) == 0x00 )
		{
			//下降沿
			regs->PlcErrFp(false);
			printf("err 下降沿\n");
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcCauBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcCauBitIndex) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcCauFp(true);
			printf("cau 上升沿\n");
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcCauBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcCauBitIndex) & 0x01) == 0x00 )
		{
			//下降沿
			regs->PlcCauFp(false);
			printf("cau 下降沿\n");
		}

		if ( ((regs->hisreadarray[index] >> regs->PlcJinbuyunxinBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcJinbuyunxinBitIndex) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcJinbuyunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcJinbuyunxinBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcJinbuyunxinBitIndex) & 0x01) == 0x00 )
		{
			//下降沿
			regs->PlcJinbuyunxinFp(false);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcYinhuadaodaiyunxinBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcYinhuadaodaiyunxinBitIndex) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcYinhuadaodaiyunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcYinhuadaodaiyunxinBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcYinhuadaodaiyunxinBitIndex) & 0x01) == 0x00 )
		{
			//下降沿
			regs->PlcYinhuadaodaiyunxinFp(false);
		}

		if ( ((regs->hisreadarray[index] >> regs->PlcHongfangwangdai1yunxinBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcHongfangwangdai1yunxinBitIndex) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcHongfangwangdai1yunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcHongfangwangdai1yunxinBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcHongfangwangdai1yunxinBitIndex) & 0x01) == 0x00 )
		{
			//下降沿
			regs->PlcHongfangwangdai1yunxinFp(false);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcHongfangwangdai2yunxinBitIndex) & 0x01) == 0x00
			&& ((regs->readarray[index] >> regs->PlcHongfangwangdai2yunxinBitIndex) & 0x01) == 0x01 )
		{
			//上升沿
			regs->PlcHongfangwangdai2yunxinFp(true);
		}
		if ( ((regs->hisreadarray[index] >> regs->PlcHongfangwangdai2yunxinBitIndex) & 0x01) == 0x01
			&& ((regs->readarray[index] >> regs->PlcHongfangwangdai2yunxinBitIndex) & 0x01) == 0x00 )
		{
			//下降沿
			regs->PlcHongfangwangdai2yunxinFp(false);
		}
	}//磁力大小改变([3]字节低8位 [2]字节高8位)，如果磁台磁力改变(磁台磁力百分比)则发送给每个网头
	else if(index == 3)
	{
		if((regs->hisreadarray[index] != regs->readarray[index]) )
		{
			//磁台磁力修改
			//interfaceevent* ptempevent = new interfaceevent();

			//ptempevent->cmd = 0x12;//网头操作命令
			//ptempevent->status = 0x00;//命令状态
			//ptempevent->data[0] = 0x58;//磁力修改
			//ptempevent->data[1] = regs->readarray[index];//磁力大小

			//发送一个事件给后台线程
			//QCoreApplication::postEvent(backendobject, ptempevent);
			//printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);

			//磁力大小改变标识，待处理
            //regs->PlcGongjiangcilidaxiaogaibianFp(true,regs->readarray[index]);
		}
	}
	else if(index == 5)
	{
		if ( ((regs->hisreadarray[index] >> 0) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 0) & 0x01) == 0x01 )
		{
			//网头下放上升沿
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//网头操作命令
			ptempevent->status = 0x00;//命令状态
			ptempevent->data[0] = 0x04;//小命令字，下放还是上抬

			//发送一个事件给后台线程
			QCoreApplication::postEvent(backendobject, ptempevent);
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> 1) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 1) & 0x01) == 0x01 )
		{
			//网头抬起上升沿
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//网头操作命令
			ptempevent->status = 0x00;//命令状态
			ptempevent->data[0] = 0x03;//小命令字，下放还是上抬

			//发送一个事件给后台线程
			QCoreApplication::postEvent(backendobject, ptempevent);
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> 2) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 2) & 0x01) == 0x01 )
		{
			//磁台上磁上升沿
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//网头操作命令
			ptempevent->status = 0x00;//命令状态
			ptempevent->data[0] = 0x07;//磁台开启

			//发送一个事件给界面线程
			QCoreApplication::postEvent(backendobject, ptempevent);
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> 3) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 3) & 0x01) == 0x01 )
		{
			//磁台去磁上升沿
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//网头操作命令
			ptempevent->status = 0x00;//命令状态
			ptempevent->data[0] = 0x08;//磁台关闭

			//发送一个事件给界面线程
			QCoreApplication::postEvent(backendobject, ptempevent);
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
		}
		if ( ((regs->hisreadarray[index] >> 4) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 4) & 0x01) == 0x01 )
		{
			//网头对零上升沿
			//运行状态，气压欠压、急停不允许启动对零
			if ((pubcom.machprintstatus != 2) && (publiccaution.hascaution(PLCLOCATION, PLCHALTCATION) == false) )
			{
				interfaceevent* ptempevent = new interfaceevent();

				ptempevent->cmd = 0x12;//网头操作命令
				ptempevent->status = 0x00;//命令状态
				ptempevent->data[0] = 0x53;//对零

				//发送一个事件给后台线程
				QCoreApplication::postEvent(backendobject, ptempevent);
				printf("plc对零\n");

			}
		}
		if ( ((regs->hisreadarray[index] >> 5) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 5) & 0x01) == 0x01 )
		{
			//网头反转上升沿
			//运行状态不能启动反转和停止反转，外部急停故障状态不允许启动反转
			if ((pubcom.machprintstatus != 2) && ((publiccaution.hascaution(PLCLOCATION,PLCHALTCATION) == false) || ((ptech->circleflag) == true)) )
			{
				bool cancircle = true;
				QList<int>::iterator it;

				//多线程保护
				pubcom.activeprintlistmutex.lockForRead();
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{

					//对零中不可以反转
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

					ptempevent->cmd = 0x12;//网头操作命令
					ptempevent->status = 0x00;//命令状态
					ptempevent->data[0] = 0x54;//反转

					//发送一个事件给界面线程
					QCoreApplication::postEvent(backendobject, ptempevent);
					ptech->circleflag = true;
				}
			}
		}
		if ( ((regs->hisreadarray[index] >> 6) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 6) & 0x01) == 0x01 )
		{
			//产量清零上升沿
			{
				ptech->curworknumber = 0;
				ptech->sumcount = 0;
				mydb.SaveValue("curworknumber",ptech->curworknumber);
			}
		}
		if ( ((regs->hisreadarray[index] >> 5) & 0x01) == 0x01
			&& ((regs->readarray[index] >> 5) & 0x01) == 0x00 )
		{
			//网头反转下降沿
			//运行状态，不允许反转
			if (pubcom.machprintstatus != 2)
			{
				bool cancircle = true;
				QList<int>::iterator it;

				//多线程保护
				pubcom.activeprintlistmutex.lockForRead();
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{

					//对零中不可以反转
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

					ptempevent->cmd = 0x12;//网头操作命令
					ptempevent->status = 0x00;//命令状态
					ptempevent->data[0] = 0x54;//反转

					//发送一个事件给界面线程
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
			//浆泵开启
			printf("浆泵开启\n");
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//网头操作命令
			ptempevent->status = 0x00;//命令状态
			ptempevent->data[0] = 0x59;//自动开启

			//发送一个事件给界面线程
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
		if ( ((regs->hisreadarray[index] >> 2) & 0x01) == 0x00
			&& ((regs->readarray[index] >> 2) & 0x01) == 0x01 )
		{
			//浆泵停止
			printf("浆泵停止\n");
			printf("%x,%x\n",regs->hisreadarray[index],regs->readarray[index]);
			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x12;//网头操作命令
			ptempevent->status = 0x00;//命令状态
			ptempevent->data[0] = 0x60;//自动关闭

			//发送一个事件给界面线程
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}

	else if((index == 6) 
		|| (index == 7))//产量设置
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
		|| (index == 9))//速差百分比
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

//工控机开机允许位位置
void SetPcEnableStartBitIndex(struct _regs* regs,int index)
{
	regs->PcEnableStartBitIndex = index;
}

//工控机主机加速位位置
void SetPcMotorAccBitIndex(struct _regs* regs,int index)
{
	regs->PcMotorAccBitIndex = index;
}

//工控机主机减速位位置
void SetPcMotorDecBitIndex(struct _regs* regs,int index)
{
	regs->PcMotorDecBitIndex = index;
}

//工控机报错位位置
void SetPcErrBitIndex(struct _regs* regs,int index)
{
	regs->PcErrBitIndex = index;
}

//工控机报警位位置
void SetPcCauBitIndex(struct _regs* regs,int index)
{
	regs->PcCauBitIndex = index;
}

//工控机开橡毯启动位位置
void SetPcMotorOnBitIndex(struct _regs* regs,int index)
{
	regs->PcMotorOnBitIndex = index;
}

//工控机橡毯停止位位置
void SetPcMotorOffBitIndex(struct _regs* regs,int index)
{
	regs->PcMotorOffBitIndex = index;
}

//工控机工作量满位位置
void SetPcWorkFinBitIndex(struct _regs* regs,int index)
{
	regs->PcWorkFinBitIndex = index;
}

//工控机设置心跳位位置
void SetPcSetHeartBeatBitIndex(struct _regs* regs,int index)
{
	regs->PcSetHeartBeatBitIndex = index;
}

//工控机请求刷新数据位位置
void SetPcFlushDataReqBitIndex(struct _regs* regs,int index)
{
	regs->PcFlushDataReqBitIndex = index;
}

//PLC主机开机请求位位置
void SetPlcStartReqBitIndex(struct _regs* regs,int index)
{
	regs->PlcStartReqBitIndex = index;
}

//PLC主机停止位位置
void SetPlcStopBitIndex(struct _regs* regs,int index)
{
	regs->PlcStopBitIndex = index;
}

//PLC报错位位置
void SetPlcErrBitIndex(struct _regs* regs,int index)
{
	regs->PlcErrBitIndex = index;
}

//PLC报警位位置
void SetPlcCauBitIndex(struct _regs* regs,int index)
{
	regs->PlcCauBitIndex = index;
}

//PLC进布运行位位置
void SetPlcJinbuyunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcJinbuyunxinBitIndex = index;
}

//PLC印花导带运行位位置
void SetPlcYinhuadaodaiyunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcYinhuadaodaiyunxinBitIndex = index;
}

//PLC烘房网带1运行位位置
void SetPlcHongfangwangdai1yunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcHongfangwangdai1yunxinBitIndex = index;
}

//PLC烘房网带2运行位位置
void SetPlcHongfangwangdai2yunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcHongfangwangdai2yunxinBitIndex = index;
}

//PLC落布运行位位置
void SetPlcLuobuyunxinBitIndex(struct _regs* regs,int index)
{
	regs->PlcLuobuyunxinBitIndex = index;
}

//PLC供浆磁力开关位位置
void SetPlcGongjiangcilikaiguanBitIndex(struct _regs* regs,int index)
{
	regs->PlcGongjiangcilikaiguanBitIndex = index;
}


//工控机输出IO字位置
void SetPcOutIOWordIndex(struct _regs* regs,int index)
{
	regs->PcOutIOWordIndex = index;
}

//工控机计米双字位置
void SetPcWorkRecDWordIndex(struct _regs* regs,int index)
{
	regs->PcWorkRecDWordIndex = index;
}


//设置输入IO处理函数指针（回调接口）
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

//置位内部函数，非对外接口
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

//开机允许
void WritePcEnableStartBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PlcStartReqBitIndex);
}

//主机加速
void WritePcMotorAccBit(struct _regs* regs,bool bit)
{

	SetWriteRegsBit(regs,bit,regs->PcMotorAccBitIndex);

}

//主机减速
void WritePcMotorDecBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcMotorDecBitIndex);

}

//工控机报错
void WritePcErrBit(struct _regs* regs,bool bit)
{

	SetWriteRegsBit(regs,bit,regs->PcErrBitIndex);

}

//工控机报警
void WritePcCauBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcCauBitIndex);

}

//马达启动
void WritePcMotorOnBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcMotorOnBitIndex);
	//printf("IO:%X",regs->writearray[regs->PcOutIOWordIndex*2 + 1]);

}

//马达停止
void WritePcMotorOffBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcMotorOffBitIndex);
	//printf("IO:%X",regs->writearray[regs->PcOutIOWordIndex*2 + 1]);
}

//工作量满
void WritePcWorkFinBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcWorkFinBitIndex);

}

//写心跳位
void WritePcSetHeartBeatBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcSetHeartBeatBitIndex);
}

//写刷新数据请求
void WritePcFlushDataReqBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,regs->PcFlushDataReqBitIndex);

}

//写工作时磁台未开启告警
void WriteMagicCauBit(struct _regs* regs,bool bit)
{
	SetWriteRegsBit(regs,bit,10);
}

//写工作量
void WritePcWorkRecDWord(struct _regs* regs,int word)
{
	regs->writearray[regs->PcWorkRecDWordIndex * 2 ] = (unsigned char)(word >> 8);
	regs->writearray[regs->PcWorkRecDWordIndex * 2 + 1] = (unsigned char)(word);
	regs->writearray[regs->PcWorkRecDWordIndex * 2 + 2] = (unsigned char)(word >> 24);
	regs->writearray[regs->PcWorkRecDWordIndex * 2 + 3] = (unsigned char)(word>>16);
}


//刷新读区的心跳寄存器
void FlushRegsReadHeatbeat(struct _regs* regs)
{
	if((regs->heartbeatcnt) < 20)
	(regs->heartbeatcnt)++;
}

//读读区的心跳计数值
void ReadRegsReadHeatbeat(struct _regs* regs,unsigned char* pch)
{
	*pch = regs->heartbeatcnt;
}


//输入IO处理函数
//启动请求
void PlcStartReqHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("start OK!\n");
		pubcom.PlcStartUpFlag = flag;
	}
}

//停止
void PlcStopHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("stop OK!\n");
		pubcom.PlcStopUpFlag = flag;
	}
}
//错误
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
//告警
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

//进布运行hook
void PlcJinbuyunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("进布运行\n");
		pubcom.PlcJinbuyunxinUpFlag = true;
		pubcom.PlcJinbuyunxinDownFlag = false;
	} 
	else
	{
		Dbgout("进布停止\n");
		pubcom.PlcJinbuyunxinDownFlag = true;
		pubcom.PlcJinbuyunxinUpFlag = false;
	}
}

//印花导带运行hook
void PlcYinhuadaodaiyunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("印花导带运行\n");
		pubcom.PlcYinhuadaodaiyunxinUpFlag = true;
		pubcom.PlcYinhuadaodaiyunxinDownFlag = false;
	} 
	else
	{
		Dbgout("印花导带停止\n");
		pubcom.PlcYinhuadaodaiyunxinDownFlag = true;
		pubcom.PlcYinhuadaodaiyunxinUpFlag = false;
	}
}

//烘房网带1运行hook
void PlcHongfangwangdai1yunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("烘房网带1运行\n");
		pubcom.PlcHongfangwangdai1yunxinUpFlag = true;
		pubcom.PlcHongfangwangdai1yunxinDownFlag = false;
	} 
	else
	{
		Dbgout("烘房网带1停止\n");
		pubcom.PlcHongfangwangdai1yunxinDownFlag = true;
		pubcom.PlcHongfangwangdai1yunxinUpFlag = false;
	}
}

//烘房网带2运行hook
void PlcHongfangwangdai2yunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("烘房网带2运行\n");
		pubcom.PlcHongfangwangdai2yunxinUpFlag = true;
		pubcom.PlcHongfangwangdai2yunxinDownFlag = false;
	} 
	else
	{
		Dbgout("烘房网带2停止\n");
		pubcom.PlcHongfangwangdai2yunxinDownFlag = true;
		pubcom.PlcHongfangwangdai2yunxinUpFlag = false;
	}
}

//落布运行hook
void PlcLuobuyunxinHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("落布运行\n");
		pubcom.PlcLuobuyunxinUpFlag = true;
		pubcom.PlcLuobuyunxinDownFlag = false;
	} 
	else
	{
		Dbgout("落布停止\n");
		pubcom.PlcLuobuyunxinDownFlag = true;
		pubcom.PlcLuobuyunxinUpFlag = false;
	}
}

//供浆磁力开关hook
void PlcGongjiangcilikaiguanHook(bool flag)
{
	if (flag == true)
	{
		Dbgout("供浆磁力开\n");
		pubcom.PlcGongjiangcilikaiguanUpFlag = true;
	} 
	else
	{
		Dbgout("供浆磁力关\n");
		pubcom.PlcGongjiangcilikaiguanDownFlag = true;
	}
}

//供浆磁力大小改变hook
void PlcGongjiangcilidaxiaogaibianHook(bool flag,unsigned char percent)
{
	pubcom.PlcGongjiangcilidaxiao = percent;
	pubcom.PlcGongjiangcilidaxiaogaibianFlag = true;
}




