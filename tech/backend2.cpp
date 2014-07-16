/*
* backend2.cpp
*
*  Created on: 2009-8-8
*      Author: 胡志文
*/
#include <windows.h>
#include "backend2.h"
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

#define QT_NO_DEBUG_OUTPUT

//全局的can设备
extern canif candevice;
extern QObject* setobject;
extern timermng pubtimermng;
extern QObject* opobject;


//A4N电机
extern CKSMCA4 *ksmc;

//数据库
extern DB mydb;

void restoreTaskRun(struct restore* pTask);
void savelocTaskRun(struct saveloc* pTask);

//零速维持时间
static zerospeedtimer = 0;

backend2::backend2()
{

}

backend2::~backend2()
{

}

void backend2::run()
{
	tcmd tempcmd;

	while (pubcom.quitthread == false)
	{
		//收命令
		candevice.refresh();

		//对零处理
		{
			QList<int>::iterator it;

			//多线程保护
			pubcom.activeprintlistmutex.lockForRead();
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{

				//高速正转
				if (pubcom.actionzerostep.value(*it) == 1)
				{
					//新增定时器
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

					//高速正转
					ksmc->MoveHomeHigh(*it + 1, 1);

					//标识
					pubcom.actionzerostep.insert(*it, 2);
				}
				//等待零位信号
				else if (pubcom.actionzerostep.value(*it) == 2)
				{
					//接受CAN命令
					QList<tcmd> cmdlist;
					int count = candevice.getcmd((*it), ZEROLOCCMD, cmdlist);

					//如果有零位信号
					if (count >= 1)
					{
						double dd = 0.0;
						ksmc->GetLocation(*it + 1, dd);

						pubcom.actionzeroloc1.insert(*it, dd);

						//应答命令
						tempcmd.cmd = ZEROLOCCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = *it;
						tempcmd.para2 = 0x01;
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
						}

						//删除老的定时器
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//标识
						pubcom.actionzerostep.insert(*it, 3);

					}
					//等待是否超时
					else if (pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) > ZEROOUTTIME)
					{
						//删除老的定时器
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//停止转动
						ksmc->MoveStop(*it + 1);

						//标识,退出对零
						pubcom.actionzerostep.insert(*it, 0);
					}
				}
				else if (pubcom.actionzerostep.value(*it) == 3)
				{

					double PulsePerUnit = 0.0;
					ksmc->GetPulsePerUnit(PulsePerUnit);

					double dd = pubcom.actionzeroloc1.value(*it) - PulsePerUnit * (pubcom.ZeroHighSpeedWidth + pubcom.ZeroDetectWidth);

					ksmc->MoveAbsPTP(*it + 1, dd);

					pubcom.actionzerostep.insert(*it, 4);

					//新增定时器
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());
				}
				//延时等指令下发完，不能马上查询是否到位，否则查询的是上一次的
				else if(pubcom.actionzerostep.value(*it) == 4)
				{
					if(pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) >= 2)
					{
						pubcom.actionzerostep.insert(*it, 5);
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));
					}
				}
				//正转到位
				else if(pubcom.actionzerostep.value(*it) == 5)
				{
					int status = 0;
					ksmc->GetMotorInPosStatus(*it+1,status);
					if ( status == 1 )
					{
						//标识
						pubcom.actionzerostep.insert(*it, 6);
					}
				}
				//反转到某位置
				else if(pubcom.actionzerostep.value(*it) == 6)
				{
					double PulsePerUnit = 0.0;
					ksmc->GetPulsePerUnit(PulsePerUnit);

					double dd = pubcom.actionzeroloc1.value(*it) - PulsePerUnit * (pubcom.ZeroLowSpeedWidth + pubcom.ZeroDetectWidth);

					//移动到接近开关附近位置
					ksmc->MoveAbsPTP(*it + 1, dd);

					//标识
					pubcom.actionzerostep.insert(*it, 7);

					//新增定时器
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

				}
				//延时等指令下发完
				else if(pubcom.actionzerostep.value(*it) == 7)
				{
					if(pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) >= 2)
					{
						pubcom.actionzerostep.insert(*it, 8);
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));
					}
				}
				//反转到位
				else if(pubcom.actionzerostep.value(*it) == 8)
				{
					int status = 0;
					ksmc->GetMotorInPosStatus(*it+1,status);
					if ( status == 1 )
					{
						//标识
						pubcom.actionzerostep.insert(*it, 9);
					}
				}//慢速反转
				else if (pubcom.actionzerostep.value(*it) == 9)
				{
					//新增定时器
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

					//反转
					ksmc->MoveHome(*it + 1, 0);

					//标识
					pubcom.actionzerostep.insert(*it, 10);
				}
				//等零位信号
				else if (pubcom.actionzerostep.value(*it) == 10)
				{
					//接受CAN命令
					QList<tcmd> cmdlist;
					int count = candevice.getcmd((*it), ZEROLOCCMD, cmdlist);

					//如果有请求
					if (count >= 1)
					{
						double dd = 0.0;
						ksmc->GetLocation(*it + 1, dd);
						pubcom.actionzeroloc2.insert(*it, dd);

						//应答命令
						tempcmd.cmd = ZEROLOCCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = *it;
						tempcmd.para2 = 0x01;
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
						}

						//删除定时器
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//标识
						pubcom.actionzerostep.insert(*it, 11);

					}
					//等待是否超时
					else if (pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) > ZEROOUTTIME)
					{
						//删除老的定时器
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//停止转动
						ksmc->MoveStop(*it + 1);

						//标识,退出对零
						pubcom.actionzerostep.insert(*it, 0);
					}
				}//反转到某位置
				else if (pubcom.actionzerostep.value(*it) == 11)
				{
					double PulsePerUnit = 0.0;
					ksmc->GetPulsePerUnit(PulsePerUnit);

					double dd = pubcom.actionzeroloc1.value(*it) + PulsePerUnit * (pubcom.ZeroHighSpeedWidth);

					ksmc->MoveAbsPTP(*it + 1, dd);

					//标识
					pubcom.actionzerostep.insert(*it, 12);

					//新增定时器
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

				}
				//延时等指令下发完
				else if(pubcom.actionzerostep.value(*it) == 12)
				{
					if(pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) >= 2)
					{
						pubcom.actionzerostep.insert(*it, 13);
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));
					}
				}
				//反转到位
				else if (pubcom.actionzerostep.value(*it) == 13)
				{
					int status = 0;
					ksmc->GetMotorInPosStatus(*it+1,status);
					if ( status == 1 )
					{
						//标识
						pubcom.actionzerostep.insert(*it, 14);
					}

				}
				//正转到某位置
				else if (pubcom.actionzerostep.value(*it) == 14)
				{
					double PulsePerUnit = 0.0;
					ksmc->GetPulsePerUnit(PulsePerUnit);

					double dd = pubcom.actionzeroloc1.value(*it) + PulsePerUnit * (pubcom.ZeroLowSpeedWidth);

					//移动到接近开关附近位置
					ksmc->MoveAbsPTP(*it + 1, dd);

					//标识
					pubcom.actionzerostep.insert(*it, 15);

					//新增定时器
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

				}
				//延时等动作做完
				else if(pubcom.actionzerostep.value(*it) == 15)
				{
					if(pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) >= 2)
					{
						pubcom.actionzerostep.insert(*it, 16);
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));
					}
				}

				//正转到位
				else if (pubcom.actionzerostep.value(*it) == 16)
				{
					int status = 0;
					ksmc->GetMotorInPosStatus(*it+1,status);
					if ( status == 1 )
					{
						//标识
						pubcom.actionzerostep.insert(*it, 17);
					}
				}
				//慢速正转
				else if (pubcom.actionzerostep.value(*it) == 17)
				{
					//新增新的定时器
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

					//正转
					ksmc->MoveHome(*it + 1, 1);

					//标识
					pubcom.actionzerostep.insert(*it, 18);
				}//等零位信号
				else if (pubcom.actionzerostep.value(*it) == 18)
				{
					//接受CAN命令
					QList<tcmd> cmdlist;
					int count = candevice.getcmd((*it), ZEROLOCCMD, cmdlist);

					//如果有请求
					if (count >= 1)
					{
						double dd = 0.0;
						ksmc->GetLocation(*it + 1, dd);
						pubcom.actionzeroloc3.insert(*it, dd);

						//应答命令
						tempcmd.cmd = ZEROLOCCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = *it;
						tempcmd.para2 = 0x01;
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
						}

						//删除老的定时器
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//标识
						pubcom.actionzerostep.insert(*it, 19);

					}
					//等待是否超时
					else if (pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) > ZEROOUTTIME)
					{
						//删除老的定时器
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//停止转动
						ksmc->MoveStop(*it + 1);

						//标识,退出对零
						pubcom.actionzerostep.insert(*it, 0);

					}
				}
				else if (pubcom.actionzerostep.value(*it) == 19)
				{
					//求位置

					int cir = 0;
					ksmc->GetPulsePerBigCircle(cir);

					if (cir > 0)
					{
						double a1,a2,a3;
						a1 = pubcom.actionzeroloc1.value(*it);
						a2 = pubcom.actionzeroloc2.value(*it);
						a3 = pubcom.actionzeroloc3.value(*it);

						//零位位置,2次求平均
						double dd = (a2 + a3)/2;

						//计算网头的偏移量导致的调整量
						double offset = 0;
						ksmc->GetOffset(offset);

						//和1号网头的距离，考虑到安装偏差
						double Moffset1 = 0;   //1号网头的安装偏差
						double Moffset2 = 0;   //对零网头的安装偏差
						ksmc->GetMotor1Offset(Moffset1);
						ksmc->GetMotorOffset(*it + 1,Moffset2);

						//offset = (*it) * offset - Moffset1 + Moffset2;
						offset = - Moffset1 + Moffset2;

						//圆网周长
						//double girth = 0;
						//ksmc->GetGirth(girth);

						//取余，小数点后2位精度
						//offset = offset - girth * (int)(offset/girth);
						//offset = offset - girth * (int)(offset/girth);

						//单位长度的脉冲
						double pulseperunit = 0;
						ksmc->GetPulsePerUnit(pulseperunit);

						//偏移脉冲
						offset = offset * pulseperunit;

						//大圈脉冲数
						int bigpulsenum = 0;
						ksmc->GetPulsePerBigCircle(bigpulsenum);

						//对零最后动作正转(和橡毯工作方向一致)
						if(pubcom.ServoDir == 0)
							dd = dd - offset - 2*bigpulsenum;
						else
							dd = dd - offset + 2*bigpulsenum;


						//移动到零位
						ksmc->MoveAbsPTP(*it + 1, dd);

						//记忆网头的位置
						ksmc->SetPrintLocation(*it + 1,dd);
					}

					//标识
					pubcom.actionzerostep.insert(*it, 0);
				}
			}
			pubcom.activeprintlistmutex.unlock();

		}

		////////////////////////////////////////////////////////////////////////////
		//停车命令处理
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.plcreqstopstep == 1)
		{
			printf("开始停车\n");
			//电机停止定时器
			pubcom.plcreqstoptimer = pubtimermng.addtimer();
			pubcom.plcreqstopstep = 2;
		}
		else if (pubcom.plcreqstopstep == 2)
		{
			//等待0速度是否已经超时
			if (pubtimermng.gettimerval(pubcom.plcreqstoptimer) > MOTORSTOPTIME)
			{
				pubtimermng.deltimer(pubcom.plcreqstoptimer);

				//告警，这个必须生成告警
				pubcom.plcreqstopstep = 0; //停止处理流程
				printf("停车超时\n");
			}
			else
			{
				//采样主电机速度，看看是否已经停止
				if ( (abs(pubcom.motorspeed*10) <= 2) )//停止跟随时的速度0.2
				{
					zerospeedtimer = pubtimermng.addtimer();
					pubcom.plcreqstopstep = 3;
					pubtimermng.deltimer(pubcom.plcreqstoptimer);
				}
			}
		}
		else if(pubcom.plcreqstopstep == 3)
		{
			//维持0速度时间是否已经OK
			if (pubtimermng.gettimerval(zerospeedtimer) > 20)
			{
				pubtimermng.deltimer(zerospeedtimer);

				printf("成功停车\n");
				QList<int>::iterator it;

				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					if (pubcom.presetprintstatus[*it] == 3)
					{
						//停止跟随
						ksmc->DisableMasterFollowing(*it + 1);
						//printf("停止跟随%d\n",*it + 1);


						int networkcanset = -1;

						ksmc->GetDriverPar(*it+1,10,networkcanset);
						//参数为0则表示可以通过网络设置
						if (networkcanset == 0)
						{
							ksmc->SetDriverPar(*it+1,16,10);
							ksmc->SetDriverPar(*it+1,17,10);
							ksmc->SetDriverPar(*it+1,18,10);
							ksmc->SetDriverPar(*it+1,19,5);
						}

						//启动记忆位置任务
						if(pubcom.savelocTask[*it].step == 0)
						{
							pubcom.savelocTask[*it].step = 1;
						}
						//退出跟随后记忆位置
						//ksmc->RememberPrintLoction(*it + 1);

					}
				}
				pubcom.activeprintlistmutex.unlock();
				pubcom.plcreqstopstep = 4;
			}
		}
		//发送到网头
		else if (pubcom.plcreqstopstep == 4)
		{
			//清空
			pubcom.sendprintcmdmap.clear();

			//命令
			tcmd tempcmd;
			tempcmd.cmd = SETMACHSTATUSCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = 0x00;
			tempcmd.para2 = 0x03;

			QList<int>::iterator it;
			pubcom.activeprintlistmutex.lockForRead();

			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				if (pubcom.presetprintstatus[*it] == 3)
				{
					tempcmd.para1 = *it;

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
						pubcom.sendprintcmdmap.insert(*it, true);
					}
				}
			}
			pubcom.activeprintlistmutex.unlock();
			if(pubcom.sendprintcmdmap.isEmpty())
			{
				pubcom.plcreqstopstep = 0;
			}
			else
			{
				pubcom.plcreqstoptimer = pubtimermng.addtimer();
				pubcom.plcreqstopstep = 5;
			}
		}
		else if (pubcom.plcreqstopstep == 5)
		{
			//时间内
			if (pubtimermng.gettimerval(pubcom.plcreqstoptimer) <= OUTTIME)
			{
				//完成了接收
				if(pubcom.sendprintcmdmap.isEmpty())
				{
					pubcom.plcreqstopstep = 0;
					pubtimermng.deltimer(pubcom.plcreqstoptimer);
				}
				else//继续接收
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
						if (pubcom.presetprintstatus[*it] == 3)
						{
							//接收命令
							if (candevice.getcmd((*it), SETMACHSTATUSCMD, &tempcmd)
								== true)
							{
								pubcom.sendprintcmdmap.remove(*it);

							}
						}

					}
					pubcom.activeprintlistmutex.unlock();
				}
			}
			else
			{
				pubcom.plcreqstopstep = 0;
				pubtimermng.deltimer(pubcom.plcreqstoptimer);
				//error
			}
		}
		//记忆位置任务运行
		for(int iiii = 0;iiii < pubcom.PrintNum;iiii++)
		{
			savelocTaskRun(&(pubcom.savelocTask[iiii]));
		}


		msleep(1);//必要的休眠还是要的，否则过多的时间耗费在空循环
	}
}


