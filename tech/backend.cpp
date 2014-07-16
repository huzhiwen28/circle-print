/*
* backend.cpp
*
*  Created on: 2009-8-8
*      Author: 胡志文
*/

#include "backend.h"
#include <iostream>
#include "custevent.h"
#include "tech.h"
#include "canif.h"
#include <QMessageBox>
#include "canidcmd.h"
#include "caution.h"
#include "set.h"
#include <QSettings>
#include <QtSql>
#include <QTableView>
#include <QSqlTableModel>
#include <QList>
#include "CMotor.h"
#include "DB.h"
#include "serialif.h"
#include "measurepulse.h"
#include "modbus.h"

extern canif candevice;
extern QObject* setobject;
extern timermng pubtimermng;
extern QObject* opobject;
extern QObject* backendobject;
extern QObject* interfaceobject;
extern QObject* a4nobject;
extern QObject* selectprintobject;
extern QObject* measurepulseobject;

extern set* pDlgset;
extern QSettings settings;
extern struct _modbusRTU modbusRTU;
extern struct _regs regs;


int GetParaRow(const QString& groupname);

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);

extern CKSMCA4 *ksmc;
extern DB mydb;
extern double f_follow;


//返回位置任务执行体
void restoreTaskRun(struct restore* pTask)
{
	if(pTask->step <= 0)
		return;

	if((pTask->step) == 1)
	{
		//新增定时器
		pTask->timer = pubtimermng.addtimer();
		pTask->step = 2;
	}
	else if((pTask->step) == 2)
	{
		if(pubtimermng.gettimerval(pTask->timer) >= 2)
		{
			pubtimermng.deltimer(pTask->timer);
			pTask->step = 3;
		}
	}
	else if((pTask->step) == 3)
	{
		int status = 0;
		ksmc->GetMotorInPosStatus((pTask->id) + 1,status);
		if ( status == 1 )
		{
			ksmc->RestorePrintLocation((pTask->id) + 1);
			//标识
			pTask->step = 0;
		}
	}
}

//记忆位置任务执行体
void savelocTaskRun(struct saveloc* pTask)
{
	if(pTask->step <= 0)
		return;

	if((pTask->step) == 1)
	{
		//新增定时器
		pTask->timer = pubtimermng.addtimer();
		pTask->step = 2;
	}
	else if((pTask->step) == 2)
	{
		if(pubtimermng.gettimerval(pTask->timer) >= 2)
		{
			pubtimermng.deltimer(pTask->timer);
			pTask->step = 3;
		}
	}
	else if((pTask->step) == 3)
	{
		int status = 0;
		ksmc->GetMotorInPosStatus((pTask->id) + 1,status);
		if ( status == 1 )
		{
			ksmc->RememberPrintLoction((pTask->id) + 1);
			//标识
			pTask->step = 0;
		}
	}
}

backend::backend()
{
	pubcom.heartbeatstep = 1;
	pubcom.heartbeatbegincnt = 0;
}

backend::~backend()
{

}

//界面事件消息处理
void backend::customEvent(QEvent * e)
{
	//得到是界面的事件
	if (e->type() == INTERFACE_EVENT) 
	{
		interfaceevent* pinterfaceevent = (interfaceevent*) e;

		//主界面网头操作
		if (pinterfaceevent->cmd == 0x02 && pinterfaceevent->status == 0x00)
		{
			//主界面操作和网头操作是互斥的
			if (pubcom.ismainuicmd != true 
				&& pubcom.opuicmdflag != true
				&& pubcom.isplccmd != true)
			{
				publiccaution.adddbginfo(QString("后台接受主界面网头操作事件"));

				//设置命令标识
				pubcom.ismainuicmd = true;
				pubcom.mainuicmdstep = 1;
				pubcom.mainuidcmd = pinterfaceevent->data[0];
			}
			else//原本就有整机命令或者网头操作界面命令
			{
				publiccaution.adddbginfo(QString("后台拒绝主界面网头操作事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x02;
				ptempevent->status = 0x02;
				ptempevent->data[0] = pinterfaceevent->data[0];

				//发送一个事件给界面线程
				QCoreApplication::postEvent(interfaceobject, ptempevent);
			}
		}
		//PLC上操作
		else if (pinterfaceevent->cmd == 0x12 && pinterfaceevent->status == 0x00)
		{
			//主界面操作和网头操作是互斥的
			if (pubcom.ismainuicmd != true 
				&& pubcom.opuicmdflag != true
				&& pubcom.isplccmd != true)
			{
				publiccaution.adddbginfo(QString("后台接受PLC网头操作事件"));

				//设置命令标识
				pubcom.isplccmd = true;
				pubcom.plccmdstep = 1;
				pubcom.plcdcmd = pinterfaceevent->data[0];
			}
			else//原本就有整机命令或者网头操作界面命令，忽略之
			{
				publiccaution.adddbginfo(QString("后台拒绝PLC网头操作事件"));
			}
		}

		//网头操作界面网头操作
		else if (pinterfaceevent->cmd == 0x03 && pinterfaceevent->status == 0x00)
		{
			//看看原来是否有网头操作指令在执行,包括主界面和网头操作界面
			if (pubcom.ismainuicmd != true 
				&& pubcom.opuicmdflag != true
				&& pubcom.isplccmd != true)
			{
				publiccaution.adddbginfo(QString("后台接受网头操作界面操作事件"));

				pubcom.opuicmdflag = true;
				pubcom.opuicmdstep = 1;
				pubcom.opuicmdcmd = pinterfaceevent->data[0];
				pubcom.opuicmdprintno = pinterfaceevent->data[1];

			}
			else//已经有操作，直接发送完成命令
			{
				publiccaution.adddbginfo(QString("后台拒绝网头操作界面操作事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x03;
				ptempevent->status = 0x01;
				ptempevent->data[0] = pinterfaceevent->data[0];

				//发送一个事件给界面线程
				QCoreApplication::postEvent(opobject, ptempevent);
			}
		}
		//参数应用
		else if (pinterfaceevent->cmd == 0x04 && pinterfaceevent->status == 0x00)
		{
			if (pubcom.isapply == true)
			{
				publiccaution.adddbginfo(QString("后台拒绝界面参数应用事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x04;
				ptempevent->status = 0x01;
				ptempevent->data[0] = 0x00;

				//发送一个事件给界面线程
				QCoreApplication::postEvent(setobject, ptempevent);
			}
			else if(pubcom.hasbdonline() == true)
			{
				publiccaution.adddbginfo(QString("后台拒绝界面参数应用事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x04;
				ptempevent->status = 0x04;
				ptempevent->data[0] = 0x00;

				//发送一个事件给界面线程
				QCoreApplication::postEvent(setobject, ptempevent);

			}
			else
			{
				publiccaution.adddbginfo(QString("后台接受界面参数应用事件"));

				//设置标识
				pubcom.applyansplace = 1;
				pubcom.isapply = true;
				pubcom.applystep = 1;
				pubcom.hasparafailed = false;
				pubcom.hasparasucceed = false;
			}
		}
		//参数确定
		else if (pinterfaceevent->cmd == 0x05 && pinterfaceevent->status == 0x00)
		{
			if (pubcom.isapply == true)
			{
				publiccaution.adddbginfo(QString("后台拒绝界面参数确定事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x05;
				ptempevent->status = 0x01;
				ptempevent->data[0] = 0x00;

				//发送一个事件给界面线程
				QCoreApplication::postEvent(interfaceobject, ptempevent);
			}
			else if(pubcom.hasbdonline() == true)
			{
				publiccaution.adddbginfo(QString("后台拒绝界面参数确定事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x05;
				ptempevent->status = 0x04;
				ptempevent->data[0] = 0x00;

				//发送一个事件给界面线程
				QCoreApplication::postEvent(interfaceobject, ptempevent);
			}
			else
			{
				publiccaution.adddbginfo(QString("后台接受界面参数确定事件"));

				//设置标识
				pubcom.applyansplace = 2;
				pubcom.isapply = true;
				pubcom.applystep = 1;
				pubcom.hasparafailed = false;
				pubcom.hasparasucceed = false;
			}
		}
		//网头调速开关
		else if (pinterfaceevent->cmd == 0x06 && pinterfaceevent->status == 0x00)
		{
			if (pubcom.printchgspeedansflag == true)
			{
				publiccaution.adddbginfo(QString("后台拒绝界面网头调试开关事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x06;
				ptempevent->status = 0x01;
				ptempevent->data[0] = 0x00;

				//发送一个事件给界面线程
				QCoreApplication::postEvent(interfaceobject, ptempevent);
			}
			else
			{
				publiccaution.adddbginfo(QString("后台接受界面网头调试开关事件"));

				//设置标识
				pubcom.printchgspeedansflag = true;
				pubcom.printchgspeedstep = 1;
				pubcom.printchgspeedfailed = false;
				pubcom.printchgspeedsucceed = false;
			}
		}
		//单网头调速开关
		else if (pinterfaceevent->cmd == 0x0b && pinterfaceevent->status == 0x00)
		{
			if (pubcom.printchgspeedansflag == false)
			{
				//设置标识
				pubcom.printchgspeedansflag = true;
				pubcom.singleprintchgspeedstep = 1;
			}
		}
		//网头激活和休眠开关
		else if (pinterfaceevent->cmd == 0x07 && pinterfaceevent->status == 0x00)
		{
			//原本就在操作，那么不理睬
			if (pubcom.printactiveflag == true)
			{
				publiccaution.adddbginfo(QString("后台拒绝界面网头激活和休眠事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x07;
				ptempevent->status = 0x03;
				ptempevent->data[0] = pinterfaceevent->data[0];

				//发送一个事件给界面线程
				QCoreApplication::postEvent(selectprintobject, ptempevent);
			}
			else
			{
				publiccaution.adddbginfo(QString("后台接受界面网头激活和休眠事件"));

				//设置标识
				pubcom.printactiveflag = true;
				pubcom.printactivestep = 1;
				pubcom.printactivenum = pinterfaceevent->data[0];
				pubcom.printactivecmd = pinterfaceevent->data[1];
			}
		}
		//强制刷新参数
		else if (pinterfaceevent->cmd == 0x08 && pinterfaceevent->status == 0x00)
		{
			if (pubcom.isapply == true )
			{
				publiccaution.adddbginfo(QString("后台拒绝界面网头强制刷新参数事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x04;
				ptempevent->status = 0x01;
				ptempevent->data[0] = 0x00;

				//发送一个事件给界面线程
				QCoreApplication::postEvent(setobject, ptempevent);
			}
			else if(pubcom.hasbdonline() == true)
			{
				publiccaution.adddbginfo(QString("后台拒绝界面网头强制刷新参数事件"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x04;
				ptempevent->status = 0x04;
				ptempevent->data[0] = 0x00;

				//发送一个事件给界面线程
				QCoreApplication::postEvent(setobject, ptempevent);

			}
			else
			{
				publiccaution.adddbginfo(QString("后台接受界面网头强制刷新参数事件"));

				//设置标识
				pubcom.applyansplace = 1;
				pubcom.isapply = true;
				pubcom.refreshapply = true;
				pubcom.applystep = 1;
				pubcom.hasparafailed = false;
				pubcom.hasparasucceed = false;
			}
		}
		//摩擦系数测试
		else if (pinterfaceevent->cmd == 0x0a && pinterfaceevent->status == 0x00)
		{
			publiccaution.adddbginfo(QString("后台接受界面橡毯移动开始事件"));

			//设置标识
			pubcom.measurepulsestartstep = 1;
		}
		//摩擦系数测试
		else if (pinterfaceevent->cmd == 0x0a && pinterfaceevent->status == 0x01)
		{
			publiccaution.adddbginfo(QString("后台接受界面橡毯移动停止事件"));

			//设置标识
			pubcom.measurepulsestopstep = 1;
		}
		else
		{
			publiccaution.adddbginfo(QString("不认识的界面事件"));
		}
	}
	else
	{
		publiccaution.adddbginfo(QString("不认识的事件"));
	}


}

//线程主循环
void backend::run()
{
	//总跟随滞后值
	float currentfollow = 0.0;
	float oldcurrentfollow = 0.0;

	while (pubcom.quitthread == false)
	{

		//CAN刷新读
		candevice.refresh();

		///////////////////////////////////////////////////////
		//内部事件处理，在处理完事件对应函数后，要删除事件队列中的事件
		///////////////////////////////////////////////////////
		if (pubcom.eventqueue.count() > 0)
		{
			QList<tintevent>::iterator i;
			for (i = pubcom.eventqueue.begin(); i != pubcom.eventqueue.end(); ++i)
			{
				switch ((*i).eventid)
				{
				case 1://板子上线

					publiccaution.addevent(
						QString("板子") + QString::number((*i).nodeid),
						"板子上线事件","板子上线事件");

					publiccaution.adddbginfo(QString("板子上线事件"));

					if (pubcom.bdonlinestep.value((*i).nodeid) == 0)
					{
						pubcom.bdonlinestep.insert((*i).nodeid,1);
					}

					//删除板子不在线故障
					publiccaution.delcaution(pubcom.bdcautionmap.value(
						((*i).nodeid)), COMCAUTION);

					break;

				case 2://板子下线

					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("板子") + QString::number((*i).nodeid),
						"板子下线事件","板子下线事件");

					//对于网头，是否是故障，得看当前网头是否激活
					if ((*i).nodeid >= PRINT1BDID && (*i).nodeid <= PRINT16BDID)
					{
						if (pubcom.presetprintstatus[(*i).nodeid] != 1 )
						{
							//设置板子不在线故障
							publiccaution.addcaution(pubcom.bdcautionmap.value(
								((*i).nodeid)), COMCAUTION);
						}
					}
					else
					{
						//设置板子不在线故障
						publiccaution.addcaution(pubcom.bdcautionmap.value(
							((*i).nodeid)), COMCAUTION);
					}
					break;

				case 3://网络故障事件

					//产生事件，提醒用户注意
					publiccaution.addevent(
						"网络",
						"网络故障","网络故障事件");

					publiccaution.addcaution(CANCAUTIONLOCATION, CANNETCAUTION);
					break;

				default:
					break;
				}
			}
		}
		pubcom.eventqueue.clear();


		//运动卡故障产生
		int errnum = ksmc->CheckKSMCA4();

		pubcom.currenta4ncaution = pubcom.historya4ncaution;
		pubcom.currenta4ncaution.clear();

		for (int ii = 1; ii <= errnum; ii++)
		{
			int location = 0;
			int errcode = 0;
			if (ksmc->GetErrorLocation(ii, location)
				&& ksmc->GetErrorCode(ii, errcode))
			{
				ta4ncaution tempcaution;
				tempcaution.errcode = errcode;
				tempcaution.location = location;
				//如果原来没有,则新增故障
				if (!pubcom.historya4ncaution.contains(tempcaution))
				{
					switch (location)
					{
					case 0:
						publiccaution.addcaution(A4NCAUTIONLOCATION, errcode);
						break;
					default:
						publiccaution.addcaution(PRINT1LOCATION + location - 1,PRINTMOTORCAUTION);
						break;
					}
				}
				pubcom.currenta4ncaution.append(tempcaution);
			}
		}

		//运动卡故障消除
		QList<ta4ncaution>::iterator jj;

		for (jj = pubcom.historya4ncaution.begin(); jj
			!= pubcom.historya4ncaution.end(); ++jj)
		{
			if (!pubcom.currenta4ncaution.contains(*jj))
			{
				switch ((*jj).location)
				{
				case 0:
					publiccaution.delcaution(A4NCAUTIONLOCATION, (*jj).errcode);
				default:
					publiccaution.delcaution((*jj).location + PRINT1LOCATION -1 , (*jj).errcode);
				}
			}
		}

		//产生串口离线故障

#ifdef  HAVESERIAL
		if(publiccaution.hascaution(SERIAL,SERIALNOEXIST) == false)
		{
		unsigned char heartchar = 0;
		ReadRegsReadHeatbeat(&regs,&heartchar);

		//曹工100ms发送一次，也就是说4秒中没有收到串口数据就算是串口离线了
		if (heartchar > 10)
		{
		if (pubcom.OfflineFlag == false)
		{
		pubcom.OfflineFlag = true;
		publiccaution.addcaution(SERIAL,SERIALLOST);
		}

		}
		else//串口离线故障消除
		{
		if (pubcom.OfflineFlag == true)
		{
		pubcom.OfflineFlag = false;
		publiccaution.delcaution(SERIAL,SERIALLOST);
		}
		}
		}
#endif

		currentfollow = pubcom.Follow;

		if(currentfollow != oldcurrentfollow)
		{
			//当前跟随比不同则重新设置
			ksmc->SetFollowDelay(currentfollow);
			oldcurrentfollow = currentfollow;
			//printf("跟随比%f\n",currentfollow);
		}

		///////////////////////////////////////////////////////
		//外部CAN请求命令处理
		///////////////////////////////////////////////////////
		//各个板子故障上报应答
		tcmd tempcmd;
		tempcmd.cmd = COMTESTCMD;
		tempcmd.id = PCCANBDID;
		tempcmd.para1 = 0;
		tempcmd.para2 = 0;

		QList<int>::iterator i;

		////////////////////////////////////////////////////////////////////////////
		//磁台状态改变,磁台控制板向网头板发生CAN命令，工控机会监听，但无需应答
		////////////////////////////////////////////////////////////////////////////

		{
			for(int ii = 0; ii < pubcom.PrintNum; ++ii)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd(MAGICIOBDID, MAGICRSPCMD,ii,cmdlist);

				//如果有请求
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						pubcom.magicpercent[(*j).para1] = (*j).para2;
						//记忆磁力值，非0、1
						if((*j).para2 >= 2)
						{
							//保存到数据库中
							mydb.SaveValue(QString("magic") + QString::number(ii),(*j).para2);						
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头发送磁台命令,纯接收，无需处理
		////////////////////////////////////////////////////////////////////////////

		{
			for(int ii = 0; ii < pubcom.PrintNum; ++ii)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd(ii, MAGICRSPCMD,cmdlist);
				count = count;
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//浆泵状态上报命令，纯接收，无需处理
		////////////////////////////////////////////////////////////////////////////
		{
			for(int ii = 0; ii < pubcom.PrintNum; ++ii)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd(BUMPIOBDID, BUMPRSPCMD,cmdlist);
				count = count;
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头发送浆泵命令，纯接收，无需处理
		////////////////////////////////////////////////////////////////////////////
		{
			for(int ii = 0; ii < pubcom.PrintNum; ++ii)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd(ii, BUMPRSPCMD,cmdlist);
				count = count;
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头板机器命令应答：加减速
		////////////////////////////////////////////////////////////////////////////

		{
			//网头状态
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), CHGSPEEDCMD, 0x00, cmdlist);

				//如果有请求
				if (count >= 1)
				{
					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("网头") + QString::number(i+1),
						"加减速命令","收到网头板加减速命令",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						switch((*j).para2) {
							case 0x01:
								//加速ON
								WritePcMotorAccBit(&regs,true);
								break;

							case 0x02:
								//减速ON
								WritePcMotorDecBit(&regs,true);
								break;

							case 0x03:
								//加速OFF
								WritePcMotorAccBit(&regs,false);
								//减速OFF
								WritePcMotorDecBit(&regs,false);
								break;
						}

						//应答状态命令
						tempcmd.cmd = CHGSPEEDCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = (*j).para2;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(
								pubcom.bdnodemap.value(i), &tempcmd)
								== false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"发送CAN命令失败");
							}
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头板机器命令应答：对零
		////////////////////////////////////////////////////////////////////////////
		{
			//网头状态
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), ZEROCMD, 0x00,0x01,cmdlist);

				//如果有请求
				if (count >= 1)
				{
					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("网头") + QString::number(i+1),
						"对零命令","收到网头板对零命令",1);

					//原本就有对零动作或者有外部急停故障
					if ((pubcom.actionzerostep.value(i) != 0) || (publiccaution.hascaution(PLCLOCATION, PLCHALTCATION) == true))
					{
						//应答命令
						tempcmd.cmd = ZEROCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = 0x02;

						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(i),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"发送CAN命令失败");

							}
						}
					}
					else
					{
						//设置标识位
						pubcom.actionzerostep.insert(i, 1);

						//反馈消息给网头板，同一时间只能一个命令是有效的，多余的命令就按照成功执行返回

						//应答状态命令
						tempcmd.cmd = ZEROCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;

						//如果允许
						if (1)
						{
							tempcmd.para2 = 0x01;
						}
						else
						{
							tempcmd.para2 = 0x02;
						}
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(i),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"发送CAN命令失败");

							}
						}
					}
				}
			}	
		}

		////////////////////////////////////////////////////////////////////////////
		//网头板机器命令应答：反转
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), RESERVECMD, 0x00, cmdlist);

				//如果有请求
				if (count >= 1)
				{

					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("网头") + QString::number(i+1),
						"反转命令","收到网头板反转命令",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//是否允许动作执行 1允许 2拒绝
						char anscmd = 1;

						//动作执行，反转
						if ((*j).para2 == 0x01)
						{
							//判断是否在跟随状态，跟随状态不允许反转
							int motorstatus = -1;
							ksmc->GetStatus(i+1,motorstatus);
							//不在跟随状态才可以,并且没有外部急停故障
							if((motorstatus != 2) && (publiccaution.hascaution(PLCLOCATION, PLCHALTCATION) == false))
							{
								ksmc->MoveCCW((i) + 1);
								pubcom.circlestatus[i] = 1;
							}
							else
							{
								anscmd = 2;
							}
						}
						//停止转动
						else if ((*j).para2 == 0x02)
						{
							ksmc->MoveStop((i) + 1);
							pubcom.circlestatus[i] = 0;

							//启动网头位置恢复任务
							if(pubcom.restoreTask[i].step == 0)
								pubcom.restoreTask[i].step = 1;
							
                            //ksmc->RestorePrintLocation(i + 1);
						}

						//应答命令
						tempcmd.cmd = RESERVECMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = (*j).para2 | anscmd << 4;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(i),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"发送CAN命令失败");
							}
						}
					}
				}
			}
		}

		//恢复位置任务运行
		for(int iiii = 0;iiii < pubcom.PrintNum;iiii++)
		{
			restoreTaskRun(&(pubcom.restoreTask[iiii]));
		}


		////////////////////////////////////////////////////////////////////////////
		//网头板机器命令应答：纵向对花
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), YMOVECMD, 0x00, cmdlist);

				//如果有请求
				if (count >= 1)
				{
					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("网头") + QString::number(i+1),
						"纵向对花命令","收到网头板纵向对花命令",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//动作执行,纵向正向对花,根据原来的状态确定是否要执行指令
						if ((*j).para2 == 0x01 && 
							pubcom.movedirectstatus.value(i) == 0)
						{
							ksmc->MoveDirect((*j).id + 1, 1);
							printf("%d号纵向正向对花\n",(*j).id + 1);
							pubcom.movedirectstatus.insert(i,1);
							pubcom.movedirecttimer.insert(i,pubtimermng.addtimer());
						}
						//纵向反向对花,根据原来的状态确定是否要执行指令
						else if ((*j).para2 == 0x02 && 
							pubcom.movedirectstatus.value(i) == 0)
						{
							ksmc->MoveDirect((*j).id + 1,0);
							printf("%d号纵向反向对花\n",(*j).id + 1);
							pubcom.movedirectstatus.insert(i,2);
							pubcom.movedirecttimer.insert(i,pubtimermng.addtimer());
						}//停止转动,根据原来的状态确定是否要执行指令
						else if ((*j).para2 == 0x03)
						{
							ksmc->MoveStop((*j).id + 1);
							printf("%d号停止对花\n",(*j).id + 1);
							pubcom.movedirectstatus.insert(i,0);
							pubtimermng.deltimer(pubcom.movedirecttimer.value(i));
							
							//是否已经进入跟随状态
							int motorstatus = -1;
							ksmc->GetStatus((*j).id + 1,motorstatus);
							//如果没有进入跟随状态则记忆
							if(motorstatus != 2)
							{
								//启动记忆位置任务
								if(pubcom.savelocTask[(*j).id].step == 0)
								{
									pubcom.savelocTask[(*j).id].step = 1;
								}
								//ksmc->RememberPrintLoction((*j).id + 1);							
							}						
						}

						//应答命令
						tempcmd.cmd = YMOVECMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = (*j).para2 | 0x01 << 4;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(i),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"发送CAN命令失败");
							}
						}

					}
				}
			}
		}

		{

			//纵向对花慢速到快速
			for (int ii = PRINT1BDID; ii <= PRINT16BDID;++ii)
			{
				if (pubcom.movedirectstatus.value(ii) == 1 
					|| pubcom.movedirectstatus.value(ii) == 2)
				{
					if ( pubtimermng.gettimerval(pubcom.movedirecttimer.value(ii)) > pubcom.LowToHighSpeedtime)
					{
						if (pubcom.movedirectstatus.value(ii) == 1)
						{
							pubcom.movedirectstatus.insert(ii,3);
							ksmc->MoveDirectHigh( ii + 1, 1);

						}
						else if (pubcom.movedirectstatus.value(ii) == 2) 
						{
							pubcom.movedirectstatus.insert(ii,4);
							ksmc->MoveDirectHigh(ii + 1, 0);
						}
					}
				}

			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头状态上报应答
		////////////////////////////////////////////////////////////////////////////
		{
			//网头状态
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTWORKSTATUSCMD, 0x00,
					cmdlist);

				//如果有请求
				if (count >= 1)
				{

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//命令处理
						pubcom.printstatus[i] = (*j).para2;

						//应答状态命令
						tempcmd.cmd = PRINTWORKSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x00;
						tempcmd.para2 = (*j).para2;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(i),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"发送CAN命令失败");
							}
						}
					}
				}
			}	
		}

		////////////////////////////////////////////////////////////////////////////
		//网头对零完成，仅仅收命令，不作其他
		////////////////////////////////////////////////////////////////////////////
		{
			//网头状态
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				tcmd tempcmd;
				candevice.getcmd((i), ZEROCMD,0x00,0x02,
					&tempcmd);
			}	
		}

		////////////////////////////////////////////////////////////////////////////
		//网头IO状态上报应答
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTSWITCHSTATUSCMD, 0x00,
					cmdlist);

				//如果有请求
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//命令处理
						pubcom.lockreleasestatus[i] = ((*j).para2 >> 0) & 1;//紧/松网

						//要注意安全，下了网头就必须进入跟随状态
						pubcom.updownstatus[i] = ((*j).para2 >> 1) & 1;//网头上/下
						pubcom.platupdownstatus[i] = ((*j).para2 >> 2) & 1;//保留
						pubcom.colorpipestatus[i] = ((*j).para2 >> 3) & 1;//料杆锁/松

						if(((((*j).para2 >> 4) & 1) == 1) &&  (pubcom.magicstatus[i] == 0))
						{
							pubcom.MagicOnFlushStep[i] = 1;
						}

						pubcom.magicstatus[i] = ((*j).para2 >> 4) & 1;//磁台开/关
						pubcom.bumpstatus[i] = ((*j).para2 >> 5) & 3;//浆泵开/关
						pubcom.zerostatus[i] = ((*j).para2 >> 7) & 1;//是/否对零

						//应答状态命令
						tempcmd.cmd = PRINTSWITCHSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x00;
						tempcmd.para2 = (*j).para2;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头横向位置状态上报应答
		////////////////////////////////////////////////////////////////////////////
		{
			//网头状态
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTXSTATUSCMD, cmdlist);

				//如果有请求
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//命令处理
						pubcom.xstatus[i] = (((short) ((*j).para1)) << 8) + (*j).para2;

						//保存到数据库中
						mydb.SaveValue(QString("x") + QString::number(i),
							pubcom.xstatus[i]);

						//应答状态命令
						tempcmd.cmd = PRINTXSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = 0x01;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
					}
				}
			}	
		}


		////////////////////////////////////////////////////////////////////////////
		//网头斜向位置状态上报应答
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTYSTATUSCMD, cmdlist);

				//如果有请求
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//命令处理
						pubcom.ystatus[i] = (((short) ((*j).para1)) << 8) + (*j).para2;

						//保存到数据库中
						mydb.SaveValue(QString("y") + QString::number(i),
							pubcom.ystatus[i]);

						//应答状态命令
						tempcmd.cmd = PRINTYSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = 0x01;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头高度位置状态上报应答
		////////////////////////////////////////////////////////////////////////////
		{
			//网头状态
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTZSTATUSCMD, cmdlist);

				//如果有请求
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//命令处理
						pubcom.zstatus[i] = (((short) ((*j).para1)) << 8) + (*j).para2;

						//保存到数据库中
						mydb.SaveValue(QString("z") + QString::number(i),
							pubcom.zstatus[i]);

						//应答状态命令
						tempcmd.cmd = PRINTZSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = 0x01;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//主机速度广播
		////////////////////////////////////////////////////////////////////////////
		{
			if(pubcom.MotorSpeedChgFlag == true)
			{
				pubcom.MotorSpeedChgFlag = false;

				//应答状态命令
				tempcmd.cmd = QUERYSPEEDCMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0;
				tempcmd.para2 = (unsigned char)pubcom.motorspeed; //填主机速度
				if (candevice.sendcmd(ALLPRINTID,
					&tempcmd) == false)
				{
					//产生事件，提醒用户注意
					publiccaution.addevent(
						"广播命令",
						"速度广播命令","发送CAN命令失败");
				}

			}
		}

		////////////////////////////////////////////////////////////////////////////
		//工控机状态参数查询命令：整机状态
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), QUERYMACHSTATUSCMD, 0x00,
					0x01, cmdlist);

				//如果有请求
				if (count >= 1)
				{
					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("网头") + QString::number(i+1),
						"整机状态查询","收到网头板整机状态查询命令",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//应答状态命令
						tempcmd.cmd = QUERYMACHSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = pubcom.machprintstatus; //填主机状态
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
					}
				}
			}

		}


		////////////////////////////////////////////////////////////////////////////
		//工控机状态参数查询命令：横向零位位置参数
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), QUERYXSTATUSCMD, 0x00, 0x01,
					cmdlist);

				//如果有请求
				if (count >= 1)
				{
					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("网头") + QString::number(i+1),
						"横向零位位置参数","收到网头板横向零位位置参数命令",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//应答状态命令
						tempcmd.cmd = QUERYXSTATUSCMD;
						tempcmd.id = PCCANBDID | i;//比较特殊
						tempcmd.para1 = pubcom.xstatus[i] & 0xff;//填横向零位位置
						tempcmd.para2 = (pubcom.xstatus[i] >> 8) & 0xff; //填横向零位位置
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//工控机状态参数查询命令：读斜向零位位置参数
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), QUERYYSTATUSCMD, 0x00, 0x01,
					cmdlist);

				//如果有请求
				if (count >= 1)
				{
					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("网头") + QString::number(i+1),
						"读斜向零位位置","收到网头板读斜向零位位置命令",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//应答状态命令
						tempcmd.cmd = QUERYYSTATUSCMD;
						tempcmd.id = PCCANBDID | i;//比较特殊
						tempcmd.para1 = pubcom.ystatus[i] & 0xff;//填斜向零位位置
						tempcmd.para2 = (pubcom.ystatus[i] >> 8) & 0xff; //填斜向零位位置
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//工控机状态参数查询命令：读高度零位位置参数
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), QUERYZSTATUSCMD, 0x00, 0x01,
					cmdlist);

				//如果有请求
				if (count >= 1)
				{
					//产生事件，提醒用户注意
					publiccaution.addevent(
						QString("网头") + QString::number(i+1),
						"高度零位位置","收到网头板高度零位位置命令",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//应答状态命令
						tempcmd.cmd = QUERYZSTATUSCMD;
						tempcmd.id = PCCANBDID | i;//比较特殊
						tempcmd.para1 = pubcom.zstatus[i] & 0xff;//填斜向零位位置
						tempcmd.para2 = (pubcom.zstatus[i] >> 8) & 0xff; //填斜向零位位置
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
					}
				}
			}
		}




		////////////////////////////////////////////////////////////////////////////
		//PLC报警信号
		////////////////////////////////////////////////////////////////////////////
		{

			//上升沿产生
			if ( pubcom.PlcCauUpFlag == true  )
			{
				pubcom.PlcCauUpFlag = false;
				publiccaution.addcaution(PLCLOCATION, PLCCAUTION);

			}

			//下降沿消除
			if ( pubcom.PlcCauDownFlag == true )
			{
				pubcom.PlcCauDownFlag = false;
				publiccaution.delcaution(PLCLOCATION, PLCCAUTION);
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//PLC错误信号
		////////////////////////////////////////////////////////////////////////////
		{
			//上升沿产生
			if ( pubcom.PlcErrUpFlag == true )
			{
				pubcom.PlcErrUpFlag = false;
				publiccaution.addcaution(PLCLOCATION, PLCHALTCATION);

			}

			//下降沿消除
			if ( pubcom.PlcErrDownFlag == true )
			{
				pubcom.PlcErrDownFlag = false;
				publiccaution.delcaution(PLCLOCATION, PLCHALTCATION);
			}

		}


		///////////////////////////////////////////////////////////////////////////
		//过程处理//////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////

		//心跳测试无论网头板是否激活都得测试，但是是否是故障就和是否激活有关系了 
		//激活的才作为故障，没有激活的不是
		if (pubcom.heartbeatstep == 1)
		{
			pubcom.heartbeattimer = pubtimermng.addtimer();
			if(pubcom.heartbeatbegincnt > 3)
			{	
				pubcom.heartbeatstep = 2;
			}
			else
			{
				pubcom.heartbeatstep = 4;
			}
		}
		else if (pubcom.heartbeatstep == 2)
		{
			tcmd tempcmd;
			tempcmd.cmd = COMTESTCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = 0;
			tempcmd.para2 = 0;

			//初始化并发命令，如果发送失败，认为是CAN网络问题，进入失败计数
			if (candevice.sendcmd(ALLID, &tempcmd) == false)
			{
				//产生事件，提醒用户注意
				publiccaution.addevent(
					"广播命令",
					"广播到所有板子","发送CAN命令失败");

				pubcom.networkfailcount++;

				//退出心跳测试
				pubcom.heartbeatstep = 4;
			}
			else
			{
				if(publiccaution.hascaution(CANCAUTIONLOCATION,CANNETCAUTION))
				{
					publiccaution.addcaution(CANCAUTIONLOCATION, CANNETCAUTION);
				}
				pubcom.networkfailcount = 0; //网络计数复位
				pubcom.heartbeatstep = 3;

				//节点初始化
				QList<int>::iterator i;
				for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
				{
					heartbeatmap.insert(*i, false);
				}
			}
		}
		//接收命令
		else if (pubcom.heartbeatstep == 3)
		{
			//等待时间内
			if (pubtimermng.gettimerval(pubcom.heartbeattimer) <= OUTTIME)
			{
				//是否全部收到命令
				bool isallget = true;

				for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
				{
					if (heartbeatmap.value(*i) == false)
					{
						isallget = false;
						break;
					}
				}

				//全部收到就下一步
				if (isallget == true)
				{
					pubcom.heartbeatstep = 4;
				}
				//否则继续收
				else
				{
					tcmd tempcmd;
					tempcmd.cmd = COMTESTCMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = 0;
					tempcmd.para2 = 0;

					//接收命令
					QList<int>::iterator i;
					for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
					{
						QList<tcmd> cmdlist;
						int count = candevice.getcmd((*i), COMTESTCMD,
							cmdlist);

						//重复ID
						if (count > 1)
						{
							pubcom.nodedupidmap.insert(*i,pubcom.nodedupidmap.value(*i)+1);

							//清0
							pubcom.nodeonlinecountmap.insert(*i, 0);

							heartbeatmap.insert(*i, true);

							//上线事件
							if (pubcom.nodeonlinemap.value(*i) == false)
							{
								tintevent tempevent;
								tempevent.eventid = 1;
								tempevent.nodeid = *i;
								pubcom.eventqueue.append(tempevent);

								pubcom.nodeonlinemap.insert((*i), true);
							}

						}
						else if ( heartbeatmap.value(*i) == false 
							&& count == 1)
						{
							pubcom.nodedupidmap.insert(*i,0);

							//重复ID故障消除
							if (publiccaution.hascaution(pubcom.bdcautionmap.value((*i)), REPIDCAUTION))
							{
								publiccaution.delcaution(pubcom.bdcautionmap.value((*i)), REPIDCAUTION);
							}

							//清0
							pubcom.nodeonlinecountmap.insert(*i, 0);

							heartbeatmap.insert(*i, true);

							//上线事件
							if (pubcom.nodeonlinemap.value(*i) == false)
							{
								tintevent tempevent;
								tempevent.eventid = 1;
								tempevent.nodeid = *i;
								pubcom.eventqueue.append(tempevent);

								pubcom.nodeonlinemap.insert((*i), true);
							}
						}
					}

				}
			}
			else //等待超时
			{
				QList<int>::iterator i;
				for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
				{
					//超时没有收到信息，计数加1
					if (heartbeatmap.value(*i) == false)
					{
						pubcom.nodeonlinecountmap.insert(*i,
							pubcom.nodeonlinecountmap.value(*i) + 1);
					}
				}

				pubcom.heartbeatstep = 4;

				//产生事件，提醒用户注意
				publiccaution.addevent(
					"CAN通讯",
					"心跳测试超时","CAN通讯心跳测试超时");
			}
		}
		else if (pubcom.heartbeatstep == 4)//10秒时间才可以继续下一次测试，否则，太频繁会导致网络风暴
		{
			//时间内
			if (pubtimermng.gettimerval(pubcom.heartbeattimer) > HEARTBEATTIME)
			{
				pubtimermng.deltimer(pubcom.heartbeattimer);
				pubcom.heartbeatstep = 1;

				//启动后心跳测试要等待，使网络平静下来才真正做
				if(pubcom.heartbeatbegincnt <= 3)
				{
					pubcom.heartbeatbegincnt ++;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//网络故障，网络发送失败计数，如果到达3就认为网络失败，生成网络失败事件 
		//////////////////////////////////////////////////////////////////////////
		if (pubcom.networkfailcount >= pubcom.canfailedcnt)
		{
			if (publiccaution.hascaution(pubcom.bdcautionmap.value(PCCANBDID),
				CANNETCAUTION) == false)
			{
				tintevent tempevent;
				tempevent.eventid = 3;
				tempevent.nodeid = 0;
				pubcom.eventqueue.append(tempevent);
			}
			pubcom.networkfailcount = 0;
		}

		//////////////////////////////////////////////////////////////////////////
		//节点通信故障，失败7次就生成事件
		//////////////////////////////////////////////////////////////////////////
		{
			QList<int>::iterator i;
			for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
			{
				if (pubcom.nodeonlinecountmap.value(*i) >= pubcom.canfailedcnt)
				{
					pubcom.nodeonlinecountmap.insert(*i, 0);
					if (pubcom.nodeonlinemap.value(*i) == true)
					{
						pubcom.nodeonlinemap.insert(*i, false);

						tintevent tempevent;
						tempevent.eventid = 2;
						tempevent.nodeid = *i;
						pubcom.eventqueue.append(tempevent);
					}
				}
				if (pubcom.nodedupidmap.value(*i) >= pubcom.canfailedcnt )
				{
					pubcom.nodedupidmap.insert(*i,0);

					//重复ID故障产生
					publiccaution.addcaution(pubcom.bdcautionmap.value((*i)), REPIDCAUTION);

				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//数据刷新请求
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.FlushDataReqStep == 1)
		{
			WritePcFlushDataReqBit(&regs,true);
			pubcom.FlushDataReqTimer = pubtimermng.addtimer();
			pubcom.FlushDataReqStep = 2; 
		}
		else if (pubcom.FlushDataReqStep == 2)
		{
			//维持时间
			if (pubtimermng.gettimerval(pubcom.FlushDataReqTimer) > MODBUSBITHOLDTIME)
			{
				WritePcFlushDataReqBit(&regs,false);
				pubtimermng.deltimer(pubcom.FlushDataReqTimer);
				pubcom.FlushDataReqStep = 0; 
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//启动处理流程
		////////////////////////////////////////////////////////////////////////////

		if (pubcom.plcreqstartstep == 1)
		{

			QList<int>::iterator it;
			int workcnt = 0;

			//多线程保护
			pubcom.activeprintlistmutex.lockForRead();

			//初始化，激活的网头
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{

				//有工作状态的
				if (pubcom.presetprintstatus[*it] == 3)
				{
					workcnt++;
					break;
				}
			}
			pubcom.activeprintlistmutex.unlock();

			//如果没有激活的网头或者工作网头数量为0
			if ((pubcom.activeprintlist.count() == 0) || (workcnt == 0))
			{
				//答复
				pubcom.plcreqstartstep = 10;
				pubcom.printok = true;
			}
			else
			{
				pubcom.plcreqstartstep = 2;
			}
		}
		//初始化
		else if (pubcom.plcreqstartstep == 2)
		{
			//检查网头
			pubcom.printcmdansmap.clear();
			QList<int>::iterator it;

			//多线程保护
			pubcom.activeprintlistmutex.lockForRead();

			//初始化，激活的网头
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{

				//不是在准备状态才问启动与否
				if (pubcom.presetprintstatus[*it] == 3)
				{
					pubcom.printcmdansmap.insert(*it, false);
					pubcom.sendprintcmdmap.insert(*it, false);
					pubcom.printreadymap.insert(*it, false);
				}
			}
			pubcom.activeprintlistmutex.unlock();

			pubcom.plcreqstartstep = 3;
			pubcom.plcreqstartcount = 0;
		}
		else if (pubcom.plcreqstartstep == 3)//发送CAN命令到各个网头
		{
			if (pubcom.plcreqstartcount > 1)
			{
				pubcom.plcreqstartstep = 10;
				pubcom.printok = false;
			}
			else
			{
				QList<int>::iterator it;
				bool sendonesucceed = false;

				pubcom.plcreqstartcount++;

				tcmd tempcmd;
				tempcmd.cmd = STARTWORKCMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x00;
				tempcmd.para2 = 0x01;

				pubcom.activeprintlistmutex.lockForRead();

				//询问网头
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					if (pubcom.presetprintstatus[*it] != 4)
					{
						//没有成功发送命令
						if (pubcom.sendprintcmdmap.value(*it) == false)
						{
							//发送命令
							if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"发送CAN命令失败");

							}
							else
							{
								sendonesucceed = true;
								pubcom.sendprintcmdmap.insert(*it, true);
							}
						}
					}
				}
				pubcom.activeprintlistmutex.unlock();

				if (sendonesucceed == true)//至少一个发送成功,都要进入下一步接收命令反馈
				{
					pubcom.plcreqstartstep = 4;
					pubcom.plcreqstarttimer = pubtimermng.addtimer();
				}
				else//全部发送错误
				{
					//do nothing
				}
			}
		}
		else if (pubcom.plcreqstartstep == 4)//接受网头命令反馈
		{
			tcmd tempcmd;
			tempcmd.cmd = STARTWORKCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = 0x30;
			tempcmd.para2 = 0x01;

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//等返回
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				if (pubcom.presetprintstatus[*it] != 4)
				{

					//成功发送了命令
					if (pubcom.sendprintcmdmap.value(*it) == true
						&& pubcom.printcmdansmap.value(*it) == false)
					{
						//接收命令
						if (candevice.getcmd((*it), STARTWORKCMD, &tempcmd) == true)
						{
							//是否准备好
							if (tempcmd.para2 == 0x01)
							{
								pubcom.printreadymap.insert(*it, true);
							}
							pubcom.printcmdansmap.insert(*it, true);
						}
					}
				}
			}
			pubcom.activeprintlistmutex.unlock();

			//时间内
			if (pubtimermng.gettimerval(pubcom.plcreqstarttimer) <= OUTTIME)
			{
				//是否全部收到
				bool allget = true;

				QMap<int, bool>::iterator it;
				for (it = pubcom.sendprintcmdmap.begin(); it
					!= pubcom.sendprintcmdmap.end(); ++it)
				{
					if (it.value() == true && pubcom.printcmdansmap.value(
						it.key()) == false)
					{
						allget = false;
						break;
					}
				}
				if (allget == true)
				{
					pubtimermng.deltimer(pubcom.plcreqstarttimer);

					//接收失败标识
					bool hasrecvfail = false;

					for (it = pubcom.printcmdansmap.begin(); it
						!= pubcom.printcmdansmap.end(); ++it)
					{
						if (it.value() == false)
						{
							hasrecvfail = true;
							break;
						}
					}

					if (hasrecvfail == false)
					{
						pubcom.plcreqstartstep = 5; //没有失败的，那么进行下一步
					}
					else
					{
						pubcom.plcreqstartstep = 3; //重发

						for (it = pubcom.printcmdansmap.begin(); it
							!= pubcom.printcmdansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								//重置接收失败的发送标识，可以重新发送
								pubcom.sendprintcmdmap.insert(it.key(), false);
							}
						}
					}
				}
				else //没有全部都收到，继续等
				{
					//do nothing
				}
			}
			else //超时
			{
				QMap<int, bool>::iterator it;

				pubtimermng.deltimer(pubcom.plcreqstarttimer);

				//接收失败标识
				bool hasrecvfail = false;

				for (it = pubcom.printcmdansmap.begin(); it
					!= pubcom.printcmdansmap.end(); ++it)
				{
					if (it.value() == false)
					{
						hasrecvfail = true;
						break;
					}
				}

				if (hasrecvfail == false)
				{
					pubcom.plcreqstartstep = 5;
				}
				else
				{
					pubcom.plcreqstartstep = 3;
					for (it = pubcom.printcmdansmap.begin(); it
						!= pubcom.printcmdansmap.end(); ++it)
					{
						if (it.value() == false)
						{
							//重置接收失败的发送标识，可以重新发送
							pubcom.sendprintcmdmap.insert(it.key(), false);
						}
					}
				}
			}
		}
		else if (pubcom.plcreqstartstep == 5)//初始化
		{
			//检查网头
			pubcom.printcmdansmap.clear();
			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//初始化
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				if (pubcom.presetprintstatus[*it] == 3)
				{

					pubcom.printcmdansmap.insert(*it, false);
					pubcom.sendprintcmdmap.insert(*it, false);
				}
			}
			pubcom.activeprintlistmutex.unlock();

			pubcom.plcreqstartstep = 6;
		}
		else if (pubcom.plcreqstartstep == 6)
		{
			//网头是否全部准备好
			bool printok = true;
			QMap<int, bool>::iterator it;

			for (it = pubcom.printreadymap.begin(); it
				!= pubcom.printreadymap.end(); ++it)
			{
				if (it.value() == false)
				{
					printok = false;
					break;
				}
			}

			if (printok == false)
			{
				pubcom.plcreqstartstep = 10;//网头没有准备,停止这个过程了
				pubcom.printok = false;
			}
			else//网头都是准备好了
			{
				pubcom.plcreqstartstep = 7;
			}
		}
		else if (pubcom.plcreqstartstep == 7)//网头状态切换初始化
		{
			//检查网头
			pubcom.printcmdansmap.clear();

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//初始化
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				if (pubcom.presetprintstatus[*it] == 3)
				{

					pubcom.printcmdansmap.insert(*it, false);
					pubcom.sendprintcmdmap.insert(*it, false);
					pubcom.printreadymap.insert(*it, false);
				}
			}
			pubcom.activeprintlistmutex.unlock();

			pubcom.plcreqstartstep = 8;

			pubcom.plcreqstartcount = 0;
		}
		else if (pubcom.plcreqstartstep == 8)//状态切换命令发送
		{
			bool sendonesucceed = false;

			tcmd tempcmd;

			QList<int>::iterator it;
			if (pubcom.plcreqstartcount > 1)
			{
				pubcom.printok = false;
				pubcom.plcreqstartstep = 10;//网头没有准备,停止这个过程了
			}
			else
			{
				pubcom.plcreqstartcount++;

				tempcmd.cmd = SETMACHSTATUSCMD;//状态切换命令
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x00;
				tempcmd.para2 = 0x05; //印花工作状态

				pubcom.activeprintlistmutex.lockForRead();

				//网头状态切换
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					if (pubcom.presetprintstatus[*it] == 3)
					{

						//没有成功发送命令
						if (pubcom.sendprintcmdmap.value(*it) == false)
						{
							tempcmd.para1 = *it;

							//发送命令
							if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"发送CAN命令失败");

							}
							else
							{
								sendonesucceed = true;
								pubcom.sendprintcmdmap.insert(*it, true);
							}
						}
					}
				}
				pubcom.activeprintlistmutex.unlock();

				if (sendonesucceed == true)
				{
					pubcom.plcreqstarttimer = pubtimermng.addtimer();
					pubcom.plcreqstartstep = 9;
				}
				else
				{
					//do nothing
				}
			}
		}
		else if (pubcom.plcreqstartstep == 9)//状态命令反馈
		{
			tcmd tempcmd;
			tempcmd.cmd = SETMACHSTATUSCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = 0x00;
			tempcmd.para2 = 0x01;

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//等返回
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				if (pubcom.presetprintstatus[*it] != 4)
				{

					//成功发送了命令
					if (pubcom.sendprintcmdmap.value(*it) == true
						&& pubcom.printcmdansmap.value(*it) == false)
					{
						//接收命令
						if (candevice.getcmd((*it), SETMACHSTATUSCMD, &tempcmd)
							== true)
						{
							//正确的进行了状态转换
							//if (tempcmd.para1 == 0x05 
							//	&& tempcmd.para2 == 0x01)
							if (tempcmd.para2 == 0x01)
							{
								pubcom.printreadymap.insert(*it, true);
							}
							pubcom.printcmdansmap.insert(*it, true);
						}
					}
				}
			}
			pubcom.activeprintlistmutex.unlock();

			//时间内
			if (pubtimermng.gettimerval(pubcom.plcreqstarttimer) <= OUTTIME)
			{
				//是否全部收到
				bool allget = true;

				QMap<int, bool>::iterator it;
				for (it = pubcom.sendprintcmdmap.begin(); it
					!= pubcom.sendprintcmdmap.end(); ++it)
				{
					if (it.value() == true && pubcom.printcmdansmap.value(
						it.key()) == false)
					{
						allget = false;
						break;
					}
				}
				if (allget == true)
				{
					pubtimermng.deltimer(pubcom.plcreqstarttimer);

					//接收失败标识
					bool hasrecvfail = false;

					for (it = pubcom.printcmdansmap.begin(); it
						!= pubcom.printcmdansmap.end(); ++it)
					{
						if (it.value() == false)
						{
							hasrecvfail = true;
							break;
						}
					}

					if (hasrecvfail == true)
					{
						pubcom.plcreqstartstep = 8; //继续发送

						for (it = pubcom.printcmdansmap.begin(); it
							!= pubcom.printcmdansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								//重置接收失败的发送标识，可以重新发送
								pubcom.sendprintcmdmap.insert(it.key(), false);
							}
						}
					}
					else
					{
						//网头是否准备好
						bool printok = true;
						QMap<int, bool>::iterator it;

						for (it = pubcom.printreadymap.begin(); it
							!= pubcom.printreadymap.end(); ++it)
						{
							if (it.value() == false)
							{
								printok = false;
								break;
							}
						}

						if (printok == false)
						{
							pubcom.printok = false;
						}
						else
						{
							pubcom.printok = true;
						}


						pubcom.plcreqstartstep = 10; //下一步
					}

				}
				else
				{
					//do nothing
				}
			}
			else //超时
			{
				pubtimermng.deltimer(pubcom.plcreqstarttimer);

				//接收失败标识
				bool hasrecvfail = false;

				QMap<int, bool>::iterator it;

				for (it = pubcom.printcmdansmap.begin(); it
					!= pubcom.printcmdansmap.end(); ++it)
				{
					if (it.value() == false)
					{
						hasrecvfail = true;
						break;
					}
				}

				if (hasrecvfail == true)
				{
					pubcom.plcreqstartstep = 8;
					for (it = pubcom.printcmdansmap.begin(); it
						!= pubcom.printcmdansmap.end(); ++it)
					{
						if (it.value() == false)
						{
							//重置接收失败的发送标识，可以重新发送
							pubcom.sendprintcmdmap.insert(it.key(), false);
						}
					}
				}
				else
				{
					//网头是否准备好
					bool printok = true;
					QMap<int, bool>::iterator it;

					for (it = pubcom.printreadymap.begin(); it
						!= pubcom.printreadymap.end(); ++it)
					{
						if (it.value() == false)
						{
							printok = false;
							break;
						}
					}

					if (printok == false)
					{
						pubcom.printok = false;
					}
					else
					{
						pubcom.printok = true;
					}


					pubcom.plcreqstartstep = 10;
				}
			}
		}
		else if (pubcom.plcreqstartstep == 10)//机器状态改变初始化
		{
			//设置是否可以启动
			if (pubcom.printok == true)
			{
				//进入运行状态
				pubcom.machprintstatus = 2;

				QList<int>::iterator it;

				QDomNode para;
				int paraVal1 = 0;
				int paraVal2 = 0;
				int paraVal3 = 0;
				int paraVal4 = 0;
				bool ok;

				para = GetParaByName("driver", "第一位置增益");
				paraVal1 = para.firstChildElement("value").text().toInt(&ok);

				para = GetParaByName("driver", "第一速度增益");
				paraVal2 = para.firstChildElement("value").text().toInt(&ok);


				para = GetParaByName("driver", "第一速度积分时间常数");
				paraVal3 = para.firstChildElement("value").text().toInt(&ok);

				para = GetParaByName("driver", "第一速度检测过滤");
				paraVal4 = para.firstChildElement("value").text().toInt(&ok);

				pubcom.activeprintlistmutex.lockForRead();

				//跟随
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					if (pubcom.presetprintstatus[*it] == 3)
					{
						//是否已经进入跟随状态
						int motorstatus = -1;
						ksmc->GetStatus(*it +1,motorstatus);
						//如果没有进入跟随状态则进入跟随状态
						if(motorstatus != 2)
						{
							//printf("进入跟随%d\n",*it + 1);

							//设置增益
							ksmc->SetDriverPar(*it+1,16,paraVal1);
							ksmc->SetDriverPar(*it+1,17,paraVal2);
							ksmc->SetDriverPar(*it+1,18,paraVal3);
							ksmc->SetDriverPar(*it+1,19,paraVal4);

							ksmc->EnableMasterFollowing(*it + 1);
						}
					}
				}

				pubcom.activeprintlistmutex.unlock();

				WritePcEnableStartBit(&regs,true);

				publiccaution.adddbginfo(QString("可以印花"));
				pubcom.plcreqstarttimer = pubtimermng.addtimer();
				pubcom.plcreqstartstep = 11;
			}
			else
			{
				Dbgout("状态：%d",pubcom.machprintstatus);
				publiccaution.adddbginfo(QString("不能印花"));
				pubcom.plcreqstartstep = 0;
			}
		}
		else if (pubcom.plcreqstartstep == 11)
		{
			if (pubtimermng.gettimerval(pubcom.plcreqstarttimer) > MODBUSBITHOLDTIME)
			{
				WritePcEnableStartBit(&regs,false);

				pubtimermng.deltimer(pubcom.plcreqstarttimer);
				pubcom.plcreqstartstep = 0;
			}
		}


		//主界面命令过程处理：对零,反转,网头下放/上升,磁台开启关闭
		if (pubcom.ismainuicmd == true)
		{
			bool sendonesucceed = false;

			if (pubcom.mainuicmdstep == 1)//初始化
			{
				//接收
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//只有网头不在准备状态才发命令
					if (pubcom.presetprintstatus[*it] != 4)
					{
						pubcom.uicmdtoprintansmap.insert(*it, false);
						pubcom.senduicmdtoprintmap.insert(*it, false);
					}
				}
				pubcom.activeprintlistmutex.unlock();

				pubcom.mainuicmdcount = 0;
				pubcom.mainuicmdstep = 2;
			}
			else if (pubcom.mainuicmdstep == 2)//发送
			{
				if (pubcom.mainuicmdcount > 1)
				{
					//反馈给界面
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x02;
					ptempevent->status = 0x03;

					ptempevent->data[0] = pubcom.mainuidcmd;

					//发送一个事件给界面线程
					QCoreApplication::postEvent(interfaceobject, ptempevent);

					pubcom.mainuicmdstep = 0;
					pubcom.ismainuicmd = false;
					pubcom.mainuidcmd = 0;
				}
				else
				{
					pubcom.mainuicmdcount++;

					QList<int>::iterator it;

					pubcom.activeprintlistmutex.lockForRead();

					//只有激活的才发送命令
					for (it = pubcom.activeprintlist.begin(); it
						!= pubcom.activeprintlist.end(); ++it)
					{
						//只有网头不再准备状态才发命令
						if (pubcom.presetprintstatus[*it] != 4)
						{
							if (pubcom.senduicmdtoprintmap.value(*it) == false)
							{
								tcmd tempcmd;

								//对零要单独操作
								if (pubcom.mainuidcmd == 0x53)
								{
									tempcmd.cmd = REQZEROCMD;//网头操作命令
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = 0x01;
								}
								//反转要单独操作
								else if (pubcom.mainuidcmd == 0x54)
								{
									tempcmd.cmd = REQCIRCLECMD;//网头操作命令
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = 0x01;
								}
								else
								{
									tempcmd.cmd = PRINTOPCMD;//网头操作命令
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = pubcom.mainuidcmd;//小命令
								}

								if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
									&tempcmd) == false)
								{
									msleep(1);
									if (candevice.sendcmd(pubcom.bdnodemap.value(
										*it), &tempcmd) == false)
									{
										//产生事件，提醒用户注意
										publiccaution.addevent(
											pubcom.bdcautionmap.value(*it),
											SENDCMDFAILEVENT,"发送CAN命令失败");
									}
									else
									{
										pubcom.senduicmdtoprintmap.insert(*it, true);
										sendonesucceed = true;
									}
								}
								else
								{
									pubcom.senduicmdtoprintmap.insert(*it, true);
									sendonesucceed = true;
								}
							}
						}
					}
					pubcom.activeprintlistmutex.unlock();

					//只要有一个发送成功，就整体进入第二步
					if (sendonesucceed == true)
					{
						pubcom.mainuicmdstep = 3;
						pubcom.mainuidcmdtimer = pubtimermng.addtimer();
					}
					else//没有一个发送成功
					{
						//do nothing
					}
				}
			}
			else if (pubcom.mainuicmdstep == 3)//接收
			{
				bool allget = true;
				//接收
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//只有网头不再准备状态才发命令
					if (pubcom.presetprintstatus[*it] != 4)
					{
						tcmd tempcmd;

						if (pubcom.senduicmdtoprintmap.value(*it) == true
							&& pubcom.uicmdtoprintansmap.value(*it) == false)
						{
							//对零
							if (pubcom.mainuidcmd == 0x53)
							{
								if (candevice.getcmd((*it), REQZEROCMD, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//反转
							else if (pubcom.mainuidcmd == 0x54)
							{
								if (candevice.getcmd((*it), REQCIRCLECMD, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							else
							{
								if (candevice.getcmd((*it), PRINTOPCMD, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
						}
					}
				}
				pubcom.activeprintlistmutex.unlock();

				if (pubtimermng.gettimerval(pubcom.mainuidcmdtimer) <= OUTTIME)
				{
					QMap<int, bool>::iterator it;
					for (it = pubcom.senduicmdtoprintmap.begin(); it
						!= pubcom.senduicmdtoprintmap.end(); ++it)
					{
						if (it.value() == true
							&& pubcom.uicmdtoprintansmap.value(it.key())
							== false)
						{
							allget = false;
							break;
						}
					}
					if (allget == true)
					{
						pubtimermng.deltimer(pubcom.mainuidcmdtimer);

						bool hasrecvfail = false;

						for (it = pubcom.uicmdtoprintansmap.begin(); it
							!= pubcom.uicmdtoprintansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								hasrecvfail = true;
								break;
							}
						}
						if (hasrecvfail == true)
						{
							pubcom.mainuicmdstep = 2;
							for (it = pubcom.uicmdtoprintansmap.begin(); it
								!= pubcom.uicmdtoprintansmap.end(); ++it)
							{
								if (it.value() == false)
								{
									//重置接收失败的发送标识，可以重新发送
									pubcom.senduicmdtoprintmap.insert(it.key(),
										false);
								}
							}
						}
						else
						{
							//反馈给界面
							backendevent* ptempevent = new backendevent();
							ptempevent->cmd = 0x02;
							ptempevent->status = 0x01;
							ptempevent->data[0] = pubcom.mainuidcmd;

							//发送一个事件给界面线程
							QCoreApplication::postEvent(interfaceobject,
								ptempevent);
							pubcom.ismainuicmd = false;
							pubcom.mainuicmdstep = 0;

						}
					}
					else
					{
						//do nothing
					}
				}
				else
				{
					QMap<int, bool>::iterator it;

					pubtimermng.deltimer(pubcom.mainuidcmdtimer);
					//接收失败标识
					bool hasrecvfail = false;

					for (it = pubcom.uicmdtoprintansmap.begin(); it
						!= pubcom.uicmdtoprintansmap.end(); ++it)
					{
						if (it.value() == false)
						{
							hasrecvfail = true;
							break;
						}
					}

					if (hasrecvfail == true)
					{
						pubcom.mainuicmdstep = 2;

						for (it = pubcom.uicmdtoprintansmap.begin(); it
							!= pubcom.uicmdtoprintansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								//重置接收失败的发送标识，可以重新发送
								pubcom.senduicmdtoprintmap.insert(it.key(),
									false);
							}
						}
					}
					else
					{
						//反馈给界面
						backendevent* ptempevent = new backendevent();
						ptempevent->cmd = 0x02;
						ptempevent->status = 0x01;
						ptempevent->data[0] = pubcom.mainuidcmd;

						//发送一个事件给界面线程
						QCoreApplication::postEvent(interfaceobject, ptempevent);
						pubcom.ismainuicmd = false;
						pubcom.mainuicmdstep = 0;
					}
				}
			}
		}


		//PLC命令过程处理：对零,反转,网头下放/上升,磁台开启关闭
		if (pubcom.isplccmd == true)
		{
			bool sendonesucceed = false;

			if (pubcom.plccmdstep == 1)//初始化
			{
				//接收
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//只有网头不在准备状态才发命令
					if (pubcom.presetprintstatus[*it] != 4)
					{
						pubcom.uicmdtoprintansmap.insert(*it, false);
						pubcom.senduicmdtoprintmap.insert(*it, false);
					}
				}
				pubcom.activeprintlistmutex.unlock();

				pubcom.plccmdcount = 0;
				pubcom.plccmdstep = 2;
			}
			else if (pubcom.plccmdstep == 2)//发送
			{
				if (pubcom.plccmdcount > 1)
				{
					pubcom.plccmdstep = 0;
					pubcom.isplccmd = false;
					pubcom.plcdcmd = 0;
				}
				else
				{
					pubcom.plccmdcount++;

					QList<int>::iterator it;

					pubcom.activeprintlistmutex.lockForRead();

					//只有激活的才发送命令
					for (it = pubcom.activeprintlist.begin(); it
						!= pubcom.activeprintlist.end(); ++it)
					{
						//只有网头不再准备状态才发命令
						if (pubcom.presetprintstatus[*it] != 4)
						{
							if (pubcom.senduicmdtoprintmap.value(*it) == false)
							{
								tcmd tempcmd;

								//对零要单独操作
								if (pubcom.plcdcmd == 0x53)
								{
									tempcmd.cmd = REQZEROCMD;//网头操作命令
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = 0x01;
								}
								//反转要单独操作
								else if (pubcom.plcdcmd == 0x54)
								{
									tempcmd.cmd = REQCIRCLECMD;//网头操作命令
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = 0x01;
								}
								//磁力大小改变要单独操作
								else if (pubcom.plcdcmd == 0x58)
								{
									tempcmd.cmd = CHGMAGICCMD;//网头操作命令
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = regs.readarray[3];//磁力大小
								}
								//磁力自动模式开启
								else if (pubcom.plcdcmd == 0x59)
								{
									tempcmd.cmd = CHGMAGICMODE;//修改磁力模式
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = 1;//自动开启
								}
								//磁力自动模式关闭
								else if (pubcom.plcdcmd == 0x60)
								{
									tempcmd.cmd = CHGMAGICMODE;//修改磁力模式
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = 0;//自动关闭
								}
								else
								{
									tempcmd.cmd = PRINTOPCMD;//网头操作命令
									tempcmd.id = PCCANBDID;//can卡
									tempcmd.para1 = *it;//目标网头卡板
									tempcmd.para2 = pubcom.plcdcmd;//小命令
								}

								if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
									&tempcmd) == false)
								{
									msleep(1);
									if (candevice.sendcmd(pubcom.bdnodemap.value(
										*it), &tempcmd) == false)
									{
										//产生事件，提醒用户注意
										publiccaution.addevent(
											pubcom.bdcautionmap.value(*it),
											SENDCMDFAILEVENT,"发送CAN命令失败");
									}
									else
									{
										pubcom.senduicmdtoprintmap.insert(*it, true);
										sendonesucceed = true;
									}
								}
								else
								{
									pubcom.senduicmdtoprintmap.insert(*it, true);
									sendonesucceed = true;
								}
							}
						}
					}
					pubcom.activeprintlistmutex.unlock();

					//只要有一个发送成功，就整体进入第二步
					if (sendonesucceed == true)
					{
						pubcom.plccmdstep = 3;
						pubcom.plcdcmdtimer = pubtimermng.addtimer();
					}
					else//没有一个发送成功
					{
						//do nothing
					}
				}
			}
			else if (pubcom.plccmdstep == 3)//接收
			{
				bool allget = true;
				//接收
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//只有网头不再准备状态才发命令
					if (pubcom.presetprintstatus[*it] != 4)
					{
						tcmd tempcmd;

						if (pubcom.senduicmdtoprintmap.value(*it) == true
							&& pubcom.uicmdtoprintansmap.value(*it) == false)
						{
							//对零
							if (pubcom.plcdcmd == 0x53)
							{
								if (candevice.getcmd((*it), REQZEROCMD, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//反转
							else if (pubcom.plcdcmd == 0x54)
							{
								if (candevice.getcmd((*it), REQCIRCLECMD, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//磁力大小修改
							else if (pubcom.plcdcmd == 0x58)
							{
								if (candevice.getcmd((*it), CHGMAGICCMD, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//磁力模式修改
							else if (pubcom.plcdcmd == 0x59)
							{
								if (candevice.getcmd((*it), CHGMAGICMODE, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//磁力模式修改
							else if (pubcom.plcdcmd == 0x60)
							{
								if (candevice.getcmd((*it), CHGMAGICMODE, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							else
							{
								if (candevice.getcmd((*it), PRINTOPCMD, &tempcmd)
									== true)
								{
									//成功收到命令应答
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
						}
					}
				}
				pubcom.activeprintlistmutex.unlock();

				if (pubtimermng.gettimerval(pubcom.plcdcmdtimer) <= OUTTIME)
				{
					QMap<int, bool>::iterator it;
					for (it = pubcom.senduicmdtoprintmap.begin(); it
						!= pubcom.senduicmdtoprintmap.end(); ++it)
					{
						if (it.value() == true
							&& pubcom.uicmdtoprintansmap.value(it.key())
							== false)
						{
							allget = false;
							break;
						}
					}
					if (allget == true)
					{
						pubtimermng.deltimer(pubcom.plcdcmdtimer);

						bool hasrecvfail = false;

						for (it = pubcom.uicmdtoprintansmap.begin(); it
							!= pubcom.uicmdtoprintansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								hasrecvfail = true;
								break;
							}
						}
						if (hasrecvfail == true)
						{
							pubcom.plccmdstep = 2;
							for (it = pubcom.uicmdtoprintansmap.begin(); it
								!= pubcom.uicmdtoprintansmap.end(); ++it)
							{
								if (it.value() == false)
								{
									//重置接收失败的发送标识，可以重新发送
									pubcom.senduicmdtoprintmap.insert(it.key(),
										false);
								}
							}
						}
						else
						{
							pubcom.isplccmd = false;
							pubcom.plccmdstep = 0;

						}
					}
					else
					{
						//do nothing
					}
				}
				else
				{
					QMap<int, bool>::iterator it;

					pubtimermng.deltimer(pubcom.plcdcmdtimer);
					//接收失败标识
					bool hasrecvfail = false;

					for (it = pubcom.uicmdtoprintansmap.begin(); it
						!= pubcom.uicmdtoprintansmap.end(); ++it)
					{
						if (it.value() == false)
						{
							hasrecvfail = true;
							break;
						}
					}

					if (hasrecvfail == true)
					{
						pubcom.plccmdstep = 2;

						for (it = pubcom.uicmdtoprintansmap.begin(); it
							!= pubcom.uicmdtoprintansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								//重置接收失败的发送标识，可以重新发送
								pubcom.senduicmdtoprintmap.insert(it.key(),
									false);
							}
						}
					}
					else
					{
						pubcom.isplccmd = false;
						pubcom.plccmdstep = 0;
					}
				}
			}
		}

		//磁台上磁刷新初始值
		if(pubcom.isplccmd == false)
		{
			for(int ii = 0;ii <16;ii++ )
			{
				//发送
				if(pubcom.MagicOnFlushStep[ii] == 1)
				{
					int magicval;
					mydb.GetValue(QString("magic") + QString::number(ii) ,magicval);

					//防止磁力为0
					if(magicval == 0)
						magicval = 1;

					tcmd tempcmd;
					tempcmd.cmd = CHGMAGICCMD;//网头操作命令
					tempcmd.id = PCCANBDID;//can卡
					tempcmd.para1 = ii;//目标网头卡板
					tempcmd.para2 = magicval;//磁力大小

					if (candevice.sendcmd(pubcom.bdnodemap.value(ii),
						&tempcmd) == false)
					{
						msleep(1);
						if (candevice.sendcmd(pubcom.bdnodemap.value(
							ii), &tempcmd) == false)
						{
							//任务停止
							pubcom.MagicOnFlushStep[ii] = 0;

							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(ii),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
						else
						{
							pubcom.MagicOnFlushStep[ii] = 2;
							pubcom.MagicOnFlushTimer[ii] = pubtimermng.addtimer();

						}
					}
					else
					{
						pubcom.MagicOnFlushStep[ii] = 2;
						pubcom.MagicOnFlushTimer[ii] = pubtimermng.addtimer();
					}
				}//接收
				else if(pubcom.MagicOnFlushStep[ii] == 2)
				{
					if (pubtimermng.gettimerval(pubcom.MagicOnFlushTimer[ii]) <= OUTTIME)
					{
						tcmd tempcmd;
						if (candevice.getcmd((ii), CHGMAGICCMD, &tempcmd)
							== true)
						{
							pubcom.MagicOnFlushStep[ii] = 0;
							pubtimermng.deltimer(pubcom.MagicOnFlushTimer[ii]);
						}
					}
					else
					{
						pubcom.MagicOnFlushStep[ii] = 0;
						pubtimermng.deltimer(pubcom.MagicOnFlushTimer[ii]);
					}
				}
			}
		}


		//网头操作界面命令
		if (pubcom.opuicmdflag == true)
		{
			//初始化
			if (pubcom.opuicmdstep == 1)
			{
				pubcom.opuicmdtimer = 0;
				pubcom.opuicmdcount = 0;
				pubcom.opuicmdstep = 2;
			}
			else if (pubcom.opuicmdstep == 2)
			{
				if (pubcom.opuicmdcount > 1)
				{
					pubcom.opuicmdflag = false;
					pubcom.opuicmdtimer = 0;
					pubcom.opuicmdcount = 0;

					//反馈信息给界面
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x03;
					ptempevent->status = 0x02;
					ptempevent->data[0] = pubcom.opuicmdcmd;
					ptempevent->data[1] = pubcom.opuicmdprintno;

					//发送一个事件给界面线程
					QCoreApplication::postEvent(opobject, ptempevent);

				}
				else
				{
					pubcom.opuicmdcount++;
					tcmd tempcmd;

					//对零命令特殊处理，格式不一致
					if (pubcom.opuicmdcmd == REQZEROCMD)
					{
						tempcmd.cmd = REQZEROCMD;//网头操作命令
						tempcmd.id = PCCANBDID;//can卡
						tempcmd.para1 = pubcom.opuicmdprintno;//目标网头卡板
						tempcmd.para2 = 0x01;//小命令
					}
					else if (pubcom.opuicmdcmd == REQCIRCLECMD)
					{
						tempcmd.cmd = REQCIRCLECMD;//网头操作命令
						tempcmd.id = PCCANBDID;//can卡
						tempcmd.para1 = pubcom.opuicmdprintno;//目标网头卡板
						tempcmd.para2 = 0x01;//小命令
					}
					else
					{
						tempcmd.cmd = PRINTOPCMD;//网头操作命令
						tempcmd.id = PCCANBDID;//can卡
						tempcmd.para1 = pubcom.opuicmdprintno;//目标网头卡板
						tempcmd.para2 = pubcom.opuicmdcmd;//小命令
					}

					if (candevice.sendcmd(
						pubcom.bdnodemap.value(tempcmd.para1), &tempcmd)
						== false)
					{
						msleep(1);
						if (candevice.sendcmd(pubcom.bdnodemap.value(
							tempcmd.para1), &tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(tempcmd.para1),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
						else
						{
							pubcom.opuicmdstep = 3;
							pubcom.opuicmdtimer = pubtimermng.addtimer();
						}
					}
					else
					{

						pubcom.opuicmdstep = 3;
						pubcom.opuicmdtimer = pubtimermng.addtimer();
					}
				}
			}
			else if (pubcom.opuicmdstep == 3)//接收
			{
				bool recvsucceed = false;
				tcmd tempcmd;

				//对零命令特殊处理
				if (pubcom.opuicmdcmd == REQZEROCMD)
				{
					if (candevice.getcmd(pubcom.opuicmdprintno, REQZEROCMD,
						&tempcmd) == true)
					{
						//成功收到命令应答
						pubtimermng.deltimer(pubcom.opuicmdtimer);
						pubcom.opuicmdflag = false;
						pubcom.opuicmdstep = 0;

						//反馈信息给界面
						backendevent* ptempevent = new backendevent();
						ptempevent->cmd = 0x03;
						ptempevent->status = 0x01;
						ptempevent->data[0] = pubcom.opuicmdcmd;
						ptempevent->data[1] = pubcom.opuicmdprintno;

						//发送一个事件给界面线程
						QCoreApplication::postEvent(opobject, ptempevent);

						recvsucceed = true;
					}
				}
				else
				{
					if (candevice.getcmd(pubcom.opuicmdprintno, PRINTOPCMD,
						&tempcmd) == true)
					{
						//成功收到命令应答
						pubtimermng.deltimer(pubcom.opuicmdtimer);
						pubcom.opuicmdflag = false;
						pubcom.opuicmdstep = 0;

						//反馈信息给界面
						backendevent* ptempevent = new backendevent();
						ptempevent->cmd = 0x03;
						ptempevent->status = 0x01;
						ptempevent->data[0] = pubcom.opuicmdcmd;
						ptempevent->data[1] = pubcom.opuicmdprintno;

						//发送一个事件给界面线程
						QCoreApplication::postEvent(opobject, ptempevent);

						recvsucceed = true;
					}
				}

				if (pubtimermng.gettimerval(pubcom.opuicmdtimer) > OUTTIME)
				{
					//超时
					pubtimermng.deltimer(pubcom.opuicmdtimer);

					if (recvsucceed == false)
					{
						pubcom.opuicmdstep = 2;
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头状态选择
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.printactiveflag == true)
		{
			//初始化
			if (pubcom.printactivestep == 1)
			{
				pubcom.printactivecount = 0;
				pubcom.printactivetimer = 0;
				pubcom.printactivestep = 2;
			}
			else if (pubcom.printactivestep == 2)
			{
				if (pubcom.printactivecount > 1)
				{
					pubcom.printactiveflag = false;
					pubcom.printactivecount = 0;

					//反馈给界面
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x07;//参数应用命令
					ptempevent->status = 0x02;//命令状态，失败
					ptempevent->data[0] = pubcom.printactivenum;

					//发送一个事件给界面线程
					QCoreApplication::postEvent(selectprintobject, ptempevent);
				}
				else
				{
					//发送命令
					tcmd tempcmd;
					pubcom.printactivecount++;

					tempcmd.cmd = SETMACHSTATUSCMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = pubcom.printactivenum;

					//准备状态
					tempcmd.para2 = pubcom.printactivecmd;

					if (candevice.sendcmd(pubcom.bdnodemap.value(
						pubcom.printactivenum), &tempcmd) == false)
					{
						msleep(1);
						if (candevice.sendcmd(pubcom.bdnodemap.value(
							pubcom.printactivenum), &tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								pubcom.bdcautionmap.value(pubcom.printactivenum),
								SENDCMDFAILEVENT,"发送CAN命令失败");
						}
						else
						{
							pubcom.printactivestep = 3;
							pubcom.printactivetimer = pubtimermng.addtimer();
						}
					}
					else
					{
						pubcom.printactivestep = 3;
						pubcom.printactivetimer = pubtimermng.addtimer();
					}
				}
			}
			else if (pubcom.printactivestep == 3)
			{
				QList<tcmd> cmdlist;

				int count = 0;

				count = candevice.getcmd(pubcom.printactivenum,
					SETMACHSTATUSCMD, cmdlist);

				//如果有应答
				if (count >= 1)
				{
					pubtimermng.deltimer(pubcom.printactivetimer);
					pubcom.printactiveflag = false;
					pubcom.printactivestep = 0;

					//发送事件
					//反馈给界面
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x07;//参数应用命令
					ptempevent->status = 0x01;//命令状态，正常返回
					ptempevent->data[0] = pubcom.printactivenum;
					ptempevent->data[1] = cmdlist[0].para1;

					//发送一个事件给界面线程
					QCoreApplication::postEvent(selectprintobject, ptempevent);
				}


				//超时
				if (pubtimermng.gettimerval(pubcom.printactivetimer) > OUTTIME)
				{
					pubtimermng.deltimer(pubcom.printactivetimer);
					pubcom.printactivestep = 2;
				}
			}
		}


		////////////////////////////////////////////////////////////////////////////
		//上浆磁台磁力改变
		////////////////////////////////////////////////////////////////////////////
		if((pubcom.PlcGongjiangcilidaxiaogaibianFlag == true)
			 && (pubcom.Shangjiangcitaistep == 0))
		{
			//开机状态才发送
			if(pubcom.Shangjiangcitaiflag == true)
			{
				pubcom.Shangjiangcitaistep = 1;
			}
			pubcom.PlcGongjiangcilidaxiaogaibianFlag = false;
		}

		if((pubcom.PlcGongjiangcilikaiguanUpFlag == true) 
			&& (pubcom.Shangjiangcitaistep == 0))
		{
			pubcom.PlcGongjiangcilikaiguanUpFlag = false;
			pubcom.Shangjiangcitaiflag = true;
			pubcom.Shangjiangcitaistep = 1; 
		}

		if((pubcom.PlcGongjiangcilikaiguanDownFlag == true) 
			&& (pubcom.Shangjiangcitaistep == 0))
		{
			pubcom.PlcGongjiangcilikaiguanDownFlag = false;
			pubcom.Shangjiangcitaiflag = false;
			pubcom.Shangjiangcitaistep = 1; 
		}

		//发送命令到磁台控制板
		{
			//初始化
			if (pubcom.Shangjiangcitaistep == 1)
			{
				pubcom.Shangjiangcitaicount = 0;
				pubcom.Shangjiangcitaitimer = 0;
				pubcom.Shangjiangcitaistep = 2;
			}
			//发送
			else if (pubcom.Shangjiangcitaistep == 2)
			{
				if (pubcom.Shangjiangcitaicount > 1)
				{
					//返回
					pubcom.Shangjiangcitaistep = 0;
				}
				else
				{
					pubcom.Shangjiangcitaicount++;

					//发送命令
					tcmd tempcmd;
					tempcmd.cmd = MAGICRSPCMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = pubcom.Shangjiangcitaibianhao - 1;

					//磁台开启还是关闭
					if(pubcom.Shangjiangcitaiflag == true)
					{
						tempcmd.para2 = pubcom.PlcGongjiangcilidaxiao;
					}
					else if(pubcom.Shangjiangcitaiflag == false)
					{
						tempcmd.para2 = 0;
					}

					if (candevice.sendcmd(MAGICIONODEID, &tempcmd) == false)
					{
						//产生事件，提醒用户注意
						publiccaution.addevent(
							"磁台板",
							"发送给磁台板CAN命令失败","发送CAN命令失败");
					}
					else
					{
						pubcom.Shangjiangcitaistep = 3;
						pubcom.Shangjiangcitaitimer = pubtimermng.addtimer();
					}
				}
			}//接收
			else if (pubcom.Shangjiangcitaistep == 3)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd(MAGICIOBDID, MAGICRSPCMD, pubcom.Shangjiangcitaibianhao -1,
					cmdlist);

				//如果有应答
				if (count >= 1)
				{
					pubtimermng.deltimer(pubcom.Shangjiangcitaitimer);
					pubcom.Shangjiangcitaistep = 0;
				}
				//超时
				if (pubtimermng.gettimerval(pubcom.Shangjiangcitaitimer) > OUTTIME)
				{
					pubtimermng.deltimer(pubcom.Shangjiangcitaitimer);
					pubcom.Shangjiangcitaistep = 2;
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//网头调速设置参数
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.printchgspeedansflag == true)
		{
			//初始化
			if (pubcom.printchgspeedstep == 1)
			{
				pubcom.printchgspeedcount = 0;
				pubcom.printchgspeedtimer = 0;

				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					pubcom.sendprintchgspeedmap.insert(*it, false);
					pubcom.printchgspeedansmap.insert(*it, false);
				}
				pubcom.activeprintlistmutex.unlock();

				pubcom.printchgspeedstep = 2;
			}
			//发送
			else if (pubcom.printchgspeedstep == 2)
			{
				if (pubcom.printchgspeedcount > 1)
				{
					//返回
					pubcom.printchgspeedansflag = false;
					pubcom.printchgspeedstep = 0;

					//反馈给界面
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x06;//参数应用命令
					if (pubcom.printchgspeedsucceed == true)
					{
						ptempevent->status = 0x02;//命令状态
					}
					else
					{
						ptempevent->status = 0x03;//命令状态
					}

					//发送一个事件给界面线程
					QCoreApplication::postEvent(interfaceobject, ptempevent);
				}
				else
				{
					pubcom.printchgspeedcount++;

					//广播命令
					tcmd tempcmd;
					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = 0x05;

					QDomNode paranode =
						GetParaByName("prod", QString("网头调速允许"));
					bool ok;

					if (paranode.firstChildElement("value").text().toInt(&ok)
						== 1)
					{
						tempcmd.para2 = 1;
					}
					else
					{
						tempcmd.para2 = 2;
					}

					if (candevice.sendcmd(ALLPRINTID, &tempcmd) == false)
					{
						msleep(1);
						if (candevice.sendcmd(ALLPRINTID, &tempcmd) == false)
						{
							//产生事件，提醒用户注意
							publiccaution.addevent(
								"所有网头板",
								"发送给所有网头板CAN命令失败","发送CAN命令失败");
						}
						else
						{
							pubcom.printchgspeedstep = 3;
							pubcom.printchgspeedtimer = pubtimermng.addtimer();

							QList<int>::iterator it;
							pubcom.activeprintlistmutex.lockForRead();

							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								pubcom.sendprintchgspeedmap.insert(*it, true);
							}
							pubcom.activeprintlistmutex.unlock();

						}
					}
					else
					{
						pubcom.printchgspeedstep = 3;
						pubcom.printchgspeedtimer = pubtimermng.addtimer();

						QList<int>::iterator it;
						pubcom.activeprintlistmutex.lockForRead();

						for (it = pubcom.activeprintlist.begin(); it
							!= pubcom.activeprintlist.end(); ++it)
						{
							pubcom.sendprintchgspeedmap.insert(*it, true);
						}
						pubcom.activeprintlistmutex.unlock();

					}
				}
			}//接收
			else if (pubcom.printchgspeedstep == 3)
			{
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					QList<tcmd> cmdlist;
					int count = candevice.getcmd((*it), SETPRINTPARACMD, 0x05,
						cmdlist);

					//如果有应答
					if (count >= 1)
					{
						pubcom.printchgspeedsucceed = true;
						pubcom.printchgspeedansmap.insert(*it, true);
					}
				}
				pubcom.activeprintlistmutex.unlock();


				if (pubtimermng.gettimerval(pubcom.printchgspeedtimer)
					<= OUTTIME)
				{
					bool allget = true;//是否全部接收到
					QMap<int, bool>::iterator it;
					for (it = pubcom.sendprintchgspeedmap.begin(); it
						!= pubcom.sendprintchgspeedmap.end(); ++it)
					{
						if (it.value() == true
							&& pubcom.printchgspeedansmap.value(*it)
							!= true)
						{
							allget = false;
							break;
						}
					}
					if (allget == true)
					{
						pubtimermng.deltimer(pubcom.printchgspeedtimer);
						bool hasrecvfail = false;
						QMap<int, bool>::iterator it;
						for (it = pubcom.printchgspeedansmap.begin(); it
							!= pubcom.printchgspeedansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								hasrecvfail = true;
								break;
							}
						}

						if (hasrecvfail == true)
						{
							pubcom.printchgspeedstep = 2;

							//重置失败
							for (it = pubcom.printchgspeedansmap.begin(); it
								!= pubcom.printchgspeedansmap.end(); ++it)
							{
								if (it.value() == false)
								{
									pubcom.sendprintchgspeedmap.insert(
										it.key(), false);
								}
							}
						}
						else
						{
							pubcom.printchgspeedstep = 0;
							pubcom.printchgspeedansflag = false;
							pubtimermng.deltimer(pubcom.printchgspeedtimer);

							//反馈给界面
							backendevent* ptempevent = new backendevent();
							ptempevent->cmd = 0x06;//参数应用命令
							ptempevent->status = 0x01;//命令状态,全部成功

							//发送一个事件给界面线程
							QCoreApplication::postEvent(interfaceobject,
								ptempevent);
						}
					}
					else
					{

					}
				}
				else
				{
					pubtimermng.deltimer(pubcom.printchgspeedtimer);
					bool hasrecvfail = false;
					QMap<int, bool>::iterator it;
					for (it = pubcom.printchgspeedansmap.begin(); it
						!= pubcom.printchgspeedansmap.end(); ++it)
					{
						if (it.value() == false)
						{
							hasrecvfail = true;
							break;
						}
					}

					if (hasrecvfail == true)
					{
						//重试
						pubcom.printchgspeedstep = 2;

						for (it = pubcom.printchgspeedansmap.begin(); it
							!= pubcom.printchgspeedansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								pubcom.sendprintchgspeedmap.insert(it.key(),
									false);
							}
						}
					}
					else
					{
						pubcom.printchgspeedstep = 0;
						pubcom.printchgspeedansflag = false;

						//反馈给界面
						backendevent* ptempevent = new backendevent();
						ptempevent->cmd = 0x06;//参数应用命令
						ptempevent->status = 0x01;//命令状态,全部成功

						//发送一个事件给界面线程
						QCoreApplication::postEvent(interfaceobject, ptempevent);
					}
				}
			}
		}


		////////////////////////////////////////////////////////////////////////////
		//单网头调速设置参数
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.printchgspeedansflag == true)
		{
			//初始化
			if (pubcom.singleprintchgspeedstep == 1)
			{
				pubcom.singleprintchgspeedtimer = 0;
				pubcom.singleprintchgspeedstep = 2;
			}
			//发送
			else if (pubcom.singleprintchgspeedstep == 2)
			{
				//命令
				tcmd tempcmd;
				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x05;

				QDomNode paranode =
					GetParaByName("prod", QString("网头调速允许"));
				bool ok;

				if (paranode.firstChildElement("value").text().toInt(&ok)
					== 1)
				{
					tempcmd.para2 = 1;
				}
				else
				{
					tempcmd.para2 = 2;
				}

				if (candevice.sendcmd(pubcom.bdnodemap.value(pubcom.singleprintchgspeedid), &tempcmd) == false)
				{
					msleep(1);
					if (candevice.sendcmd(pubcom.bdnodemap.value(pubcom.singleprintchgspeedid), &tempcmd) == false)
					{
						;
					}
					else
					{
						pubcom.singleprintchgspeedstep = 3;
						pubcom.singleprintchgspeedtimer = pubtimermng.addtimer();
					}
				}
				else
				{
						pubcom.singleprintchgspeedstep = 3;
						pubcom.singleprintchgspeedtimer = pubtimermng.addtimer();
				}
			}//接收
			else if (pubcom.singleprintchgspeedstep == 3)
			{

				QList<tcmd> cmdlist;
				if(candevice.getcmd(pubcom.singleprintchgspeedid, SETPRINTPARACMD, 0x05,
						cmdlist))
				{
						pubcom.singleprintchgspeedstep = 0;
						pubcom.printchgspeedansflag = false;
						pubtimermng.deltimer(pubcom.singleprintchgspeedtimer);
				}

				if (pubtimermng.gettimerval(pubcom.singleprintchgspeedtimer)
					> OUTTIME)
				{
						pubcom.singleprintchgspeedstep = 0;
						pubcom.printchgspeedansflag = false;
						pubtimermng.deltimer(pubcom.singleprintchgspeedtimer);
				}
			}
		}



		////////////////////////////////////////////////////////////////////////////
		//摩擦系数测试橡毯开始转动
		////////////////////////////////////////////////////////////////////////////
		//初始化
		if (pubcom.measurepulsestartstep == 1)
		{
			pubcom.measurepulsestartcount = 0;
			pubcom.measurepulsestarttimer = 0;
			pubcom.measurepulsestartstep = 2;
		}
		//设置开始
		else if (pubcom.measurepulsestartstep == 2)
		{
			WritePcMotorOnBit(&regs,true);

			pubcom.measurepulsestartstep = 3;
			pubcom.measurepulsestarttimer = pubtimermng.addtimer();
		}//延时一段时间
		else if (pubcom.measurepulsestartstep == 3)
		{
			if (pubtimermng.gettimerval(pubcom.measurepulsestarttimer)
				<= MODBUSBITHOLDTIME)
			{
				;
			}
			else
			{
				pubcom.measurepulsestartstep = 0;
				pubtimermng.deltimer(pubcom.measurepulsestarttimer);

				WritePcMotorOnBit(&regs,false);

				//反馈给界面
				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x0a;//橡毯
				ptempevent->status = 0x00;//启动

				//发送一个事件给界面线程
				QCoreApplication::postEvent(measurepulseobject, ptempevent);
			}
		}


		////////////////////////////////////////////////////////////////////////////
		//摩擦系数测试橡毯停止转动
		////////////////////////////////////////////////////////////////////////////
		//初始化
		if (pubcom.measurepulsestopstep == 1)
		{
			pubcom.measurepulsestopcount = 0;
			pubcom.measurepulsestoptimer = 0;
			pubcom.measurepulsestopstep = 2;
		}
		//设置开始
		else if (pubcom.measurepulsestopstep == 2)
		{
			WritePcMotorOffBit(&regs,true);

			pubcom.measurepulsestopstep = 3;
			pubcom.measurepulsestoptimer = pubtimermng.addtimer();
		}//延时一段时间
		else if (pubcom.measurepulsestopstep == 3)
		{
			if (pubtimermng.gettimerval(pubcom.measurepulsestoptimer)
				<= MODBUSBITHOLDTIME)
			{
				;
			}
			else
			{
				pubcom.measurepulsestopstep = 0;
				pubtimermng.deltimer(pubcom.measurepulsestoptimer);

				WritePcMotorOffBit(&regs,false);

				//反馈给界面
				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x0a;//橡毯
				ptempevent->status = 0x01;//停止

				//发送一个事件给界面线程
				QCoreApplication::postEvent(measurepulseobject, ptempevent);
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//参数应用
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.isapply == true)
		{
			//遍历修改的，发送CAN命令
			if (pubcom.applystep == 1)
			{
				pubcom.applycount = 0;

				//清除命令
				pubcom.applyparacmdpair.clear();


				//工艺参数
				QMap<tablekey, tablenode>::iterator i;
				for (i = pDlgset->tablemapprod.begin(); i
					!= pDlgset->tablemapprod.end(); ++i)
				{
					bool ok;

					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{

						if (i.value().id.toElement().text() == QString(
							"找零速度"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetHMSpeed(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"修改找零速度","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"高速找零速度"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetHMSpeedHigh(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"高速找零速度","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"对花速度"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetJogSpeed(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"对花速度","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"高速对花速度"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetJogSpeedHigh(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"高速对花速度","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"低速到高速对花时间"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							pubcom.LowToHighSpeedtime = i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"高速对花速度","修改了工艺参数",1);
						}						
						else if (i.value().id.toElement().text() == QString("抬网延时"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x03;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x03;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);

							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"抬网延时","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"下网延时"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();
							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x04;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x04;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);


							}
							pubcom.activeprintlistmutex.unlock();


							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"下网延时","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"计数器选择"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"计数器选择","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"横向对花速度"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{

								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x01;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x01;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);

							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"横向对花速度","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"斜向对花速度"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}



							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x02;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x02;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);

							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"斜向对花速度","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"网头调速允许"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x05;
								if (i.value().node.toElement().text().toInt(&ok)
									== 0)
								{
									tempcmd.para2 = 2;
								}
								else
								{
									tempcmd.para2 = 1;
								}

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x05;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);

							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"网头调速允许","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"横/斜向电机强制开"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{

								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x07;
								if (i.value().node.toElement().text().toInt(&ok)
									== 0)
								{
									tempcmd.para2 = 1;
								}
								else
								{
									tempcmd.para2 = 2;
								}

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x07;
								tempcmd.para2 = 0x00;

								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);

							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"横/斜向电机强制开","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"抬网倒转延时"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x08;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x08;
								tempcmd.para2 = 0x00;

								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);
							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"抬网倒转延时","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"浆泵开启时间"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;
							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{

								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x06;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x06;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);
							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"浆泵开启时间","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"设定印制米数停车"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							pubcom.NeedStopAtWorkfinished = i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"设定印制米数停车","修改了工艺参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"磁台初始磁力"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;
							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x09;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x09;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);
							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"工艺参数",
								"磁台初始磁力","修改了工艺参数",1);
						}
					}
				}
				//机器参数1
				//QMap<tablekey,tablenode>::iterator i;
				for (i = pDlgset->tablemapmachsub1.begin(); i
					!= pDlgset->tablemapmachsub1.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{

						if (i.value().id.toElement().text() == QString(
							"抬网自转速度及旋向调整"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetCCWJogSpeed(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"抬网自转速度及旋向调整","修改了机器参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"抬/下网气缸延时"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x02;
								tempcmd.para2
									= (unsigned char) (i.value().node.toElement().text().toFloat(
									&ok) * 10);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x02;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);

							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"抬/下网气缸延时","修改了机器参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"张/松网气缸延时"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x03;
								tempcmd.para2
									= (unsigned char) (i.value().node.toElement().text().toFloat(
									&ok) * 10);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x03;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);

							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"张/松网气缸延时","修改了机器参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"锁/松料杆气缸延时"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x04;
								tempcmd.para2
									= (unsigned char) (i.value().node.toElement().text().toFloat(
									&ok) * 10);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x04;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);
							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"锁/松料杆气缸延时","修改了机器参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"料杆升降气缸延时"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}



							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 0x05;
								tempcmd.para2
									= (unsigned char) (i.value().node.toElement().text().toFloat(
									&ok) * 10);

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x05;
								tempcmd.para2 = 0x00;


								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);
							}
							pubcom.activeprintlistmutex.unlock();

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"料杆升降气缸延时","修改了机器参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"网头调速允许按钮是否工作"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							if (i.value().node.toElement().text().toInt(&ok)
								== 0)
							{
								pubcom.enuichgspdbt = false;
							}
							else
							{
								pubcom.enuichgspdbt = true;
							}

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"网头调速允许按钮是否工作","修改了机器参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"伺服电机升速"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"伺服电机升速","修改了机器参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"主编码器选正负"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetPulseDirect(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"主编码器选正负","修改了机器参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"全机最高速限制"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetMaxSpeed(
								i.value().node.toElement().text().toInt(&ok)
								* 1000);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"全机最高速限制","修改了机器参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"上浆磁台对应磁台编号"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							pubcom.Shangjiangcitaibianhao = i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"机器参数",
								"上浆磁台对应磁台编号","修改了机器参数",1);
						}
					}
				}
				//机器参数2
				for (i = pDlgset->tablemapmachsub2.begin(); i
					!= pDlgset->tablemapmachsub2.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						for (int jj = 1; jj <= 16; ++jj)
						{
							QString ParaName = QString("网头") + QString::number(jj) + QString("纵向位置偏差");
							if (i.value().id.toElement().text() == ParaName)
							{
								//设置修改完成
								if (i.value().flag.toElement().text().toInt(&ok)
									== 1)
								{
									i.value().flag.firstChild().setNodeValue(
										QString::number(0));
								}

								ksmc->SetMotorOffset(jj,i.value().node.toElement().text().toDouble(
									&ok));

								//产生事件，提醒用户注意
								publiccaution.addevent(
									"机器参数",
									ParaName,"修改了机器参数",1);
							}
						}
					}
				}

				//机器参数3
				//QMap<tablekey,tablenode>::iterator i;
				for (i = pDlgset->tablemapmachsub3.begin(); i
					!= pDlgset->tablemapmachsub3.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						for (int jj = 1; jj <= 16; ++jj)
						{
							QString ParaName = QString("网头") + QString::number(jj) + QString("对应浆泵编号");

							if (i.value().id.toElement().text() == ParaName
								&& (pubcom.PrintNum >= jj)
								&& (pubcom.activeprintlist.contains(jj-1)))
							{
								//设置修改完成
								if (i.value().flag.toElement().text().toInt(&ok)
									== 1)
								{
									i.value().flag.firstChild().setNodeValue(
										QString::number(0));
								}

								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETBUMPNOCMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = jj-1;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok) - 1;

								t.sendcmd.canid = pubcom.bdnodemap.value(jj-1);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETBUMPNOCMD;
								tempcmd.id = (jj-1);
								tempcmd.para1 = jj-1;
								tempcmd.para2 = 0x00;

								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[jj-1].append(t);

								//产生事件，提醒用户注意
								publiccaution.addevent(
									"机器参数",
									ParaName,"修改了机器参数",1);
							}
						}
					}
				}

				//机器参数4
				//QMap<tablekey,tablenode>::iterator i;
				for (i = pDlgset->tablemapmachsub4.begin(); i
					!= pDlgset->tablemapmachsub4.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						for (int jj = 1; jj <= 16; ++jj)
						{
							QString ParaName = QString("浆泵") + QString::number(jj) + QString("输出点");

							if (i.value().id.toElement().text() == ParaName )
							{
								//设置修改完成
								if (i.value().flag.toElement().text().toInt(&ok)
									== 1)
								{
									i.value().flag.firstChild().setNodeValue(
										QString::number(0));

								}

								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETBUMPIONOCMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = jj-1;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok) - 1;

								t.sendcmd.canid = pubcom.bdnodemap.value(BUMPIOBDID);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETBUMPIONOCMD;
								tempcmd.id = (BUMPIOBDID);
								tempcmd.para1 = jj-1;
								tempcmd.para2 = 0x00;

								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[BUMPIOBDID].append(t);

								//产生事件，提醒用户注意
								publiccaution.addevent(
									"机器参数",
									ParaName,"修改了机器参数",1);
							}
						}
					}
				}

				//机器参数5
				//QMap<tablekey,tablenode>::iterator i;
				for (i = pDlgset->tablemapmachsub5.begin(); i
					!= pDlgset->tablemapmachsub5.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						for (int jj = 1; jj <= 16; ++jj)
						{
							QString ParaName = QString("磁台") + QString::number(jj) + QString("输出点");

							if (i.value().id.toElement().text() == ParaName )
							{
								//设置修改完成
								if (i.value().flag.toElement().text().toInt(&ok)
									== 1)
								{
									i.value().flag.firstChild().setNodeValue(
										QString::number(0));

								}

								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETMAGICIONOCMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = jj-1;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok) - 1;

								t.sendcmd.canid = pubcom.bdnodemap.value(MAGICIOBDID);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETMAGICIONOCMD;
								tempcmd.id = (MAGICIOBDID);
								tempcmd.para1 = jj-1;
								tempcmd.para2 = 0x00;

								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[MAGICIOBDID].append(t);

								//产生事件，提醒用户注意
								publiccaution.addevent(
									"机器参数",
									ParaName,"修改了机器参数",1);
							}
						}
					}
				}

				//机器参数6
				//QMap<tablekey,tablenode>::iterator i;
				for (i = pDlgset->tablemapmachsub6.begin(); i
					!= pDlgset->tablemapmachsub6.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{

						for (int jj = 1; jj <= 16; ++jj)
						{
							QString ParaName = QString("磁台") + QString::number(jj) + QString("模拟量输出点");

							if (i.value().id.toElement().text() == ParaName )
							{
								//设置修改完成
								if (i.value().flag.toElement().text().toInt(&ok)
									== 1)
								{
									i.value().flag.firstChild().setNodeValue(
										QString::number(0));

								}

								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETMAGICANANOCMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = jj-1;
								tempcmd.para2
									= i.value().node.toElement().text().toInt(
									&ok) - 1;

								t.sendcmd.canid = pubcom.bdnodemap.value(MAGICIOBDID);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETMAGICANANOCMD;
								tempcmd.id = (MAGICIOBDID);
								tempcmd.para1 = jj-1;
								tempcmd.para2 = 0x00;

								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[MAGICIOBDID].append(t);

								//产生事件，提醒用户注意
								publiccaution.addevent(
									"机器参数",
									ParaName,"修改了机器参数",1);

							}
						}
					}
				}

				//调试参数，全机套色数
				for (i = pDlgset->tablemaptune.begin(); i
					!= pDlgset->tablemaptune.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{

						if (i.value().id.toElement().text() == QString("全机套色数"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));

							}

							int oldPrintNum = pubcom.PrintNum;

							pubcom.PrintNum
								= i.value().node.toElement().text().toInt(
								&ok);

							pubcom.bdlist.clear();

							pubcom.bdlist.append(BUMPIOBDID);
							pubcom.bdlist.append(MAGICIOBDID);

							//线程保护
							pubcom.activeprintlistmutex.lockForWrite();
							pubcom.activeprintlist.clear();

							//根据网头数量设置网头板子
							for (int ii = 0 + PRINT1BDID; ii < pubcom.PrintNum
								+ PRINT1BDID; ii++)
							{
								pubcom.bdlist.append(ii);

								if (pubcom.presetprintstatus[ii] != 1)
								{
									//设置当前激活的网头板子
									pubcom.activeprintlist.append(ii);
								}
							}
							pubcom.activeprintlistmutex.unlock();

							//删除网头的故障
							for (int ii = PRINT1BDID;  ii <= PRINT16BDID ; ++ ii)
							{
								if (ii >= pubcom.PrintNum )
								{
									//删除故障
									pubcom.delcauflag = true;
									publiccaution.delcaution(
										pubcom.bdcautionmap.value(ii));
									pubcom.delcauflag = false;
								}
								else
								{
									//删除非激活网头故障
									if (pubcom.presetprintstatus[ii] == 1)
									{
										pubcom.delcauflag = true;
										publiccaution.delcaution(
											pubcom.bdcautionmap.value(ii));
										pubcom.delcauflag = false;

									}
								}
							}

							//对于马达的操作要慎重要确保是非工作状态才可以设置
							ksmc->SetOffMotor(-1);
							ksmc->DelMotor(-1);

							//根据网头数量增加马达
							for (int ii = 0; ii < pubcom.PrintNum; ii++)
							{
								if (pubcom.activeprintlist.contains(ii) == true)
								{
									ksmc->AddMotor(ii + 1);
								}
							}


							//对于新增的马达要重新设置驱动器参数
							int paraVal = 0;
							QDomNode para;

							//驱动器参数设置
							para = GetParaByName("driver", "第一位置增益");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,16,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第一速度增益");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,17,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第一速度积分时间常数");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,18,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第一速度检测过滤");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,19,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第一扭矩过滤时间常数");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,20,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第二位置增益");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,24,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第二速度增益");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,25,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第二速度积分时间常数");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,26,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第二速度检测过滤");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,27,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "第二扭矩过滤时间常数");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,28,paraVal);

									}
								}
							}
							//驱动器参数设置
							para = GetParaByName("driver", "第二增益设置");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,48,paraVal);

									}
								}
							}
							//驱动器参数设置
							para = GetParaByName("driver", "增益切换模式");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,49,paraVal);

									}
								}
							}
							//驱动器参数设置
							para = GetParaByName("driver", "增益切换延迟时间");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,50,paraVal);

									}
								}
							}
							//驱动器参数设置
							para = GetParaByName("driver", "增益切换水平");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,51,paraVal);

									}
								}
							}
							//驱动器参数设置
							para = GetParaByName("driver", "增益切换滞后");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,52,paraVal);

									}
								}
							}

							//驱动器参数设置
							para = GetParaByName("driver", "位置环增益切换时间");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//参数为0则表示可以通过网络设置
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,53,paraVal);

									}
								}
							}

							//初始化马达
							ksmc->InitMotor(-1);

							//激活马达
							ksmc->SetActiveMotor(-1);

							//网头状态重新发送
							if (pubcom.statuschgstep == 0 
								&& pubcom.plcreqstartstep == 0)
							{
								pubcom.statuschgstep = 1;
							}

							pubcom.activeprintlistmutex.lockForRead();
							QList<int>::iterator it;

							//初始化
							for (it = pubcom.activeprintlist.begin(); it
								!= pubcom.activeprintlist.end(); ++it)
							{
								tbdonlineparacmdpair t;
								tcmd tempcmd;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = PCCANBDID;
								tempcmd.para1 = 1;
								tempcmd.para2 = pubcom.PrintNum;

								t.sendcmd.canid = pubcom.bdnodemap.value(*it);
								t.sendcmd.cmd = tempcmd; 
								t.sendcmd.sendsucceed = false;

								tempcmd.cmd = SETPRINTMACHPARACMD;
								tempcmd.id = (*it);
								tempcmd.para1 = 0x01;
								tempcmd.para2 = 0x00;

								t.recvcmd.cmd = tempcmd;
								t.recvcmd.recvsucceed = false;
								pubcom.applyparacmdpair[*it].append(t);
							}
							pubcom.activeprintlistmutex.unlock();


							//删除和板子有关系的内存变量

							//刷新磁台可见数量
							backendevent* ptempevent = new backendevent();
							ptempevent->cmd = 0x07;//刷新磁台可见数量
							//发送一个事件给界面线程
							QCoreApplication::postEvent(interfaceobject,
								ptempevent);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"全机套色数","修改了调试参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"伺服电机正反转选择"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}
							pubcom.ServoDir = i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"伺服电机正反转选择","修改了调试参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"双传动用参数"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"双传动用参数","修改了调试参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"摩擦系数"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetPulserPerMeter(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"摩擦系数","修改了调试参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"主编码器信号响应时间"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"主编码器信号响应时间","修改了调试参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"机头间隔距离"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetOffset(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"机头间隔距离","修改了调试参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"网头齿数"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetToothNum2(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"网头齿数","修改了调试参数",1);
						}

						else if (i.value().id.toElement().text() == QString(
							"网头周长"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetGirth(
								i.value().node.toElement().text().toFloat(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"网头周长","修改了调试参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"编码器分辨率"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetPulsePerCircle(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"编码器分辨率","修改了调试参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"跟随滞后量"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}
							pubcom.Follow = i.value().node.toElement().text().toFloat(&ok) * 0.01;

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"调试参数",
								"跟随滞后量","修改了调试参数",1);
						}
					}
				}

				//开发参数
				for (i = pDlgset->tablemapdev.begin(); i
					!= pDlgset->tablemapdev.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						if (i.value().id.toElement().text() == QString(
							"伺服电机变速箱速比"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetServGearBoxScale(
								i.value().node.toElement().text().toDouble(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"伺服电机变速箱速比","修改了开发参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"伺服电机变速箱输出轴齿数"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetToothNum1(
								i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"伺服电机变速箱输出轴齿数","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"can口选择"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//重新应用参数
							candevice.close();
							if (candevice.open() == true)
							{
								publiccaution.delcaution(PCCAUTIONLOCATION,PCNOCANCATION);
							}


							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"can口选择","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"can速率"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//重新应用参数
							candevice.close();
							if (candevice.open() == true)
							{
								publiccaution.delcaution(PCCAUTIONLOCATION,PCNOCANCATION);
							}

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"can速率","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"串口选择"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialName(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//重新应用参数
							modbusRTU_slave_SerialClose(&modbusRTU);
							//确认串口可以重新打开，所以要先休眠，等待event锁定结束
							msleep(100);
							if (modbusRTU_slave_SerialOpen(&modbusRTU) == true)
							{
								publiccaution.delcaution(SERIAL,SERIALNOEXIST);
							}
							else
							{
								publiccaution.addcaution(SERIAL,SERIALNOEXIST);
							}
#endif

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"串口选择","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"串口通讯速率"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}
							modbusRTU_slave_SetSerialBaudRate(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//重新应用参数
							modbusRTU_slave_SerialClose(&modbusRTU);
							//确认串口可以重新打开，所以要先休眠，等待event锁定结束
							msleep(100);
							if (modbusRTU_slave_SerialOpen(&modbusRTU) == true)
							{
								publiccaution.delcaution(SERIAL,SERIALNOEXIST);
							}
							else
							{
								publiccaution.addcaution(SERIAL,SERIALNOEXIST);
							}
#endif
							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"串口通讯速率","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"串口校验方式"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialParity(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//重新应用参数
							modbusRTU_slave_SerialClose(&modbusRTU);
							//确认串口可以重新打开，所以要先休眠，等待event锁定结束
							msleep(100);
							if (modbusRTU_slave_SerialOpen(&modbusRTU) == true)
							{
								publiccaution.delcaution(SERIAL,SERIALNOEXIST);
							}
							else
							{
								publiccaution.addcaution(SERIAL,SERIALNOEXIST);
							}
#endif

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"串口校验方式","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"串口字节位数"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialByteSize(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//重新应用参数
							modbusRTU_slave_SerialClose(&modbusRTU);
							//确认串口可以重新打开，所以要先休眠，等待event锁定结束
							msleep(100);
							if (modbusRTU_slave_SerialOpen(&modbusRTU) == true)
							{
								publiccaution.delcaution(SERIAL,SERIALNOEXIST);
							}
							else
							{
								publiccaution.addcaution(SERIAL,SERIALNOEXIST);
							}
#endif
							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"串口字节位数","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"串口停止位"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialStopBits(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//重新应用参数
							modbusRTU_slave_SerialClose(&modbusRTU);
							//确认串口可以重新打开，所以要先休眠，等待event锁定结束
							msleep(100);
							if (modbusRTU_slave_SerialOpen(&modbusRTU) == true)
							{
								publiccaution.delcaution(SERIAL,SERIALNOEXIST);
							}
							else
							{
								publiccaution.addcaution(SERIAL,SERIALNOEXIST);
							}
#endif

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"串口停止位","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"RTS流控制"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialfRtsControl(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//重新应用参数
							modbusRTU_slave_SerialClose(&modbusRTU);
							//确认串口可以重新打开，所以要先休眠，等待event锁定结束
							msleep(100);
							if (modbusRTU_slave_SerialOpen(&modbusRTU) == true)
							{
								publiccaution.delcaution(SERIAL,SERIALNOEXIST);
							}
							else
							{
								publiccaution.addcaution(SERIAL,SERIALNOEXIST);
							}
#endif

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"RTS流控制","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"手脉输入信号滤波参数"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//重新应用参数
							ksmc->SetFollowFilter(i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"手脉输入信号滤波参数","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"CAN失败次数记为离线设置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//重新应用参数
							pubcom.canfailedcnt = i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"CAN失败次数记为离线设置","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"对零传感器信号宽度"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//重新应用参数
							pubcom.ZeroDetectWidth = i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"对零传感器信号宽度","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"对零高速段宽度"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//重新应用参数
							pubcom.ZeroHighSpeedWidth = i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"对零高速段宽度","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"对零低速段宽度"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//重新应用参数
							pubcom.ZeroLowSpeedWidth = i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"对零低速段宽度","修改了开发参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"是否记录调试信息"))
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//重新应用参数
							pubcom.bldbginforecflag = (bool)i.value().node.toElement().text().toInt(&ok);

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"开发参数",
								"是否记录调试信息","修改了开发参数",1);

						}
					}
				}

				//驱动器参数
				for (i = pDlgset->tablemapdriver.begin(); i
					!= pDlgset->tablemapdriver.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						int parIndex = i.value().id.toElement().text().toInt(&ok);
						int networkcanset = -1;

						for ( int index = 0; index < pubcom.PrintNum; index++ )
						{
							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->GetDriverPar(index+1,10,networkcanset);
							//参数为0则表示可以通过网络设置
							if (networkcanset == 0)
							{
								ksmc->SetDriverPar(index+1,parIndex,i.value().node.toElement().text().toInt(&ok));

							}
							//产生事件，提醒用户注意
							publiccaution.addevent(
								"驱动器参数",
								i.value().id.toElement().text(),"修改了驱动器参数",1);

						}
					}
				}

				//MODBUS参数
				for (i = pDlgset->tablemapmodbus.begin(); i
					!= pDlgset->tablemapmodbus.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						if (i.value().id.toElement().text() == QString(
							"ID编号"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}
							set_modbusRTU_slave_ModbusID(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"MODBUS参数",
								"ID编号","修改了MODBUS参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"1.5T时间"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							set_modbusRTU_slave_t15(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"MODBUS参数",
								"1.5T时间","修改了MODBUS参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"3.5T时间"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							set_modbusRTU_slave_t35(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"MODBUS参数",
								"3.5T时间","修改了MODBUS参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"应答等待时间"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							set_modbusRTU_slave_answaittime(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"MODBUS参数",
								"应答等待时间","修改了MODBUS参数",1);

						}
					}
				}

				//PLCIO参数
				for (i = pDlgset->tablemapplcio.begin(); i
					!= pDlgset->tablemapplcio.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						if (i.value().id.toElement().text() == QString(
							"工控机开机允许位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcEnableStartBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机开机允许位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机主机加速位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							SetPcMotorAccBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机主机加速位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机主机减速位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcMotorDecBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机主机减速位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机报错位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							SetPcErrBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机报错位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机报警位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							SetPcCauBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机报警位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机开橡毯启动位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcMotorOnBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机开橡毯启动位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机橡毯停止位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcMotorOffBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机橡毯停止位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机工作量满位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcWorkFinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机工作量满位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机设置心跳位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcSetHeartBeatBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机设置心跳位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机数据刷新请求位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcFlushDataReqBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"工控机数据刷新请求位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"PLC主机开机请求位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcStartReqBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC主机开机请求位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"PLC主机停止位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcStopBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC主机停止位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"PLC报错位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}



							SetPlcErrBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC报错位位置","修改了PLCIO参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"PLC报警位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcCauBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC报警位位置","修改了PLCIO参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC进布运行位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcJinbuyunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC进布运行位位置","修改了PLCIO参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC印花导带运行位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcYinhuadaodaiyunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC印花导带运行位位置","修改了PLCIO参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC烘房网带1运行位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcHongfangwangdai1yunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC烘房网带1运行位位置","修改了PLCIO参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC烘房网带2运行位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcHongfangwangdai2yunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC烘房网带2运行位位置","修改了PLCIO参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC落布运行位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcLuobuyunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC落布运行位位置","修改了PLCIO参数",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC供浆磁力开关位位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcGongjiangcilikaiguanBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCIO参数",
								"PLC供浆磁力开关位位置","修改了PLCIO参数",1);
						}

					}
				}



				//PLCPARA参数
				for (i = pDlgset->tablemapplcpara.begin(); i
					!= pDlgset->tablemapplcpara.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						if (i.value().id.toElement().text() == QString(
							"工控机输出IO字位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcOutIOWordIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCPARA参数",
								"工控机输出IO字位置","修改了PLCPARA参数",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"工控机计米双字位置"))
						{

							//设置修改完成
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							SetPcWorkRecDWordIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//产生事件，提醒用户注意
							publiccaution.addevent(
								"PLCPARA参数",
								"工控机计米双字位置","修改了PLCPARA参数",1);

						}
					}
				}

				if (pubcom.refreshapply == true)
				{
					pubcom.refreshapply = false;
				}

				//初始化后，判断是否有需要CAN处理的命令
				if (pubcom.applyparacmdpair.isEmpty() == true)
				{
					pubcom.isapply = false;
					pubcom.applystep = 0;

					//反馈给界面
					backendevent* ptempevent = new backendevent();
					if (pubcom.applyansplace == 1)
					{
						ptempevent->cmd = 0x04;//参数应用命令
					}
					else if (pubcom.applyansplace == 2)
					{
						ptempevent->cmd = 0x05;//参数应用命令
					}

					ptempevent->data[0] = 0x00;

					ptempevent->status = 0x01;//命令状态，标识部分命令没有返回

					//发送一个事件给界面线程
					if (pubcom.applyansplace == 1)
					{
						QCoreApplication::postEvent(setobject, ptempevent);
					}
					else if (pubcom.applyansplace == 2)
					{
						QCoreApplication::postEvent(interfaceobject, ptempevent);
					}
				}
				else
				{
					pubcom.applystep = 2;
				}
			}
			//发送CAN命令
			else if (pubcom.applystep == 2)
			{
				//重试次数 > 1
				if (pubcom.applycount > 1)
				{
					pubcom.isapply = false;
					pubcom.applystep = 0;

					//反馈给界面
					backendevent* ptempevent = new backendevent();
					if (pubcom.applyansplace == 1)
					{
						ptempevent->cmd = 0x04;//参数应用命令
					}
					else if (pubcom.applyansplace == 2)
					{
						ptempevent->cmd = 0x05;//参数应用命令
					}

					ptempevent->data[0] = 0x00;

					if (pubcom.hasparasucceed == true)
					{
						ptempevent->status = 0x02;//命令状态，标识部分命令没有返回
					}
					else
					{
						ptempevent->status = 0x03;//命令状态,没有参数设置成功
					}

					//发送一个事件给界面线程
					if (pubcom.applyansplace == 1)
					{
						QCoreApplication::postEvent(setobject, ptempevent);
					}
					else if (pubcom.applyansplace == 2)
					{
						QCoreApplication::postEvent(interfaceobject, ptempevent);
					}
				}
				else
				{
					QMap<int,QList<tbdonlineparacmdpair> >::iterator it;

					pubcom.applycount++;
					bool sendonesucceed = false;

					for (it = pubcom.applyparacmdpair.begin(); it
						!= pubcom.applyparacmdpair.end(); ++it)
					{
						QList<tbdonlineparacmdpair>::iterator itt;
						for (itt = it.value().begin();itt != it.value().end(); ++ itt)
						{
							//接收失败就发送
							if ((*itt).recvcmd.recvsucceed == false)
							{
								if (candevice.sendcmd((*itt).sendcmd.canid,
									&((*itt).sendcmd.cmd)) == false)
								{
									msleep(1);
									if (candevice.sendcmd((*itt).sendcmd.canid,
										&((*itt).sendcmd.cmd)) == false)
									{
										(*itt).sendcmd.sendsucceed = false;

										//产生事件，提醒用户注意
										publiccaution.addevent(
											pubcom.bdcautionmap.value(it.key()),
											SENDCMDFAILEVENT,"发送CAN命令失败");
									}
									else
									{
										sendonesucceed = true;
										(*itt).sendcmd.sendsucceed = true;
									}
								}
								else
								{
									sendonesucceed = true;
									(*itt).sendcmd.sendsucceed = true;
								}
							}

							//每个命令见间隔10ms
							msleep(10);
						}
					}

					//只要一个发送成功了，就要接收应答，进入接收应答阶段
					if (sendonesucceed == true)
					{
						pubcom.applystep = 3;
						pubcom.applytimer = pubtimermng.addtimer();
					}
					else
					{
						//step 依旧是 2，就会继续重试了
						//do nothing;
					}
				}

			}//等待CAN命令返回
			else if (pubcom.applystep == 3)
			{

				//收命令
				QMap<int,QList<tbdonlineparacmdpair> >::iterator it;

				for (it = pubcom.applyparacmdpair.begin(); it
					!= pubcom.applyparacmdpair.end(); ++it)
				{
					QList<tbdonlineparacmdpair>::iterator itt;
					for (itt = it.value().begin();itt != it.value().end(); ++ itt)
					{
						//成功发送并且没有收到才接收
						if ( (*itt).sendcmd.sendsucceed == true 
							&& (*itt).recvcmd.recvsucceed == false )
						{
							//浆泵编号命令，要特别处理
							if ((*itt).sendcmd.cmd.cmd == SETBUMPNOCMD)
							{
								QList<tcmd> cmdlist;
								int count = candevice.getcmd((*itt).recvcmd.cmd.id,
									(*itt).recvcmd.cmd.cmd, cmdlist);

								//接收到了
								if (count >= 1)
								{
									pubcom.hasparasucceed = true;
									(*itt).recvcmd.recvsucceed = true;
								}
							} 
							else
							{
								QList<tcmd> cmdlist;
								int count = candevice.getcmd((*itt).recvcmd.cmd.id,
									(*itt).recvcmd.cmd.cmd, (*itt).recvcmd.cmd.para1, cmdlist);

								//接收到了
								if (count >= 1)
								{
									pubcom.hasparasucceed = true;
									(*itt).recvcmd.recvsucceed = true;
								}
							}
						}
					}
				}


				if (pubtimermng.gettimerval(pubcom.applytimer) < OUTTIME)
				{
					bool isallget = true;//是否全部接收到，特指本次发送成功的，对于没有发送成功的不判断

					QMap<int,QList<tbdonlineparacmdpair> >::iterator it;

					for (it = pubcom.applyparacmdpair.begin(); it
						!= pubcom.applyparacmdpair.end(); ++it)
					{
						QList<tbdonlineparacmdpair>::iterator itt;
						for (itt = it.value().begin();itt != it.value().end(); ++ itt)
						{
							//成功发送才接收
							if ((*itt).sendcmd.sendsucceed
								== true)
							{
								if ((*itt).recvcmd.recvsucceed == false)
								{
									isallget = false;
									break;
								}
							}
						}

						if (isallget == false)
						{
							break;
						}
					}


					if (isallget == true)
					{
						pubtimermng.deltimer(pubcom.applytimer);

						//判断全部的命令是否全部应答了
						bool hasrecvfail = false;

						QMap<int,QList<tbdonlineparacmdpair> >::iterator it;

						for (it = pubcom.applyparacmdpair.begin(); it
							!= pubcom.applyparacmdpair.end(); ++it)
						{
							QList<tbdonlineparacmdpair>::iterator itt;
							for (itt = it.value().begin();itt != it.value().end(); ++ itt)
							{
								if ((*itt).recvcmd.recvsucceed == false)
								{
									hasrecvfail = true;
									break;
								}
							}
							if (hasrecvfail == true)
							{
								break;
							}
						}


						if (hasrecvfail == true)
						{
							//继续尝试发送
							pubcom.applystep = 2;
						}
						else
						{
							pubcom.isapply = false;
							pubcom.applystep = 0;

							//反馈给界面
							backendevent* ptempevent = new backendevent();
							if (pubcom.applyansplace == 1)
							{
								ptempevent->cmd = 0x04;//参数应用命令
							}
							else if (pubcom.applyansplace == 2)
							{
								ptempevent->cmd = 0x05;//参数应用命令
							}

							ptempevent->data[0] = 0x00;

							ptempevent->status = 0x01;//命令状态，全部完成

							//发送一个事件给界面线程
							if (pubcom.applyansplace == 1)
							{
								QCoreApplication::postEvent(setobject,
									ptempevent);
							}
							else if (pubcom.applyansplace == 2)
							{
								QCoreApplication::postEvent(interfaceobject,
									ptempevent);
							}
						}
					}
					else
					{
						//do nothing
					}
				}
				else
				{
					pubtimermng.deltimer(pubcom.applytimer);
					bool hasrecvfail = false;

					QMap<int,QList<tbdonlineparacmdpair> >::iterator it;

					for (it = pubcom.applyparacmdpair.begin(); it
						!= pubcom.applyparacmdpair.end(); ++it)
					{
						QList<tbdonlineparacmdpair>::iterator itt;
						for (itt = it.value().begin();itt != it.value().end(); ++ itt)
						{
							if ((*itt).recvcmd.recvsucceed == false)
							{
								hasrecvfail = true;
								break;
							}
						}
						if (hasrecvfail == true)
						{
							break;
						}
					}

					if (hasrecvfail == true)
					{
						//继续尝试发送
						pubcom.applystep = 2;
					}
					else
					{
						pubcom.isapply = false;
						pubcom.applystep = 0;

						//反馈给界面
						backendevent* ptempevent = new backendevent();
						if (pubcom.applyansplace == 1)
						{
							ptempevent->cmd = 0x04;//参数应用命令
						}
						else if (pubcom.applyansplace == 2)
						{
							ptempevent->cmd = 0x05;//参数应用命令
						}

						ptempevent->data[0] = 0x00;

						ptempevent->status = 0x01;//命令状态，全部成功

						//发送一个事件给界面线程
						if (pubcom.applyansplace == 1)
						{
							QCoreApplication::postEvent(setobject, ptempevent);
						}
						else if (pubcom.applyansplace == 2)
						{
							QCoreApplication::postEvent(interfaceobject,
								ptempevent);
						}
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////
		//板子上线处理流程
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.applystep == 0)
		{
			QMap<int,int>::iterator it;
			for (it = pubcom.bdonlinestep.begin();it != pubcom.bdonlinestep.end(); ++it)
			{
				//初始化
				if (it.value() == 1)
				{
					//对于网头板，如果不是激活的板子，就不要测试版本了
					if (it.key() >= PRINT1BDID 
						&& it.key() <= PRINT16BDID)
					{
						//非休眠态
						if (pubcom.presetprintstatus[it.key()] != 1)
						{
							pubcom.bdonlinecount.insert(it.key(),0);
							pubcom.bdonlinetimer.insert(it.key(),0);
							pubcom.bdonlinestep.insert(it.key(),2);
						}
						else
						{
							pubcom.bdonlinecount.insert(it.key(),0);
							pubcom.bdonlinetimer.insert(it.key(),0);

							//直接应用参数，不做版本测试
							pubcom.bdonlinestep.insert(it.key(),4);
						}
					}
					else
					{
						pubcom.bdonlinecount.insert(it.key(),0);
						pubcom.bdonlinetimer.insert(it.key(),0);
						pubcom.bdonlinestep.insert(it.key(),2);
					}

				}//版本测试
				else if (it.value() == 2)
				{
					if (pubcom.bdonlinecount.value(it.key()) > 2)
					{
						//继续设置参数
						pubcom.bdonlinestep.insert(it.key(),4);
					}
					else
					{
						//计数器+1
						pubcom.bdonlinecount.insert(it.key(),pubcom.bdonlinecount.value(it.key()) + 1);

						//发命令
						tcmd tempcmd;
						tempcmd.cmd = VERTESTCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0;
						tempcmd.para2 = 0;

						//发送命令
						if (candevice.sendcmd(pubcom.bdnodemap.value(it.key()),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(it.key()),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(it.key()),
									SENDCMDFAILEVENT,"发送CAN命令失败");

							}
							else
							{
								pubcom.bdonlinestep.insert(it.key(), 3);
								pubcom.bdonlinetimer.insert(it.key(),pubtimermng.addtimer());
							}

						}
						else
						{
							pubcom.bdonlinestep.insert(it.key(), 3);
							pubcom.bdonlinetimer.insert(it.key(),pubtimermng.addtimer());
						}
					}

				}//接收版本测试CAN命令应答
				else if(it.value() == 3)
				{
					tcmd tempcmd;

					//时间内
					if (pubtimermng.gettimerval(pubcom.bdonlinetimer.value(it.key())) <= OUTTIME)
					{
						if (candevice.getcmd(it.key(), VERTESTCMD, &tempcmd) == true)
						{
							if (pubcom.chkver(tempcmd.para1, tempcmd.para2) == true)
							{
								//OK,不处理

							}
							else//版本不适配
							{
								//设置版本故障
								publiccaution.addcaution(pubcom.bdcautionmap.value(
									it.key()), VERTESTCMD);
							}
							pubtimermng.deltimer(pubcom.bdonlinetimer.value(it.key()));					
							pubcom.bdonlinestep.insert(it.key(), 4);
						}
					}
					else //超时
					{
						pubtimermng.deltimer(pubcom.bdonlinetimer.value(it.key()));					
						pubcom.bdonlinestep.insert(it.key(), 4);
					}
				}//参数应用
				else if (it.value() == 4)
				{
					//清除命令
					pubcom.bdonlineparacmdpair[it.key()].clear();

					//浆泵板子参数应用
					if(it.key() == BUMPIOBDID)
					{
						bool ok;
						QDomNode para;
						tbdonlineparacmdpair t;
						tcmd tempcmd;

						for (int i = 1; i < 16; ++i)
						{
							QString paraname = QString("浆泵") + QString::number(i) + QString("输出点");
							para = GetParaByName("machsub4", paraname);

							tempcmd.cmd = SETBUMPIONOCMD; ;
							tempcmd.id = PCCANBDID;
							tempcmd.para1 = i-1;
							tempcmd.para2
								= para.firstChildElement("value").text().toInt(&ok) - 1;

							t.sendcmd.canid = pubcom.bdnodemap.value(BUMPIOBDID);
							t.sendcmd.cmd = tempcmd; 
							t.sendcmd.sendsucceed = false;

							tempcmd.cmd = SETBUMPIONOCMD;
							tempcmd.id = BUMPIOBDID;
							tempcmd.para1 = i-1;

							t.recvcmd.cmd = tempcmd;
							t.recvcmd.recvsucceed = false;
							pubcom.bdonlineparacmdpair[it.key()].append(t);

						}
					}
					//磁台板子
					else if(it.key() == MAGICIOBDID)
					{
						bool ok;
						QDomNode para;
						tbdonlineparacmdpair t;
						tcmd tempcmd;

						for (int i = 1; i < 16; ++i)
						{
							QString paraname = QString("磁台") + QString::number(i) + QString("输出点");
							para = GetParaByName("machsub5", paraname);

							tempcmd.cmd = SETMAGICIONOCMD;
							tempcmd.id = PCCANBDID;
							tempcmd.para1 = i-1;
							tempcmd.para2
								= para.firstChildElement("value").text().toInt(&ok) - 1;

							t.sendcmd.canid = pubcom.bdnodemap.value(MAGICIOBDID);
							t.sendcmd.cmd = tempcmd; 
							t.sendcmd.sendsucceed = false;

							tempcmd.cmd = SETMAGICIONOCMD;
							tempcmd.id = MAGICIOBDID;
							tempcmd.para1 = i-1;

							t.recvcmd.cmd = tempcmd;
							t.recvcmd.recvsucceed = false;
							pubcom.bdonlineparacmdpair[it.key()].append(t);

						}

						for (int i = 1; i < 16; ++i)
						{
							QString paraname = QString("磁台") + QString::number(i) + QString("模拟量输出点");
							para = GetParaByName("machsub6", paraname);

							tempcmd.cmd = SETMAGICANANOCMD;
							tempcmd.id = PCCANBDID;
							tempcmd.para1 = i-1;
							tempcmd.para2
								= para.firstChildElement("value").text().toInt(&ok) - 1;

							t.sendcmd.canid = pubcom.bdnodemap.value(MAGICIOBDID);
							t.sendcmd.cmd = tempcmd; 
							t.sendcmd.sendsucceed = false;

							tempcmd.cmd = SETMAGICANANOCMD;
							tempcmd.id = MAGICIOBDID;
							tempcmd.para1 = i-1;

							t.recvcmd.cmd = tempcmd;
							t.recvcmd.recvsucceed = false;
							pubcom.bdonlineparacmdpair[it.key()].append(t);
						}

					}
					//网头板子
					else if (it.key() >= PRINT1BDID 
						&& it.key() <= PRINT16BDID)
					{
						bool ok;
						QDomNode para;
						tbdonlineparacmdpair t;
						tcmd tempcmd;

						para = GetParaByName("prod", "抬网延时");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x03;
						tempcmd.para2
							= para.firstChildElement("value").text().toInt(&ok);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x03;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);


						para = GetParaByName("prod", "下网延时");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x04;
						tempcmd.para2
							= para.firstChildElement("value").text().toInt(&ok);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x04;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);


						para = GetParaByName("prod", "横向对花速度");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x01;
						tempcmd.para2
							= para.firstChildElement("value").text().toInt(&ok);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x01;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						para = GetParaByName("prod", "斜向对花速度");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x02;
						tempcmd.para2
							= para.firstChildElement("value").text().toInt(&ok);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x02;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						para = GetParaByName("prod", "网头调速允许");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x05;
						if (para.firstChildElement("value").text().toInt(&ok) == 0)
						{
							tempcmd.para2 = 2;
						}
						else
						{
							tempcmd.para2 = 1;
						}

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x05;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);


						para = GetParaByName("prod", "横/斜向电机强制开");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x07;
						if (para.firstChildElement("value").text().toInt(&ok) == 0)
						{
							tempcmd.para2 = 2;
						}
						else
						{
							tempcmd.para2 = 1;
						}

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x07;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						para = GetParaByName("prod", "抬网倒转延时");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x08;
						tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x08;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						para = GetParaByName("prod", "浆泵开启时间");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x06;
						tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x06;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						para = GetParaByName("prod", "磁台初始磁力");

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x09;
						tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x09;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						para = GetParaByName("machsub1", "抬/下网气缸延时");

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x02;
						tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x02;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);


						para = GetParaByName("machsub1", "张/松网气缸延时");

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x03;
						tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x03;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						para = GetParaByName("machsub1", "锁/松料杆气缸延时");

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x04;
						tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x04;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						para = GetParaByName("machsub1", "料杆升降气缸延时");

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x05;
						tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x05;
						tempcmd.para2 = 0x00;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						QString paraname = QString("网头") + QString::number(it.key()+ 1) + QString("对应浆泵编号");
						para = GetParaByName("machsub3", paraname);

						tempcmd.cmd = SETBUMPNOCMD ;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = it.key();
						tempcmd.para2
							= para.firstChildElement("value").text().toInt(&ok) - 1;

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETBUMPNOCMD;
						tempcmd.id = it.key();
						tempcmd.para1 = it.key();

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						//全机套色数
						tempcmd.cmd = SETPRINTMACHPARACMD ;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x01;
						tempcmd.para2 = pubcom.PrintNum;

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETPRINTMACHPARACMD;
						tempcmd.id = it.key();
						tempcmd.para1 = 0x01;

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

						//网头的初始状态
						tempcmd.cmd = SETMACHSTATUSCMD ;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = it.key();
						tempcmd.para2 = pubcom.presetprintstatus[it.key()];

						t.sendcmd.canid = pubcom.bdnodemap.value(it.key());
						t.sendcmd.cmd = tempcmd; 
						t.sendcmd.sendsucceed = false;

						tempcmd.cmd = SETMACHSTATUSCMD;
						tempcmd.id = it.key();
						tempcmd.para1 = pubcom.presetprintstatus[it.key()];

						t.recvcmd.cmd = tempcmd;
						t.recvcmd.recvsucceed = false;
						pubcom.bdonlineparacmdpair[it.key()].append(t);

					}

					//如果命令列表为空
					if (pubcom.bdonlineparacmdpair[it.key()].isEmpty() == true)
					{
						pubcom.bdonlinestep.insert(it.key(),0);
					}
					else
					{
						pubcom.bdonlinestep.insert(it.key(),5);
					}

				}//初始化
				else if (it.value() == 5)
				{
					pubcom.bdonlinecount.insert(it.key(),0);
					pubcom.bdonlinetimer.insert(it.key(),0);
					pubcom.bdonlinestep.insert(it.key(),6);
				}
				//发送参数
				else if(it.value() == 6)
				{
					if (pubcom.bdonlinecount.value(it.key()) > 2)
					{
						//退出参数设置
						pubcom.bdonlinestep.insert(it.key(),0);

						//写事件
					}
					else
					{
						//计数器+1
						pubcom.bdonlinecount.insert(it.key(),pubcom.bdonlinecount.value(it.key()) + 1);

						bool sendonesucceed = false;
						QList<tbdonlineparacmdpair>::iterator itt;
						for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
						{
							//接收失败就发送
							if ((*itt).recvcmd.recvsucceed == false)
							{
								if (candevice.sendcmd((*itt).sendcmd.canid,
									&((*itt).sendcmd.cmd)) == false)
								{
									msleep(1);
									if (candevice.sendcmd((*itt).sendcmd.canid,
										&((*itt).sendcmd.cmd)) == false)
									{
										(*itt).sendcmd.sendsucceed = false;

										//产生事件，提醒用户注意
										publiccaution.addevent(
											pubcom.bdcautionmap.value(it.key()),
											SENDCMDFAILEVENT,"发送CAN命令失败");
									}
									else
									{
										sendonesucceed = true;
										(*itt).sendcmd.sendsucceed = true;
									}
								}
								else
								{
									sendonesucceed = true;
									(*itt).sendcmd.sendsucceed = true;
								}
							}

							//每个命令见间隔10ms
							msleep(10);
						}

						//只要一个发送成功了，就要接收应答，进入接收应答阶段
						if (sendonesucceed == true)
						{
							pubcom.bdonlinestep.insert(it.key(),7);
							pubcom.bdonlinetimer.insert(it.key(),pubtimermng.addtimer());
						}
						else
						{
							//step 依旧是 6，就会继续重试了
							//do nothing;
						}
					}
				}
				//接受参数应答
				else if(it.value() == 7)
				{
					QList<tbdonlineparacmdpair>::iterator itt;
					for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
					{
						//成功发送并且没有收到才接收
						if ( (*itt).sendcmd.sendsucceed == true 
							&& (*itt).recvcmd.recvsucceed == false )
						{
							//对于浆泵编号设置命令，要特殊处理
							if ((*itt).sendcmd.cmd.cmd == SETBUMPNOCMD)
							{
								QList<tcmd> cmdlist;
								int count = candevice.getcmd((*itt).recvcmd.cmd.id,
									(*itt).recvcmd.cmd.cmd, cmdlist);

								//接收到了
								if (count >= 1)
								{
									(*itt).recvcmd.recvsucceed = true;
								}
							} 
							else
							{
								QList<tcmd> cmdlist;
								int count = candevice.getcmd((*itt).recvcmd.cmd.id,
									(*itt).recvcmd.cmd.cmd,(*itt).recvcmd.cmd.para1, cmdlist);

								//接收到了
								if (count >= 1)
								{
									(*itt).recvcmd.recvsucceed = true;
								}
							}
						}
					}

					if (pubtimermng.gettimerval(pubcom.bdonlinetimer.value(it.key())) < OUTTIME)
					{
						bool isallget = true;//是否全部接收到，特指本次发送的是否全部收到

						for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
						{
							//成功发送才接收
							if ((*itt).sendcmd.sendsucceed
								== true)
							{
								if ((*itt).recvcmd.recvsucceed == false)
								{
									isallget = false;
									break;
								}
							}
						}

						if (isallget == true)
						{
							pubtimermng.deltimer(pubcom.bdonlinetimer.value(it.key()));

							//全部是否都收到应答
							bool hasrecvfail = false;

							for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
							{
								if ((*itt).recvcmd.recvsucceed == false)
								{
									hasrecvfail = true;
									break;
								}

							}

							if (hasrecvfail == true)
							{
								pubcom.bdonlinestep.insert(it.key(),6);
							}
							else
							{
								pubcom.bdonlinestep.insert(it.key(),0);

								//记录事件
							}
						}
						else
						{
							//do nothing
						}
					}
					else
					{
						//超时
						pubtimermng.deltimer(pubcom.bdonlinetimer.value(it.key()));
						bool hasrecvfail = false;

						for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
						{
							if ((*itt).recvcmd.recvsucceed == false)
							{
								hasrecvfail = true;
								break;
							}
						}

						if (hasrecvfail == true)
						{
							pubcom.bdonlinestep.insert(it.key(),6);

						}
						else
						{
							pubcom.bdonlinestep.insert(it.key(),0);

							//记录事件
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//界面动作命令应答转发给界面，磁台满磁
		////////////////////////////////////////////////////////////////////////////

		for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
		{
			if ((*i) == MAGICIOBDID)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((*i), MAGICFULLCMD, PCCANBDID,
					cmdlist);

				//如果有请求
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//处理代码，待填充

					}
				}
			}
		}


		//PLC请求处理
		if ( pubcom.PlcStartUpFlag == true )
		{
			pubcom.PlcStartUpFlag = false;

			if (pubcom.statuschgstep == 0 
				&& pubcom.plcreqstartstep == 0)
			{
				//准备状态下才处理开机
				if(pubcom.machprintstatus == 1)
				{
					//处理代码，设置启动动作标识
					pubcom.plcreqstartstep = 1;
				}
			}
		}
        if ( pubcom.PlcStopUpFlag == true  )
		{
			pubcom.PlcStopUpFlag = false;
			if (pubcom.statuschgstep == 0 
				&& pubcom.plcreqstartstep == 0)
			{
				//运行状态下面才处理停机
				if(pubcom.machprintstatus == 2)
				{
					//机器状态立即设置
					pubcom.machprintstatus = 1;
					pubcom.plcreqstopstep = 1;
				}
			}

		}

		//修改网头板子状态
		//初始化
		if (pubcom.statuschgstep == 1)
		{
			pubcom.statuschgcount = 0;
			pubcom.statuschgtimer = 0;
			pubcom.statuschgstep = 2;

			QList<int>::iterator it;
			pubcom.activeprintlistmutex.lockForRead();

			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				pubcom.statuschgsendcmdmap.insert(*it, false);
				pubcom.statuschganscmdmap.insert(*it, false);
			}
			pubcom.activeprintlistmutex.unlock();

		}//发送到网头
		else if (pubcom.statuschgstep == 2)
		{
			if (pubcom.statuschgcount > 1)
			{
				pubcom.statuschgstep = 4;
			}
			else
			{
				pubcom.statuschgcount++;

				bool sendonesucceed = false;

				//命令
				tcmd tempcmd;
				tempcmd.cmd = SETMACHSTATUSCMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x00;

				switch (pubcom.machprintstatus)
				{
				case 1:
					tempcmd.para2 = 0x03;
					break;

				case 2:
					tempcmd.para2 = 0x05;
					break;

				default:
					break;
				}

				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//没有成功发送命令
					if (pubcom.statuschgsendcmdmap.value(*it) == false)
					{
						tempcmd.para1 = *it;

						//发送命令
						if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
								&tempcmd) == false)
							{
								//产生事件，提醒用户注意
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"发送CAN命令失败");

							}
							else
							{
								sendonesucceed = true;
								pubcom.statuschgsendcmdmap.insert(*it, true);
							}
						}
						else
						{
							sendonesucceed = true;
							pubcom.statuschgsendcmdmap.insert(*it, true);
						}
					}

				}
				pubcom.activeprintlistmutex.unlock();

				if (sendonesucceed == true)
				{
					pubcom.statuschgstep = 3;
					pubcom.statuschgtimer = pubtimermng.addtimer();
				}
			}

		}//收网头命令
		else if (pubcom.statuschgstep == 3)
		{
			tcmd tempcmd;
			tempcmd.cmd = SETMACHSTATUSCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = 0x00;
			tempcmd.para2 = 0x00;

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//等返回
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				//成功发送了命令
				if (pubcom.statuschgsendcmdmap.value(*it) == true
					&& pubcom.statuschganscmdmap.value(*it) == false)
				{
					//接收命令
					if (candevice.getcmd((*it), SETMACHSTATUSCMD, &tempcmd)
						== true)
					{
						//是否准备好
						//if (tempcmd.para2 == 0x01)
						//{
						//	pubcom.printreadymap.insert(*it, true);
						//}
						pubcom.statuschganscmdmap.insert(*it, true);
					}
				}
			}
			pubcom.activeprintlistmutex.unlock();

			//时间内
			if (pubtimermng.gettimerval(pubcom.statuschgtimer) <= OUTTIME)
			{
				//是否全部收到
				bool allget = true;

				QMap<int, bool>::iterator it;
				for (it = pubcom.statuschgsendcmdmap.begin(); it
					!= pubcom.statuschgsendcmdmap.end(); ++it)
				{
					if (it.value() == true && pubcom.statuschganscmdmap.value(
						it.key()) == false)
					{
						allget = false;
						break;
					}
				}
				if (allget == true)
				{
					pubtimermng.deltimer(pubcom.statuschgtimer);

					//接收失败标识
					bool hasrecvfail = false;

					for (it = pubcom.statuschganscmdmap.begin(); it
						!= pubcom.statuschganscmdmap.end(); ++it)
					{
						if (it.value() == false)
						{
							hasrecvfail = true;
							break;
						}
					}

					if (hasrecvfail == false)
					{
						pubcom.statuschgstep = 4; //没有失败的，那么进行下一步
					}
					else
					{
						pubcom.statuschgstep = 2; //重发

						for (it = pubcom.statuschganscmdmap.begin(); it
							!= pubcom.statuschganscmdmap.end(); ++it)
						{
							if (it.value() == false)
							{
								//重置接收失败的发送标识，可以重新发送
								pubcom.statuschgsendcmdmap.insert(it.key(),
									false);
							}
						}
					}
				}
				else //没有全部都收到，继续等
				{
					//do nothing
				}
			}
			else //超时
			{
				QMap<int, bool>::iterator it;

				pubtimermng.deltimer(pubcom.statuschgtimer);

				//接收失败标识
				bool hasrecvfail = false;

				for (it = pubcom.statuschganscmdmap.begin(); it
					!= pubcom.statuschganscmdmap.end(); ++it)
				{
					if (it.value() == false)
					{
						hasrecvfail = true;
						break;
					}
				}

				if (hasrecvfail == false)
				{
					pubcom.statuschgstep = 4;
				}
				else
				{
					pubcom.statuschgstep = 2;
					for (it = pubcom.statuschganscmdmap.begin(); it
						!= pubcom.statuschganscmdmap.end(); ++it)
					{
						if (it.value() == false)
						{
							//重置接收失败的发送标识，可以重新发送
							pubcom.statuschgsendcmdmap.insert(it.key(), false);
						}
					}
				}
			}
		}//完成设置标识
		else if (pubcom.statuschgstep == 4)
		{
			pubcom.statuschgcount = 0;
			pubcom.statuschgtimer = 0;
			pubcom.statuschgstep = 0;
		}

		//管理对PLC的故障和错误位
		if (publiccaution.IOHasCaution() == true)
		{

			//进入IO故障
			if (pubcom.IOCautionStatus == false)
			{
				pubcom.IOCautionStatus = true;
				WritePcCauBit(&regs,true);
			} 
		} 
		else
		{
			//退出IO故障
			if (pubcom.IOCautionStatus == true)
			{
				pubcom.IOCautionStatus = false;
				WritePcCauBit(&regs,false);
			}
		}


		if (publiccaution.IOHasErr() == true)
		{
			//进入IO错误
			if (pubcom.IOErrStatus == false)
			{
				pubcom.IOErrStatus = true;
                WritePcErrBit(&regs,true);

			}
		} 
		else
		{
			//退出IO错误
			if (pubcom.IOErrStatus == true)
			{
				pubcom.IOErrStatus = false;
				WritePcErrBit(&regs,false);
			}
		}

		//运行时是否有下放磁台未加磁
		{
			QList<int>::iterator it;
			bool magicflag = false;

			pubcom.activeprintlistmutex.lockForRead();

			//等返回
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				if((pubcom.machprintstatus == 2) 
					&& (pubcom.updownstatus[*it] == 0)
					&& (pubcom.magicstatus[*it] == 0))
				{
					magicflag = true;
					break;
				}
			}
			pubcom.activeprintlistmutex.unlock();
			WriteMagicCauBit(&regs,magicflag);
		}
		
		//写网头和磁台的状态到寄存器
		do{

			//是否工作状态高字节
			regs.writearray[3*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.presetprintstatus[8] == 0x03))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.presetprintstatus[9] == 0x03))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.presetprintstatus[10] == 0x03))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.presetprintstatus[11] == 0x03))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.presetprintstatus[12] == 0x03))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.presetprintstatus[13] == 0x03))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.presetprintstatus[14] == 0x03))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.presetprintstatus[15] == 0x03))<<7);
			//是否工作状态低字节
			regs.writearray[3*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.presetprintstatus[0] == 0x03))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.presetprintstatus[1] == 0x03))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.presetprintstatus[2] == 0x03))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.presetprintstatus[3] == 0x03))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.presetprintstatus[4] == 0x03))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.presetprintstatus[5] == 0x03))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.presetprintstatus[6] == 0x03))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.presetprintstatus[7] == 0x03))<<7);

			//网头下放状态高字节
			regs.writearray[4*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.updownstatus[8] == 0))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.updownstatus[9] == 0))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.updownstatus[10] == 0))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.updownstatus[11] == 0))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.updownstatus[12] == 0))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.updownstatus[13] == 0))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.updownstatus[14] == 0))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.updownstatus[15] == 0))<<7);
			//网头下放状态低字节
			regs.writearray[4*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.updownstatus[0] == 0))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.updownstatus[1] == 0))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.updownstatus[2] == 0))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.updownstatus[3] == 0))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.updownstatus[4] == 0))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.updownstatus[5] == 0))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.updownstatus[6] == 0))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.updownstatus[7] == 0))<<7);

			//磁台工作状态高字节
			regs.writearray[5*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.magicstatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.magicstatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.magicstatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.magicstatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.magicstatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.magicstatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.magicstatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.magicstatus[15] == 1))<<7);
			//磁台工作状态低字节
			regs.writearray[5*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.magicstatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.magicstatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.magicstatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.magicstatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.magicstatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.magicstatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.magicstatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.magicstatus[7] == 1))<<7);

			//对零状态高字节
			regs.writearray[6*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.zerostatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.zerostatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.zerostatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.zerostatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.zerostatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.zerostatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.zerostatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.zerostatus[15] == 1))<<7);
			//对零状态低字节
			regs.writearray[6*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.zerostatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.zerostatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.zerostatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.zerostatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.zerostatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.zerostatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.zerostatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.zerostatus[7] == 1))<<7);

			//反转状态高字节
			regs.writearray[7*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.circlestatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.circlestatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.circlestatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.circlestatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.circlestatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.circlestatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.circlestatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.circlestatus[15] == 1))<<7);
			//反转状态低字节
			regs.writearray[7*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.circlestatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.circlestatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.circlestatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.circlestatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.circlestatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.circlestatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.circlestatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.circlestatus[7] == 1))<<7);


			//松张网状态高字节
			regs.writearray[8*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.lockreleasestatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.lockreleasestatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.lockreleasestatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.lockreleasestatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.lockreleasestatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.lockreleasestatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.lockreleasestatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.lockreleasestatus[15] == 1))<<7);
			//松张网状态低字节
			regs.writearray[8*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.lockreleasestatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.lockreleasestatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.lockreleasestatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.lockreleasestatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.lockreleasestatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.lockreleasestatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.lockreleasestatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.lockreleasestatus[7] == 1))<<7);
			//料杆锁定状态高字节
			regs.writearray[9*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.colorpipestatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.colorpipestatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.colorpipestatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.colorpipestatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.colorpipestatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.colorpipestatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.colorpipestatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.colorpipestatus[15] == 1))<<7);
			//料杆锁定状态低字节
			regs.writearray[9*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.colorpipestatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.colorpipestatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.colorpipestatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.colorpipestatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.colorpipestatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.colorpipestatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.colorpipestatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.colorpipestatus[7] == 1))<<7);
		}while(0);


		//作为一个告警，提醒用户，网络不是很健康
		if (candevice.getcmdcount() > UNKNOWCMDNUM)
		{
			//产生事件，提醒用户注意
			publiccaution.addevent(CANCAUTIONLOCATION, TOOMUCHUNUSEDCMDEVENT);
		}

		msleep(1);//必要的休眠还是要的，否则过多的时间耗费在空循环
	}
}


