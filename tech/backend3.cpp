/*
* backend3.cpp
*
*  Created on: 2009-8-8
*      Author: ��־��
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

//ȫ�ֵ�can�豸
extern canif candevice;
extern QObject* setobject;
extern timermng pubtimermng;
extern QObject* opobject;


//A4N���
extern CKSMCA4 *ksmc;

extern HANDLE hCom;//���ھ��

//���ݿ�
extern DB mydb;
backend3::backend3()
{
	// TODO Auto-generated constructor stub
}

backend3::~backend3()
{
	// TODO Auto-generated destructor stub
}

//�����¼���Ϣ����
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


		//��ʼ����������
		if (candevice.sendcmd(OUTIONODEID, &tempcmd) == false)
		{
			//����1ms�����Է���
			msleep(1);

			if (candevice.sendcmd(OUTIONODEID, &tempcmd) == false)
			{


			}
			else
			{
				WriteFile(hCom, str, 1, &wCount2, NULL);//��������
			}
		}
		else
		{
			WriteFile(hCom, str, 1, &wCount2, NULL);//��������
		}

		msleep(50);//��Ҫ�����߻���Ҫ�ģ���������ʱ��ķ��ڿ�ѭ��
	}

}


