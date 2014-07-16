/*
* backend3.cpp
*
*  Created on: 2009-8-8
*      Author: 胡志文
*/

#include "backend3.h"
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

extern HANDLE hCom;//串口句柄

//数据库
extern DB mydb;
backend3::backend3()
{
	// TODO Auto-generated constructor stub
}

backend3::~backend3()
{
	// TODO Auto-generated destructor stub
}

//界面事件消息处理
void backend3::run()
{

	tcmd tempcmd;

	char str[8] = {1,0,0,0,0,0,0,0};
	DWORD wCount2;

	while (pubcom.quitthread == false)
	{
		tempcmd.cmd = COMTESTCMD;
		tempcmd.id = PCCANBDID;
		tempcmd.para1 = 0;
		tempcmd.para2 = 0;


		//初始化并发命令
		if (candevice.sendcmd(OUTIONODEID, &tempcmd) == false)
		{
			//休眠1ms再重试发送
			msleep(1);

			if (candevice.sendcmd(OUTIONODEID, &tempcmd) == false)
			{


			}
			else
			{
				WriteFile(hCom, str, 1, &wCount2, NULL);//发送数据
			}
		}
		else
		{
			WriteFile(hCom, str, 1, &wCount2, NULL);//发送数据
		}

		msleep(50);//必要的休眠还是要的，否则过多的时间耗费在空循环
	}

}


