/*
* backend.cpp
*
*  Created on: 2009-8-8
*      Author: ��־��
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


//����λ������ִ����
void restoreTaskRun(struct restore* pTask)
{
	if(pTask->step <= 0)
		return;

	if((pTask->step) == 1)
	{
		//������ʱ��
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
			//��ʶ
			pTask->step = 0;
		}
	}
}

//����λ������ִ����
void savelocTaskRun(struct saveloc* pTask)
{
	if(pTask->step <= 0)
		return;

	if((pTask->step) == 1)
	{
		//������ʱ��
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
			//��ʶ
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

//�����¼���Ϣ����
void backend::customEvent(QEvent * e)
{
	//�õ��ǽ�����¼�
	if (e->type() == INTERFACE_EVENT) 
	{
		interfaceevent* pinterfaceevent = (interfaceevent*) e;

		//��������ͷ����
		if (pinterfaceevent->cmd == 0x02 && pinterfaceevent->status == 0x00)
		{
			//�������������ͷ�����ǻ����
			if (pubcom.ismainuicmd != true 
				&& pubcom.opuicmdflag != true
				&& pubcom.isplccmd != true)
			{
				publiccaution.adddbginfo(QString("��̨������������ͷ�����¼�"));

				//���������ʶ
				pubcom.ismainuicmd = true;
				pubcom.mainuicmdstep = 1;
				pubcom.mainuidcmd = pinterfaceevent->data[0];
			}
			else//ԭ�������������������ͷ������������
			{
				publiccaution.adddbginfo(QString("��̨�ܾ���������ͷ�����¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x02;
				ptempevent->status = 0x02;
				ptempevent->data[0] = pinterfaceevent->data[0];

				//����һ���¼��������߳�
				QCoreApplication::postEvent(interfaceobject, ptempevent);
			}
		}
		//PLC�ϲ���
		else if (pinterfaceevent->cmd == 0x12 && pinterfaceevent->status == 0x00)
		{
			//�������������ͷ�����ǻ����
			if (pubcom.ismainuicmd != true 
				&& pubcom.opuicmdflag != true
				&& pubcom.isplccmd != true)
			{
				publiccaution.adddbginfo(QString("��̨����PLC��ͷ�����¼�"));

				//���������ʶ
				pubcom.isplccmd = true;
				pubcom.plccmdstep = 1;
				pubcom.plcdcmd = pinterfaceevent->data[0];
			}
			else//ԭ�������������������ͷ���������������֮
			{
				publiccaution.adddbginfo(QString("��̨�ܾ�PLC��ͷ�����¼�"));
			}
		}

		//��ͷ����������ͷ����
		else if (pinterfaceevent->cmd == 0x03 && pinterfaceevent->status == 0x00)
		{
			//����ԭ���Ƿ�����ͷ����ָ����ִ��,�������������ͷ��������
			if (pubcom.ismainuicmd != true 
				&& pubcom.opuicmdflag != true
				&& pubcom.isplccmd != true)
			{
				publiccaution.adddbginfo(QString("��̨������ͷ������������¼�"));

				pubcom.opuicmdflag = true;
				pubcom.opuicmdstep = 1;
				pubcom.opuicmdcmd = pinterfaceevent->data[0];
				pubcom.opuicmdprintno = pinterfaceevent->data[1];

			}
			else//�Ѿ��в�����ֱ�ӷ����������
			{
				publiccaution.adddbginfo(QString("��̨�ܾ���ͷ������������¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x03;
				ptempevent->status = 0x01;
				ptempevent->data[0] = pinterfaceevent->data[0];

				//����һ���¼��������߳�
				QCoreApplication::postEvent(opobject, ptempevent);
			}
		}
		//����Ӧ��
		else if (pinterfaceevent->cmd == 0x04 && pinterfaceevent->status == 0x00)
		{
			if (pubcom.isapply == true)
			{
				publiccaution.adddbginfo(QString("��̨�ܾ��������Ӧ���¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x04;
				ptempevent->status = 0x01;
				ptempevent->data[0] = 0x00;

				//����һ���¼��������߳�
				QCoreApplication::postEvent(setobject, ptempevent);
			}
			else if(pubcom.hasbdonline() == true)
			{
				publiccaution.adddbginfo(QString("��̨�ܾ��������Ӧ���¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x04;
				ptempevent->status = 0x04;
				ptempevent->data[0] = 0x00;

				//����һ���¼��������߳�
				QCoreApplication::postEvent(setobject, ptempevent);

			}
			else
			{
				publiccaution.adddbginfo(QString("��̨���ܽ������Ӧ���¼�"));

				//���ñ�ʶ
				pubcom.applyansplace = 1;
				pubcom.isapply = true;
				pubcom.applystep = 1;
				pubcom.hasparafailed = false;
				pubcom.hasparasucceed = false;
			}
		}
		//����ȷ��
		else if (pinterfaceevent->cmd == 0x05 && pinterfaceevent->status == 0x00)
		{
			if (pubcom.isapply == true)
			{
				publiccaution.adddbginfo(QString("��̨�ܾ��������ȷ���¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x05;
				ptempevent->status = 0x01;
				ptempevent->data[0] = 0x00;

				//����һ���¼��������߳�
				QCoreApplication::postEvent(interfaceobject, ptempevent);
			}
			else if(pubcom.hasbdonline() == true)
			{
				publiccaution.adddbginfo(QString("��̨�ܾ��������ȷ���¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x05;
				ptempevent->status = 0x04;
				ptempevent->data[0] = 0x00;

				//����һ���¼��������߳�
				QCoreApplication::postEvent(interfaceobject, ptempevent);
			}
			else
			{
				publiccaution.adddbginfo(QString("��̨���ܽ������ȷ���¼�"));

				//���ñ�ʶ
				pubcom.applyansplace = 2;
				pubcom.isapply = true;
				pubcom.applystep = 1;
				pubcom.hasparafailed = false;
				pubcom.hasparasucceed = false;
			}
		}
		//��ͷ���ٿ���
		else if (pinterfaceevent->cmd == 0x06 && pinterfaceevent->status == 0x00)
		{
			if (pubcom.printchgspeedansflag == true)
			{
				publiccaution.adddbginfo(QString("��̨�ܾ�������ͷ���Կ����¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x06;
				ptempevent->status = 0x01;
				ptempevent->data[0] = 0x00;

				//����һ���¼��������߳�
				QCoreApplication::postEvent(interfaceobject, ptempevent);
			}
			else
			{
				publiccaution.adddbginfo(QString("��̨���ܽ�����ͷ���Կ����¼�"));

				//���ñ�ʶ
				pubcom.printchgspeedansflag = true;
				pubcom.printchgspeedstep = 1;
				pubcom.printchgspeedfailed = false;
				pubcom.printchgspeedsucceed = false;
			}
		}
		//����ͷ���ٿ���
		else if (pinterfaceevent->cmd == 0x0b && pinterfaceevent->status == 0x00)
		{
			if (pubcom.printchgspeedansflag == false)
			{
				//���ñ�ʶ
				pubcom.printchgspeedansflag = true;
				pubcom.singleprintchgspeedstep = 1;
			}
		}
		//��ͷ��������߿���
		else if (pinterfaceevent->cmd == 0x07 && pinterfaceevent->status == 0x00)
		{
			//ԭ�����ڲ�������ô�����
			if (pubcom.printactiveflag == true)
			{
				publiccaution.adddbginfo(QString("��̨�ܾ�������ͷ����������¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x07;
				ptempevent->status = 0x03;
				ptempevent->data[0] = pinterfaceevent->data[0];

				//����һ���¼��������߳�
				QCoreApplication::postEvent(selectprintobject, ptempevent);
			}
			else
			{
				publiccaution.adddbginfo(QString("��̨���ܽ�����ͷ����������¼�"));

				//���ñ�ʶ
				pubcom.printactiveflag = true;
				pubcom.printactivestep = 1;
				pubcom.printactivenum = pinterfaceevent->data[0];
				pubcom.printactivecmd = pinterfaceevent->data[1];
			}
		}
		//ǿ��ˢ�²���
		else if (pinterfaceevent->cmd == 0x08 && pinterfaceevent->status == 0x00)
		{
			if (pubcom.isapply == true )
			{
				publiccaution.adddbginfo(QString("��̨�ܾ�������ͷǿ��ˢ�²����¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x04;
				ptempevent->status = 0x01;
				ptempevent->data[0] = 0x00;

				//����һ���¼��������߳�
				QCoreApplication::postEvent(setobject, ptempevent);
			}
			else if(pubcom.hasbdonline() == true)
			{
				publiccaution.adddbginfo(QString("��̨�ܾ�������ͷǿ��ˢ�²����¼�"));

				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x04;
				ptempevent->status = 0x04;
				ptempevent->data[0] = 0x00;

				//����һ���¼��������߳�
				QCoreApplication::postEvent(setobject, ptempevent);

			}
			else
			{
				publiccaution.adddbginfo(QString("��̨���ܽ�����ͷǿ��ˢ�²����¼�"));

				//���ñ�ʶ
				pubcom.applyansplace = 1;
				pubcom.isapply = true;
				pubcom.refreshapply = true;
				pubcom.applystep = 1;
				pubcom.hasparafailed = false;
				pubcom.hasparasucceed = false;
			}
		}
		//Ħ��ϵ������
		else if (pinterfaceevent->cmd == 0x0a && pinterfaceevent->status == 0x00)
		{
			publiccaution.adddbginfo(QString("��̨���ܽ�����̺�ƶ���ʼ�¼�"));

			//���ñ�ʶ
			pubcom.measurepulsestartstep = 1;
		}
		//Ħ��ϵ������
		else if (pinterfaceevent->cmd == 0x0a && pinterfaceevent->status == 0x01)
		{
			publiccaution.adddbginfo(QString("��̨���ܽ�����̺�ƶ�ֹͣ�¼�"));

			//���ñ�ʶ
			pubcom.measurepulsestopstep = 1;
		}
		else
		{
			publiccaution.adddbginfo(QString("����ʶ�Ľ����¼�"));
		}
	}
	else
	{
		publiccaution.adddbginfo(QString("����ʶ���¼�"));
	}


}

//�߳���ѭ��
void backend::run()
{
	//�ܸ����ͺ�ֵ
	float currentfollow = 0.0;
	float oldcurrentfollow = 0.0;

	while (pubcom.quitthread == false)
	{

		//CANˢ�¶�
		candevice.refresh();

		///////////////////////////////////////////////////////
		//�ڲ��¼������ڴ������¼���Ӧ������Ҫɾ���¼������е��¼�
		///////////////////////////////////////////////////////
		if (pubcom.eventqueue.count() > 0)
		{
			QList<tintevent>::iterator i;
			for (i = pubcom.eventqueue.begin(); i != pubcom.eventqueue.end(); ++i)
			{
				switch ((*i).eventid)
				{
				case 1://��������

					publiccaution.addevent(
						QString("����") + QString::number((*i).nodeid),
						"���������¼�","���������¼�");

					publiccaution.adddbginfo(QString("���������¼�"));

					if (pubcom.bdonlinestep.value((*i).nodeid) == 0)
					{
						pubcom.bdonlinestep.insert((*i).nodeid,1);
					}

					//ɾ�����Ӳ����߹���
					publiccaution.delcaution(pubcom.bdcautionmap.value(
						((*i).nodeid)), COMCAUTION);

					break;

				case 2://��������

					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("����") + QString::number((*i).nodeid),
						"���������¼�","���������¼�");

					//������ͷ���Ƿ��ǹ��ϣ��ÿ���ǰ��ͷ�Ƿ񼤻�
					if ((*i).nodeid >= PRINT1BDID && (*i).nodeid <= PRINT16BDID)
					{
						if (pubcom.presetprintstatus[(*i).nodeid] != 1 )
						{
							//���ð��Ӳ����߹���
							publiccaution.addcaution(pubcom.bdcautionmap.value(
								((*i).nodeid)), COMCAUTION);
						}
					}
					else
					{
						//���ð��Ӳ����߹���
						publiccaution.addcaution(pubcom.bdcautionmap.value(
							((*i).nodeid)), COMCAUTION);
					}
					break;

				case 3://��������¼�

					//�����¼��������û�ע��
					publiccaution.addevent(
						"����",
						"�������","��������¼�");

					publiccaution.addcaution(CANCAUTIONLOCATION, CANNETCAUTION);
					break;

				default:
					break;
				}
			}
		}
		pubcom.eventqueue.clear();


		//�˶������ϲ���
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
				//���ԭ��û��,����������
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

		//�˶�����������
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

		//�����������߹���

#ifdef  HAVESERIAL
		if(publiccaution.hascaution(SERIAL,SERIALNOEXIST) == false)
		{
		unsigned char heartchar = 0;
		ReadRegsReadHeatbeat(&regs,&heartchar);

		//�ܹ�100ms����һ�Σ�Ҳ����˵4����û���յ��������ݾ����Ǵ���������
		if (heartchar > 10)
		{
		if (pubcom.OfflineFlag == false)
		{
		pubcom.OfflineFlag = true;
		publiccaution.addcaution(SERIAL,SERIALLOST);
		}

		}
		else//�������߹�������
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
			//��ǰ����Ȳ�ͬ����������
			ksmc->SetFollowDelay(currentfollow);
			oldcurrentfollow = currentfollow;
			//printf("�����%f\n",currentfollow);
		}

		///////////////////////////////////////////////////////
		//�ⲿCAN���������
		///////////////////////////////////////////////////////
		//�������ӹ����ϱ�Ӧ��
		tcmd tempcmd;
		tempcmd.cmd = COMTESTCMD;
		tempcmd.id = PCCANBDID;
		tempcmd.para1 = 0;
		tempcmd.para2 = 0;

		QList<int>::iterator i;

		////////////////////////////////////////////////////////////////////////////
		//��̨״̬�ı�,��̨���ư�����ͷ�巢��CAN������ػ��������������Ӧ��
		////////////////////////////////////////////////////////////////////////////

		{
			for(int ii = 0; ii < pubcom.PrintNum; ++ii)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd(MAGICIOBDID, MAGICRSPCMD,ii,cmdlist);

				//���������
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						pubcom.magicpercent[(*j).para1] = (*j).para2;
						//�������ֵ����0��1
						if((*j).para2 >= 2)
						{
							//���浽���ݿ���
							mydb.SaveValue(QString("magic") + QString::number(ii),(*j).para2);						
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷ���ʹ�̨����,�����գ����账��
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
		//����״̬�ϱ���������գ����账��
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
		//��ͷ���ͽ�����������գ����账��
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
		//��ͷ���������Ӧ�𣺼Ӽ���
		////////////////////////////////////////////////////////////////////////////

		{
			//��ͷ״̬
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), CHGSPEEDCMD, 0x00, cmdlist);

				//���������
				if (count >= 1)
				{
					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("��ͷ") + QString::number(i+1),
						"�Ӽ�������","�յ���ͷ��Ӽ�������",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						switch((*j).para2) {
							case 0x01:
								//����ON
								WritePcMotorAccBit(&regs,true);
								break;

							case 0x02:
								//����ON
								WritePcMotorDecBit(&regs,true);
								break;

							case 0x03:
								//����OFF
								WritePcMotorAccBit(&regs,false);
								//����OFF
								WritePcMotorDecBit(&regs,false);
								break;
						}

						//Ӧ��״̬����
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"����CAN����ʧ��");
							}
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷ���������Ӧ�𣺶���
		////////////////////////////////////////////////////////////////////////////
		{
			//��ͷ״̬
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), ZEROCMD, 0x00,0x01,cmdlist);

				//���������
				if (count >= 1)
				{
					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("��ͷ") + QString::number(i+1),
						"��������","�յ���ͷ���������",1);

					//ԭ�����ж��㶯���������ⲿ��ͣ����
					if ((pubcom.actionzerostep.value(i) != 0) || (publiccaution.hascaution(PLCLOCATION, PLCHALTCATION) == true))
					{
						//Ӧ������
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"����CAN����ʧ��");

							}
						}
					}
					else
					{
						//���ñ�ʶλ
						pubcom.actionzerostep.insert(i, 1);

						//������Ϣ����ͷ�壬ͬһʱ��ֻ��һ����������Ч�ģ����������Ͱ��ճɹ�ִ�з���

						//Ӧ��״̬����
						tempcmd.cmd = ZEROCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;

						//�������
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"����CAN����ʧ��");

							}
						}
					}
				}
			}	
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷ���������Ӧ�𣺷�ת
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), RESERVECMD, 0x00, cmdlist);

				//���������
				if (count >= 1)
				{

					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("��ͷ") + QString::number(i+1),
						"��ת����","�յ���ͷ�巴ת����",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//�Ƿ�������ִ�� 1���� 2�ܾ�
						char anscmd = 1;

						//����ִ�У���ת
						if ((*j).para2 == 0x01)
						{
							//�ж��Ƿ��ڸ���״̬������״̬������ת
							int motorstatus = -1;
							ksmc->GetStatus(i+1,motorstatus);
							//���ڸ���״̬�ſ���,����û���ⲿ��ͣ����
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
						//ֹͣת��
						else if ((*j).para2 == 0x02)
						{
							ksmc->MoveStop((i) + 1);
							pubcom.circlestatus[i] = 0;

							//������ͷλ�ûָ�����
							if(pubcom.restoreTask[i].step == 0)
								pubcom.restoreTask[i].step = 1;
							
                            //ksmc->RestorePrintLocation(i + 1);
						}

						//Ӧ������
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"����CAN����ʧ��");
							}
						}
					}
				}
			}
		}

		//�ָ�λ����������
		for(int iiii = 0;iiii < pubcom.PrintNum;iiii++)
		{
			restoreTaskRun(&(pubcom.restoreTask[iiii]));
		}


		////////////////////////////////////////////////////////////////////////////
		//��ͷ���������Ӧ������Ի�
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), YMOVECMD, 0x00, cmdlist);

				//���������
				if (count >= 1)
				{
					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("��ͷ") + QString::number(i+1),
						"����Ի�����","�յ���ͷ������Ի�����",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//����ִ��,��������Ի�,����ԭ����״̬ȷ���Ƿ�Ҫִ��ָ��
						if ((*j).para2 == 0x01 && 
							pubcom.movedirectstatus.value(i) == 0)
						{
							ksmc->MoveDirect((*j).id + 1, 1);
							printf("%d����������Ի�\n",(*j).id + 1);
							pubcom.movedirectstatus.insert(i,1);
							pubcom.movedirecttimer.insert(i,pubtimermng.addtimer());
						}
						//������Ի�,����ԭ����״̬ȷ���Ƿ�Ҫִ��ָ��
						else if ((*j).para2 == 0x02 && 
							pubcom.movedirectstatus.value(i) == 0)
						{
							ksmc->MoveDirect((*j).id + 1,0);
							printf("%d��������Ի�\n",(*j).id + 1);
							pubcom.movedirectstatus.insert(i,2);
							pubcom.movedirecttimer.insert(i,pubtimermng.addtimer());
						}//ֹͣת��,����ԭ����״̬ȷ���Ƿ�Ҫִ��ָ��
						else if ((*j).para2 == 0x03)
						{
							ksmc->MoveStop((*j).id + 1);
							printf("%d��ֹͣ�Ի�\n",(*j).id + 1);
							pubcom.movedirectstatus.insert(i,0);
							pubtimermng.deltimer(pubcom.movedirecttimer.value(i));
							
							//�Ƿ��Ѿ��������״̬
							int motorstatus = -1;
							ksmc->GetStatus((*j).id + 1,motorstatus);
							//���û�н������״̬�����
							if(motorstatus != 2)
							{
								//��������λ������
								if(pubcom.savelocTask[(*j).id].step == 0)
								{
									pubcom.savelocTask[(*j).id].step = 1;
								}
								//ksmc->RememberPrintLoction((*j).id + 1);							
							}						
						}

						//Ӧ������
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"����CAN����ʧ��");
							}
						}

					}
				}
			}
		}

		{

			//����Ի����ٵ�����
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
		//��ͷ״̬�ϱ�Ӧ��
		////////////////////////////////////////////////////////////////////////////
		{
			//��ͷ״̬
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTWORKSTATUSCMD, 0x00,
					cmdlist);

				//���������
				if (count >= 1)
				{

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//�����
						pubcom.printstatus[i] = (*j).para2;

						//Ӧ��״̬����
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(i),
									SENDCMDFAILEVENT,"����CAN����ʧ��");
							}
						}
					}
				}
			}	
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷ������ɣ������������������
		////////////////////////////////////////////////////////////////////////////
		{
			//��ͷ״̬
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				tcmd tempcmd;
				candevice.getcmd((i), ZEROCMD,0x00,0x02,
					&tempcmd);
			}	
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷIO״̬�ϱ�Ӧ��
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTSWITCHSTATUSCMD, 0x00,
					cmdlist);

				//���������
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//�����
						pubcom.lockreleasestatus[i] = ((*j).para2 >> 0) & 1;//��/����

						//Ҫע�ⰲȫ��������ͷ�ͱ���������״̬
						pubcom.updownstatus[i] = ((*j).para2 >> 1) & 1;//��ͷ��/��
						pubcom.platupdownstatus[i] = ((*j).para2 >> 2) & 1;//����
						pubcom.colorpipestatus[i] = ((*j).para2 >> 3) & 1;//�ϸ���/��

						if(((((*j).para2 >> 4) & 1) == 1) &&  (pubcom.magicstatus[i] == 0))
						{
							pubcom.MagicOnFlushStep[i] = 1;
						}

						pubcom.magicstatus[i] = ((*j).para2 >> 4) & 1;//��̨��/��
						pubcom.bumpstatus[i] = ((*j).para2 >> 5) & 3;//���ÿ�/��
						pubcom.zerostatus[i] = ((*j).para2 >> 7) & 1;//��/�����

						//Ӧ��״̬����
						tempcmd.cmd = PRINTSWITCHSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0x00;
						tempcmd.para2 = (*j).para2;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷ����λ��״̬�ϱ�Ӧ��
		////////////////////////////////////////////////////////////////////////////
		{
			//��ͷ״̬
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTXSTATUSCMD, cmdlist);

				//���������
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//�����
						pubcom.xstatus[i] = (((short) ((*j).para1)) << 8) + (*j).para2;

						//���浽���ݿ���
						mydb.SaveValue(QString("x") + QString::number(i),
							pubcom.xstatus[i]);

						//Ӧ��״̬����
						tempcmd.cmd = PRINTXSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = 0x01;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
						}
					}
				}
			}	
		}


		////////////////////////////////////////////////////////////////////////////
		//��ͷб��λ��״̬�ϱ�Ӧ��
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTYSTATUSCMD, cmdlist);

				//���������
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//�����
						pubcom.ystatus[i] = (((short) ((*j).para1)) << 8) + (*j).para2;

						//���浽���ݿ���
						mydb.SaveValue(QString("y") + QString::number(i),
							pubcom.ystatus[i]);

						//Ӧ��״̬����
						tempcmd.cmd = PRINTYSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = 0x01;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷ�߶�λ��״̬�ϱ�Ӧ��
		////////////////////////////////////////////////////////////////////////////
		{
			//��ͷ״̬
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), PRINTZSTATUSCMD, cmdlist);

				//���������
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//�����
						pubcom.zstatus[i] = (((short) ((*j).para1)) << 8) + (*j).para2;

						//���浽���ݿ���
						mydb.SaveValue(QString("z") + QString::number(i),
							pubcom.zstatus[i]);

						//Ӧ��״̬����
						tempcmd.cmd = PRINTZSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = 0x01;
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//�����ٶȹ㲥
		////////////////////////////////////////////////////////////////////////////
		{
			if(pubcom.MotorSpeedChgFlag == true)
			{
				pubcom.MotorSpeedChgFlag = false;

				//Ӧ��״̬����
				tempcmd.cmd = QUERYSPEEDCMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0;
				tempcmd.para2 = (unsigned char)pubcom.motorspeed; //�������ٶ�
				if (candevice.sendcmd(ALLPRINTID,
					&tempcmd) == false)
				{
					//�����¼��������û�ע��
					publiccaution.addevent(
						"�㲥����",
						"�ٶȹ㲥����","����CAN����ʧ��");
				}

			}
		}

		////////////////////////////////////////////////////////////////////////////
		//���ػ�״̬������ѯ�������״̬
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), QUERYMACHSTATUSCMD, 0x00,
					0x01, cmdlist);

				//���������
				if (count >= 1)
				{
					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("��ͷ") + QString::number(i+1),
						"����״̬��ѯ","�յ���ͷ������״̬��ѯ����",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//Ӧ��״̬����
						tempcmd.cmd = QUERYMACHSTATUSCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = i;
						tempcmd.para2 = pubcom.machprintstatus; //������״̬
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
						}
					}
				}
			}

		}


		////////////////////////////////////////////////////////////////////////////
		//���ػ�״̬������ѯ���������λλ�ò���
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), QUERYXSTATUSCMD, 0x00, 0x01,
					cmdlist);

				//���������
				if (count >= 1)
				{
					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("��ͷ") + QString::number(i+1),
						"������λλ�ò���","�յ���ͷ�������λλ�ò�������",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//Ӧ��״̬����
						tempcmd.cmd = QUERYXSTATUSCMD;
						tempcmd.id = PCCANBDID | i;//�Ƚ�����
						tempcmd.para1 = pubcom.xstatus[i] & 0xff;//�������λλ��
						tempcmd.para2 = (pubcom.xstatus[i] >> 8) & 0xff; //�������λλ��
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//���ػ�״̬������ѯ�����б����λλ�ò���
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), QUERYYSTATUSCMD, 0x00, 0x01,
					cmdlist);

				//���������
				if (count >= 1)
				{
					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("��ͷ") + QString::number(i+1),
						"��б����λλ��","�յ���ͷ���б����λλ������",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//Ӧ��״̬����
						tempcmd.cmd = QUERYYSTATUSCMD;
						tempcmd.id = PCCANBDID | i;//�Ƚ�����
						tempcmd.para1 = pubcom.ystatus[i] & 0xff;//��б����λλ��
						tempcmd.para2 = (pubcom.ystatus[i] >> 8) & 0xff; //��б����λλ��
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//���ػ�״̬������ѯ������߶���λλ�ò���
		////////////////////////////////////////////////////////////////////////////
		{
			for (int i = PRINT1BDID; i <= PRINT16BDID; i++)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((i), QUERYZSTATUSCMD, 0x00, 0x01,
					cmdlist);

				//���������
				if (count >= 1)
				{
					//�����¼��������û�ע��
					publiccaution.addevent(
						QString("��ͷ") + QString::number(i+1),
						"�߶���λλ��","�յ���ͷ��߶���λλ������",1);

					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//Ӧ��״̬����
						tempcmd.cmd = QUERYZSTATUSCMD;
						tempcmd.id = PCCANBDID | i;//�Ƚ�����
						tempcmd.para1 = pubcom.zstatus[i] & 0xff;//��б����λλ��
						tempcmd.para2 = (pubcom.zstatus[i] >> 8) & 0xff; //��б����λλ��
						if (candevice.sendcmd(pubcom.bdnodemap.value(i),
							&tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(i),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
						}
					}
				}
			}
		}




		////////////////////////////////////////////////////////////////////////////
		//PLC�����ź�
		////////////////////////////////////////////////////////////////////////////
		{

			//�����ز���
			if ( pubcom.PlcCauUpFlag == true  )
			{
				pubcom.PlcCauUpFlag = false;
				publiccaution.addcaution(PLCLOCATION, PLCCAUTION);

			}

			//�½�������
			if ( pubcom.PlcCauDownFlag == true )
			{
				pubcom.PlcCauDownFlag = false;
				publiccaution.delcaution(PLCLOCATION, PLCCAUTION);
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//PLC�����ź�
		////////////////////////////////////////////////////////////////////////////
		{
			//�����ز���
			if ( pubcom.PlcErrUpFlag == true )
			{
				pubcom.PlcErrUpFlag = false;
				publiccaution.addcaution(PLCLOCATION, PLCHALTCATION);

			}

			//�½�������
			if ( pubcom.PlcErrDownFlag == true )
			{
				pubcom.PlcErrDownFlag = false;
				publiccaution.delcaution(PLCLOCATION, PLCHALTCATION);
			}

		}


		///////////////////////////////////////////////////////////////////////////
		//���̴���//////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////

		//��������������ͷ���Ƿ񼤻�ò��ԣ������Ƿ��ǹ��Ͼͺ��Ƿ񼤻��й�ϵ�� 
		//����Ĳ���Ϊ���ϣ�û�м���Ĳ���
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

			//��ʼ����������������ʧ�ܣ���Ϊ��CAN�������⣬����ʧ�ܼ���
			if (candevice.sendcmd(ALLID, &tempcmd) == false)
			{
				//�����¼��������û�ע��
				publiccaution.addevent(
					"�㲥����",
					"�㲥�����а���","����CAN����ʧ��");

				pubcom.networkfailcount++;

				//�˳���������
				pubcom.heartbeatstep = 4;
			}
			else
			{
				if(publiccaution.hascaution(CANCAUTIONLOCATION,CANNETCAUTION))
				{
					publiccaution.addcaution(CANCAUTIONLOCATION, CANNETCAUTION);
				}
				pubcom.networkfailcount = 0; //���������λ
				pubcom.heartbeatstep = 3;

				//�ڵ��ʼ��
				QList<int>::iterator i;
				for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
				{
					heartbeatmap.insert(*i, false);
				}
			}
		}
		//��������
		else if (pubcom.heartbeatstep == 3)
		{
			//�ȴ�ʱ����
			if (pubtimermng.gettimerval(pubcom.heartbeattimer) <= OUTTIME)
			{
				//�Ƿ�ȫ���յ�����
				bool isallget = true;

				for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
				{
					if (heartbeatmap.value(*i) == false)
					{
						isallget = false;
						break;
					}
				}

				//ȫ���յ�����һ��
				if (isallget == true)
				{
					pubcom.heartbeatstep = 4;
				}
				//���������
				else
				{
					tcmd tempcmd;
					tempcmd.cmd = COMTESTCMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = 0;
					tempcmd.para2 = 0;

					//��������
					QList<int>::iterator i;
					for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
					{
						QList<tcmd> cmdlist;
						int count = candevice.getcmd((*i), COMTESTCMD,
							cmdlist);

						//�ظ�ID
						if (count > 1)
						{
							pubcom.nodedupidmap.insert(*i,pubcom.nodedupidmap.value(*i)+1);

							//��0
							pubcom.nodeonlinecountmap.insert(*i, 0);

							heartbeatmap.insert(*i, true);

							//�����¼�
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

							//�ظ�ID��������
							if (publiccaution.hascaution(pubcom.bdcautionmap.value((*i)), REPIDCAUTION))
							{
								publiccaution.delcaution(pubcom.bdcautionmap.value((*i)), REPIDCAUTION);
							}

							//��0
							pubcom.nodeonlinecountmap.insert(*i, 0);

							heartbeatmap.insert(*i, true);

							//�����¼�
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
			else //�ȴ���ʱ
			{
				QList<int>::iterator i;
				for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
				{
					//��ʱû���յ���Ϣ��������1
					if (heartbeatmap.value(*i) == false)
					{
						pubcom.nodeonlinecountmap.insert(*i,
							pubcom.nodeonlinecountmap.value(*i) + 1);
					}
				}

				pubcom.heartbeatstep = 4;

				//�����¼��������û�ע��
				publiccaution.addevent(
					"CANͨѶ",
					"�������Գ�ʱ","CANͨѶ�������Գ�ʱ");
			}
		}
		else if (pubcom.heartbeatstep == 4)//10��ʱ��ſ��Լ�����һ�β��ԣ�����̫Ƶ���ᵼ������籩
		{
			//ʱ����
			if (pubtimermng.gettimerval(pubcom.heartbeattimer) > HEARTBEATTIME)
			{
				pubtimermng.deltimer(pubcom.heartbeattimer);
				pubcom.heartbeatstep = 1;

				//��������������Ҫ�ȴ���ʹ����ƽ��������������
				if(pubcom.heartbeatbegincnt <= 3)
				{
					pubcom.heartbeatbegincnt ++;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//������ϣ����緢��ʧ�ܼ������������3����Ϊ����ʧ�ܣ���������ʧ���¼� 
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
		//�ڵ�ͨ�Ź��ϣ�ʧ��7�ξ������¼�
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

					//�ظ�ID���ϲ���
					publiccaution.addcaution(pubcom.bdcautionmap.value((*i)), REPIDCAUTION);

				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//����ˢ������
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.FlushDataReqStep == 1)
		{
			WritePcFlushDataReqBit(&regs,true);
			pubcom.FlushDataReqTimer = pubtimermng.addtimer();
			pubcom.FlushDataReqStep = 2; 
		}
		else if (pubcom.FlushDataReqStep == 2)
		{
			//ά��ʱ��
			if (pubtimermng.gettimerval(pubcom.FlushDataReqTimer) > MODBUSBITHOLDTIME)
			{
				WritePcFlushDataReqBit(&regs,false);
				pubtimermng.deltimer(pubcom.FlushDataReqTimer);
				pubcom.FlushDataReqStep = 0; 
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//������������
		////////////////////////////////////////////////////////////////////////////

		if (pubcom.plcreqstartstep == 1)
		{

			QList<int>::iterator it;
			int workcnt = 0;

			//���̱߳���
			pubcom.activeprintlistmutex.lockForRead();

			//��ʼ�����������ͷ
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{

				//�й���״̬��
				if (pubcom.presetprintstatus[*it] == 3)
				{
					workcnt++;
					break;
				}
			}
			pubcom.activeprintlistmutex.unlock();

			//���û�м������ͷ���߹�����ͷ����Ϊ0
			if ((pubcom.activeprintlist.count() == 0) || (workcnt == 0))
			{
				//��
				pubcom.plcreqstartstep = 10;
				pubcom.printok = true;
			}
			else
			{
				pubcom.plcreqstartstep = 2;
			}
		}
		//��ʼ��
		else if (pubcom.plcreqstartstep == 2)
		{
			//�����ͷ
			pubcom.printcmdansmap.clear();
			QList<int>::iterator it;

			//���̱߳���
			pubcom.activeprintlistmutex.lockForRead();

			//��ʼ�����������ͷ
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{

				//������׼��״̬�����������
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
		else if (pubcom.plcreqstartstep == 3)//����CAN���������ͷ
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

				//ѯ����ͷ
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					if (pubcom.presetprintstatus[*it] != 4)
					{
						//û�гɹ���������
						if (pubcom.sendprintcmdmap.value(*it) == false)
						{
							//��������
							if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
								&tempcmd) == false)
							{
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"����CAN����ʧ��");

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

				if (sendonesucceed == true)//����һ�����ͳɹ�,��Ҫ������һ�����������
				{
					pubcom.plcreqstartstep = 4;
					pubcom.plcreqstarttimer = pubtimermng.addtimer();
				}
				else//ȫ�����ʹ���
				{
					//do nothing
				}
			}
		}
		else if (pubcom.plcreqstartstep == 4)//������ͷ�����
		{
			tcmd tempcmd;
			tempcmd.cmd = STARTWORKCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = 0x30;
			tempcmd.para2 = 0x01;

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//�ȷ���
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				if (pubcom.presetprintstatus[*it] != 4)
				{

					//�ɹ�����������
					if (pubcom.sendprintcmdmap.value(*it) == true
						&& pubcom.printcmdansmap.value(*it) == false)
					{
						//��������
						if (candevice.getcmd((*it), STARTWORKCMD, &tempcmd) == true)
						{
							//�Ƿ�׼����
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

			//ʱ����
			if (pubtimermng.gettimerval(pubcom.plcreqstarttimer) <= OUTTIME)
			{
				//�Ƿ�ȫ���յ�
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

					//����ʧ�ܱ�ʶ
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
						pubcom.plcreqstartstep = 5; //û��ʧ�ܵģ���ô������һ��
					}
					else
					{
						pubcom.plcreqstartstep = 3; //�ط�

						for (it = pubcom.printcmdansmap.begin(); it
							!= pubcom.printcmdansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
								pubcom.sendprintcmdmap.insert(it.key(), false);
							}
						}
					}
				}
				else //û��ȫ�����յ���������
				{
					//do nothing
				}
			}
			else //��ʱ
			{
				QMap<int, bool>::iterator it;

				pubtimermng.deltimer(pubcom.plcreqstarttimer);

				//����ʧ�ܱ�ʶ
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
							//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
							pubcom.sendprintcmdmap.insert(it.key(), false);
						}
					}
				}
			}
		}
		else if (pubcom.plcreqstartstep == 5)//��ʼ��
		{
			//�����ͷ
			pubcom.printcmdansmap.clear();
			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//��ʼ��
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
			//��ͷ�Ƿ�ȫ��׼����
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
				pubcom.plcreqstartstep = 10;//��ͷû��׼��,ֹͣ���������
				pubcom.printok = false;
			}
			else//��ͷ����׼������
			{
				pubcom.plcreqstartstep = 7;
			}
		}
		else if (pubcom.plcreqstartstep == 7)//��ͷ״̬�л���ʼ��
		{
			//�����ͷ
			pubcom.printcmdansmap.clear();

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//��ʼ��
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
		else if (pubcom.plcreqstartstep == 8)//״̬�л������
		{
			bool sendonesucceed = false;

			tcmd tempcmd;

			QList<int>::iterator it;
			if (pubcom.plcreqstartcount > 1)
			{
				pubcom.printok = false;
				pubcom.plcreqstartstep = 10;//��ͷû��׼��,ֹͣ���������
			}
			else
			{
				pubcom.plcreqstartcount++;

				tempcmd.cmd = SETMACHSTATUSCMD;//״̬�л�����
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x00;
				tempcmd.para2 = 0x05; //ӡ������״̬

				pubcom.activeprintlistmutex.lockForRead();

				//��ͷ״̬�л�
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					if (pubcom.presetprintstatus[*it] == 3)
					{

						//û�гɹ���������
						if (pubcom.sendprintcmdmap.value(*it) == false)
						{
							tempcmd.para1 = *it;

							//��������
							if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
								&tempcmd) == false)
							{
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"����CAN����ʧ��");

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
		else if (pubcom.plcreqstartstep == 9)//״̬�����
		{
			tcmd tempcmd;
			tempcmd.cmd = SETMACHSTATUSCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = 0x00;
			tempcmd.para2 = 0x01;

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//�ȷ���
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				if (pubcom.presetprintstatus[*it] != 4)
				{

					//�ɹ�����������
					if (pubcom.sendprintcmdmap.value(*it) == true
						&& pubcom.printcmdansmap.value(*it) == false)
					{
						//��������
						if (candevice.getcmd((*it), SETMACHSTATUSCMD, &tempcmd)
							== true)
						{
							//��ȷ�Ľ�����״̬ת��
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

			//ʱ����
			if (pubtimermng.gettimerval(pubcom.plcreqstarttimer) <= OUTTIME)
			{
				//�Ƿ�ȫ���յ�
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

					//����ʧ�ܱ�ʶ
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
						pubcom.plcreqstartstep = 8; //��������

						for (it = pubcom.printcmdansmap.begin(); it
							!= pubcom.printcmdansmap.end(); ++it)
						{
							if (it.value() == false)
							{
								//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
								pubcom.sendprintcmdmap.insert(it.key(), false);
							}
						}
					}
					else
					{
						//��ͷ�Ƿ�׼����
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


						pubcom.plcreqstartstep = 10; //��һ��
					}

				}
				else
				{
					//do nothing
				}
			}
			else //��ʱ
			{
				pubtimermng.deltimer(pubcom.plcreqstarttimer);

				//����ʧ�ܱ�ʶ
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
							//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
							pubcom.sendprintcmdmap.insert(it.key(), false);
						}
					}
				}
				else
				{
					//��ͷ�Ƿ�׼����
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
		else if (pubcom.plcreqstartstep == 10)//����״̬�ı��ʼ��
		{
			//�����Ƿ��������
			if (pubcom.printok == true)
			{
				//��������״̬
				pubcom.machprintstatus = 2;

				QList<int>::iterator it;

				QDomNode para;
				int paraVal1 = 0;
				int paraVal2 = 0;
				int paraVal3 = 0;
				int paraVal4 = 0;
				bool ok;

				para = GetParaByName("driver", "��һλ������");
				paraVal1 = para.firstChildElement("value").text().toInt(&ok);

				para = GetParaByName("driver", "��һ�ٶ�����");
				paraVal2 = para.firstChildElement("value").text().toInt(&ok);


				para = GetParaByName("driver", "��һ�ٶȻ���ʱ�䳣��");
				paraVal3 = para.firstChildElement("value").text().toInt(&ok);

				para = GetParaByName("driver", "��һ�ٶȼ�����");
				paraVal4 = para.firstChildElement("value").text().toInt(&ok);

				pubcom.activeprintlistmutex.lockForRead();

				//����
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					if (pubcom.presetprintstatus[*it] == 3)
					{
						//�Ƿ��Ѿ��������״̬
						int motorstatus = -1;
						ksmc->GetStatus(*it +1,motorstatus);
						//���û�н������״̬��������״̬
						if(motorstatus != 2)
						{
							//printf("�������%d\n",*it + 1);

							//��������
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

				publiccaution.adddbginfo(QString("����ӡ��"));
				pubcom.plcreqstarttimer = pubtimermng.addtimer();
				pubcom.plcreqstartstep = 11;
			}
			else
			{
				Dbgout("״̬��%d",pubcom.machprintstatus);
				publiccaution.adddbginfo(QString("����ӡ��"));
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


		//������������̴�������,��ת,��ͷ�·�/����,��̨�����ر�
		if (pubcom.ismainuicmd == true)
		{
			bool sendonesucceed = false;

			if (pubcom.mainuicmdstep == 1)//��ʼ��
			{
				//����
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//ֻ����ͷ����׼��״̬�ŷ�����
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
			else if (pubcom.mainuicmdstep == 2)//����
			{
				if (pubcom.mainuicmdcount > 1)
				{
					//����������
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x02;
					ptempevent->status = 0x03;

					ptempevent->data[0] = pubcom.mainuidcmd;

					//����һ���¼��������߳�
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

					//ֻ�м���Ĳŷ�������
					for (it = pubcom.activeprintlist.begin(); it
						!= pubcom.activeprintlist.end(); ++it)
					{
						//ֻ����ͷ����׼��״̬�ŷ�����
						if (pubcom.presetprintstatus[*it] != 4)
						{
							if (pubcom.senduicmdtoprintmap.value(*it) == false)
							{
								tcmd tempcmd;

								//����Ҫ��������
								if (pubcom.mainuidcmd == 0x53)
								{
									tempcmd.cmd = REQZEROCMD;//��ͷ��������
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = 0x01;
								}
								//��תҪ��������
								else if (pubcom.mainuidcmd == 0x54)
								{
									tempcmd.cmd = REQCIRCLECMD;//��ͷ��������
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = 0x01;
								}
								else
								{
									tempcmd.cmd = PRINTOPCMD;//��ͷ��������
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = pubcom.mainuidcmd;//С����
								}

								if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
									&tempcmd) == false)
								{
									msleep(1);
									if (candevice.sendcmd(pubcom.bdnodemap.value(
										*it), &tempcmd) == false)
									{
										//�����¼��������û�ע��
										publiccaution.addevent(
											pubcom.bdcautionmap.value(*it),
											SENDCMDFAILEVENT,"����CAN����ʧ��");
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

					//ֻҪ��һ�����ͳɹ������������ڶ���
					if (sendonesucceed == true)
					{
						pubcom.mainuicmdstep = 3;
						pubcom.mainuidcmdtimer = pubtimermng.addtimer();
					}
					else//û��һ�����ͳɹ�
					{
						//do nothing
					}
				}
			}
			else if (pubcom.mainuicmdstep == 3)//����
			{
				bool allget = true;
				//����
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//ֻ����ͷ����׼��״̬�ŷ�����
					if (pubcom.presetprintstatus[*it] != 4)
					{
						tcmd tempcmd;

						if (pubcom.senduicmdtoprintmap.value(*it) == true
							&& pubcom.uicmdtoprintansmap.value(*it) == false)
						{
							//����
							if (pubcom.mainuidcmd == 0x53)
							{
								if (candevice.getcmd((*it), REQZEROCMD, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//��ת
							else if (pubcom.mainuidcmd == 0x54)
							{
								if (candevice.getcmd((*it), REQCIRCLECMD, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							else
							{
								if (candevice.getcmd((*it), PRINTOPCMD, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
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
									//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
									pubcom.senduicmdtoprintmap.insert(it.key(),
										false);
								}
							}
						}
						else
						{
							//����������
							backendevent* ptempevent = new backendevent();
							ptempevent->cmd = 0x02;
							ptempevent->status = 0x01;
							ptempevent->data[0] = pubcom.mainuidcmd;

							//����һ���¼��������߳�
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
					//����ʧ�ܱ�ʶ
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
								//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
								pubcom.senduicmdtoprintmap.insert(it.key(),
									false);
							}
						}
					}
					else
					{
						//����������
						backendevent* ptempevent = new backendevent();
						ptempevent->cmd = 0x02;
						ptempevent->status = 0x01;
						ptempevent->data[0] = pubcom.mainuidcmd;

						//����һ���¼��������߳�
						QCoreApplication::postEvent(interfaceobject, ptempevent);
						pubcom.ismainuicmd = false;
						pubcom.mainuicmdstep = 0;
					}
				}
			}
		}


		//PLC������̴�������,��ת,��ͷ�·�/����,��̨�����ر�
		if (pubcom.isplccmd == true)
		{
			bool sendonesucceed = false;

			if (pubcom.plccmdstep == 1)//��ʼ��
			{
				//����
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//ֻ����ͷ����׼��״̬�ŷ�����
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
			else if (pubcom.plccmdstep == 2)//����
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

					//ֻ�м���Ĳŷ�������
					for (it = pubcom.activeprintlist.begin(); it
						!= pubcom.activeprintlist.end(); ++it)
					{
						//ֻ����ͷ����׼��״̬�ŷ�����
						if (pubcom.presetprintstatus[*it] != 4)
						{
							if (pubcom.senduicmdtoprintmap.value(*it) == false)
							{
								tcmd tempcmd;

								//����Ҫ��������
								if (pubcom.plcdcmd == 0x53)
								{
									tempcmd.cmd = REQZEROCMD;//��ͷ��������
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = 0x01;
								}
								//��תҪ��������
								else if (pubcom.plcdcmd == 0x54)
								{
									tempcmd.cmd = REQCIRCLECMD;//��ͷ��������
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = 0x01;
								}
								//������С�ı�Ҫ��������
								else if (pubcom.plcdcmd == 0x58)
								{
									tempcmd.cmd = CHGMAGICCMD;//��ͷ��������
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = regs.readarray[3];//������С
								}
								//�����Զ�ģʽ����
								else if (pubcom.plcdcmd == 0x59)
								{
									tempcmd.cmd = CHGMAGICMODE;//�޸Ĵ���ģʽ
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = 1;//�Զ�����
								}
								//�����Զ�ģʽ�ر�
								else if (pubcom.plcdcmd == 0x60)
								{
									tempcmd.cmd = CHGMAGICMODE;//�޸Ĵ���ģʽ
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = 0;//�Զ��ر�
								}
								else
								{
									tempcmd.cmd = PRINTOPCMD;//��ͷ��������
									tempcmd.id = PCCANBDID;//can��
									tempcmd.para1 = *it;//Ŀ����ͷ����
									tempcmd.para2 = pubcom.plcdcmd;//С����
								}

								if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
									&tempcmd) == false)
								{
									msleep(1);
									if (candevice.sendcmd(pubcom.bdnodemap.value(
										*it), &tempcmd) == false)
									{
										//�����¼��������û�ע��
										publiccaution.addevent(
											pubcom.bdcautionmap.value(*it),
											SENDCMDFAILEVENT,"����CAN����ʧ��");
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

					//ֻҪ��һ�����ͳɹ������������ڶ���
					if (sendonesucceed == true)
					{
						pubcom.plccmdstep = 3;
						pubcom.plcdcmdtimer = pubtimermng.addtimer();
					}
					else//û��һ�����ͳɹ�
					{
						//do nothing
					}
				}
			}
			else if (pubcom.plccmdstep == 3)//����
			{
				bool allget = true;
				//����
				QList<int>::iterator it;
				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//ֻ����ͷ����׼��״̬�ŷ�����
					if (pubcom.presetprintstatus[*it] != 4)
					{
						tcmd tempcmd;

						if (pubcom.senduicmdtoprintmap.value(*it) == true
							&& pubcom.uicmdtoprintansmap.value(*it) == false)
						{
							//����
							if (pubcom.plcdcmd == 0x53)
							{
								if (candevice.getcmd((*it), REQZEROCMD, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//��ת
							else if (pubcom.plcdcmd == 0x54)
							{
								if (candevice.getcmd((*it), REQCIRCLECMD, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//������С�޸�
							else if (pubcom.plcdcmd == 0x58)
							{
								if (candevice.getcmd((*it), CHGMAGICCMD, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//����ģʽ�޸�
							else if (pubcom.plcdcmd == 0x59)
							{
								if (candevice.getcmd((*it), CHGMAGICMODE, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							//����ģʽ�޸�
							else if (pubcom.plcdcmd == 0x60)
							{
								if (candevice.getcmd((*it), CHGMAGICMODE, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
									pubcom.uicmdtoprintansmap.insert(*it, true);
								}
							}
							else
							{
								if (candevice.getcmd((*it), PRINTOPCMD, &tempcmd)
									== true)
								{
									//�ɹ��յ�����Ӧ��
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
									//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
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
					//����ʧ�ܱ�ʶ
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
								//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
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

		//��̨�ϴ�ˢ�³�ʼֵ
		if(pubcom.isplccmd == false)
		{
			for(int ii = 0;ii <16;ii++ )
			{
				//����
				if(pubcom.MagicOnFlushStep[ii] == 1)
				{
					int magicval;
					mydb.GetValue(QString("magic") + QString::number(ii) ,magicval);

					//��ֹ����Ϊ0
					if(magicval == 0)
						magicval = 1;

					tcmd tempcmd;
					tempcmd.cmd = CHGMAGICCMD;//��ͷ��������
					tempcmd.id = PCCANBDID;//can��
					tempcmd.para1 = ii;//Ŀ����ͷ����
					tempcmd.para2 = magicval;//������С

					if (candevice.sendcmd(pubcom.bdnodemap.value(ii),
						&tempcmd) == false)
					{
						msleep(1);
						if (candevice.sendcmd(pubcom.bdnodemap.value(
							ii), &tempcmd) == false)
						{
							//����ֹͣ
							pubcom.MagicOnFlushStep[ii] = 0;

							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(ii),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
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
				}//����
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


		//��ͷ������������
		if (pubcom.opuicmdflag == true)
		{
			//��ʼ��
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

					//������Ϣ������
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x03;
					ptempevent->status = 0x02;
					ptempevent->data[0] = pubcom.opuicmdcmd;
					ptempevent->data[1] = pubcom.opuicmdprintno;

					//����һ���¼��������߳�
					QCoreApplication::postEvent(opobject, ptempevent);

				}
				else
				{
					pubcom.opuicmdcount++;
					tcmd tempcmd;

					//�����������⴦����ʽ��һ��
					if (pubcom.opuicmdcmd == REQZEROCMD)
					{
						tempcmd.cmd = REQZEROCMD;//��ͷ��������
						tempcmd.id = PCCANBDID;//can��
						tempcmd.para1 = pubcom.opuicmdprintno;//Ŀ����ͷ����
						tempcmd.para2 = 0x01;//С����
					}
					else if (pubcom.opuicmdcmd == REQCIRCLECMD)
					{
						tempcmd.cmd = REQCIRCLECMD;//��ͷ��������
						tempcmd.id = PCCANBDID;//can��
						tempcmd.para1 = pubcom.opuicmdprintno;//Ŀ����ͷ����
						tempcmd.para2 = 0x01;//С����
					}
					else
					{
						tempcmd.cmd = PRINTOPCMD;//��ͷ��������
						tempcmd.id = PCCANBDID;//can��
						tempcmd.para1 = pubcom.opuicmdprintno;//Ŀ����ͷ����
						tempcmd.para2 = pubcom.opuicmdcmd;//С����
					}

					if (candevice.sendcmd(
						pubcom.bdnodemap.value(tempcmd.para1), &tempcmd)
						== false)
					{
						msleep(1);
						if (candevice.sendcmd(pubcom.bdnodemap.value(
							tempcmd.para1), &tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(tempcmd.para1),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
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
			else if (pubcom.opuicmdstep == 3)//����
			{
				bool recvsucceed = false;
				tcmd tempcmd;

				//�����������⴦��
				if (pubcom.opuicmdcmd == REQZEROCMD)
				{
					if (candevice.getcmd(pubcom.opuicmdprintno, REQZEROCMD,
						&tempcmd) == true)
					{
						//�ɹ��յ�����Ӧ��
						pubtimermng.deltimer(pubcom.opuicmdtimer);
						pubcom.opuicmdflag = false;
						pubcom.opuicmdstep = 0;

						//������Ϣ������
						backendevent* ptempevent = new backendevent();
						ptempevent->cmd = 0x03;
						ptempevent->status = 0x01;
						ptempevent->data[0] = pubcom.opuicmdcmd;
						ptempevent->data[1] = pubcom.opuicmdprintno;

						//����һ���¼��������߳�
						QCoreApplication::postEvent(opobject, ptempevent);

						recvsucceed = true;
					}
				}
				else
				{
					if (candevice.getcmd(pubcom.opuicmdprintno, PRINTOPCMD,
						&tempcmd) == true)
					{
						//�ɹ��յ�����Ӧ��
						pubtimermng.deltimer(pubcom.opuicmdtimer);
						pubcom.opuicmdflag = false;
						pubcom.opuicmdstep = 0;

						//������Ϣ������
						backendevent* ptempevent = new backendevent();
						ptempevent->cmd = 0x03;
						ptempevent->status = 0x01;
						ptempevent->data[0] = pubcom.opuicmdcmd;
						ptempevent->data[1] = pubcom.opuicmdprintno;

						//����һ���¼��������߳�
						QCoreApplication::postEvent(opobject, ptempevent);

						recvsucceed = true;
					}
				}

				if (pubtimermng.gettimerval(pubcom.opuicmdtimer) > OUTTIME)
				{
					//��ʱ
					pubtimermng.deltimer(pubcom.opuicmdtimer);

					if (recvsucceed == false)
					{
						pubcom.opuicmdstep = 2;
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷ״̬ѡ��
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.printactiveflag == true)
		{
			//��ʼ��
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

					//����������
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x07;//����Ӧ������
					ptempevent->status = 0x02;//����״̬��ʧ��
					ptempevent->data[0] = pubcom.printactivenum;

					//����һ���¼��������߳�
					QCoreApplication::postEvent(selectprintobject, ptempevent);
				}
				else
				{
					//��������
					tcmd tempcmd;
					pubcom.printactivecount++;

					tempcmd.cmd = SETMACHSTATUSCMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = pubcom.printactivenum;

					//׼��״̬
					tempcmd.para2 = pubcom.printactivecmd;

					if (candevice.sendcmd(pubcom.bdnodemap.value(
						pubcom.printactivenum), &tempcmd) == false)
					{
						msleep(1);
						if (candevice.sendcmd(pubcom.bdnodemap.value(
							pubcom.printactivenum), &tempcmd) == false)
						{
							//�����¼��������û�ע��
							publiccaution.addevent(
								pubcom.bdcautionmap.value(pubcom.printactivenum),
								SENDCMDFAILEVENT,"����CAN����ʧ��");
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

				//�����Ӧ��
				if (count >= 1)
				{
					pubtimermng.deltimer(pubcom.printactivetimer);
					pubcom.printactiveflag = false;
					pubcom.printactivestep = 0;

					//�����¼�
					//����������
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x07;//����Ӧ������
					ptempevent->status = 0x01;//����״̬����������
					ptempevent->data[0] = pubcom.printactivenum;
					ptempevent->data[1] = cmdlist[0].para1;

					//����һ���¼��������߳�
					QCoreApplication::postEvent(selectprintobject, ptempevent);
				}


				//��ʱ
				if (pubtimermng.gettimerval(pubcom.printactivetimer) > OUTTIME)
				{
					pubtimermng.deltimer(pubcom.printactivetimer);
					pubcom.printactivestep = 2;
				}
			}
		}


		////////////////////////////////////////////////////////////////////////////
		//�Ͻ���̨�����ı�
		////////////////////////////////////////////////////////////////////////////
		if((pubcom.PlcGongjiangcilidaxiaogaibianFlag == true)
			 && (pubcom.Shangjiangcitaistep == 0))
		{
			//����״̬�ŷ���
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

		//���������̨���ư�
		{
			//��ʼ��
			if (pubcom.Shangjiangcitaistep == 1)
			{
				pubcom.Shangjiangcitaicount = 0;
				pubcom.Shangjiangcitaitimer = 0;
				pubcom.Shangjiangcitaistep = 2;
			}
			//����
			else if (pubcom.Shangjiangcitaistep == 2)
			{
				if (pubcom.Shangjiangcitaicount > 1)
				{
					//����
					pubcom.Shangjiangcitaistep = 0;
				}
				else
				{
					pubcom.Shangjiangcitaicount++;

					//��������
					tcmd tempcmd;
					tempcmd.cmd = MAGICRSPCMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = pubcom.Shangjiangcitaibianhao - 1;

					//��̨�������ǹر�
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
						//�����¼��������û�ע��
						publiccaution.addevent(
							"��̨��",
							"���͸���̨��CAN����ʧ��","����CAN����ʧ��");
					}
					else
					{
						pubcom.Shangjiangcitaistep = 3;
						pubcom.Shangjiangcitaitimer = pubtimermng.addtimer();
					}
				}
			}//����
			else if (pubcom.Shangjiangcitaistep == 3)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd(MAGICIOBDID, MAGICRSPCMD, pubcom.Shangjiangcitaibianhao -1,
					cmdlist);

				//�����Ӧ��
				if (count >= 1)
				{
					pubtimermng.deltimer(pubcom.Shangjiangcitaitimer);
					pubcom.Shangjiangcitaistep = 0;
				}
				//��ʱ
				if (pubtimermng.gettimerval(pubcom.Shangjiangcitaitimer) > OUTTIME)
				{
					pubtimermng.deltimer(pubcom.Shangjiangcitaitimer);
					pubcom.Shangjiangcitaistep = 2;
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//��ͷ�������ò���
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.printchgspeedansflag == true)
		{
			//��ʼ��
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
			//����
			else if (pubcom.printchgspeedstep == 2)
			{
				if (pubcom.printchgspeedcount > 1)
				{
					//����
					pubcom.printchgspeedansflag = false;
					pubcom.printchgspeedstep = 0;

					//����������
					backendevent* ptempevent = new backendevent();
					ptempevent->cmd = 0x06;//����Ӧ������
					if (pubcom.printchgspeedsucceed == true)
					{
						ptempevent->status = 0x02;//����״̬
					}
					else
					{
						ptempevent->status = 0x03;//����״̬
					}

					//����һ���¼��������߳�
					QCoreApplication::postEvent(interfaceobject, ptempevent);
				}
				else
				{
					pubcom.printchgspeedcount++;

					//�㲥����
					tcmd tempcmd;
					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = 0x05;

					QDomNode paranode =
						GetParaByName("prod", QString("��ͷ��������"));
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
							//�����¼��������û�ע��
							publiccaution.addevent(
								"������ͷ��",
								"���͸�������ͷ��CAN����ʧ��","����CAN����ʧ��");
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
			}//����
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

					//�����Ӧ��
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
					bool allget = true;//�Ƿ�ȫ�����յ�
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

							//����ʧ��
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

							//����������
							backendevent* ptempevent = new backendevent();
							ptempevent->cmd = 0x06;//����Ӧ������
							ptempevent->status = 0x01;//����״̬,ȫ���ɹ�

							//����һ���¼��������߳�
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
						//����
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

						//����������
						backendevent* ptempevent = new backendevent();
						ptempevent->cmd = 0x06;//����Ӧ������
						ptempevent->status = 0x01;//����״̬,ȫ���ɹ�

						//����һ���¼��������߳�
						QCoreApplication::postEvent(interfaceobject, ptempevent);
					}
				}
			}
		}


		////////////////////////////////////////////////////////////////////////////
		//����ͷ�������ò���
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.printchgspeedansflag == true)
		{
			//��ʼ��
			if (pubcom.singleprintchgspeedstep == 1)
			{
				pubcom.singleprintchgspeedtimer = 0;
				pubcom.singleprintchgspeedstep = 2;
			}
			//����
			else if (pubcom.singleprintchgspeedstep == 2)
			{
				//����
				tcmd tempcmd;
				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x05;

				QDomNode paranode =
					GetParaByName("prod", QString("��ͷ��������"));
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
			}//����
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
		//Ħ��ϵ��������̺��ʼת��
		////////////////////////////////////////////////////////////////////////////
		//��ʼ��
		if (pubcom.measurepulsestartstep == 1)
		{
			pubcom.measurepulsestartcount = 0;
			pubcom.measurepulsestarttimer = 0;
			pubcom.measurepulsestartstep = 2;
		}
		//���ÿ�ʼ
		else if (pubcom.measurepulsestartstep == 2)
		{
			WritePcMotorOnBit(&regs,true);

			pubcom.measurepulsestartstep = 3;
			pubcom.measurepulsestarttimer = pubtimermng.addtimer();
		}//��ʱһ��ʱ��
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

				//����������
				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x0a;//��̺
				ptempevent->status = 0x00;//����

				//����һ���¼��������߳�
				QCoreApplication::postEvent(measurepulseobject, ptempevent);
			}
		}


		////////////////////////////////////////////////////////////////////////////
		//Ħ��ϵ��������ֹ̺ͣת��
		////////////////////////////////////////////////////////////////////////////
		//��ʼ��
		if (pubcom.measurepulsestopstep == 1)
		{
			pubcom.measurepulsestopcount = 0;
			pubcom.measurepulsestoptimer = 0;
			pubcom.measurepulsestopstep = 2;
		}
		//���ÿ�ʼ
		else if (pubcom.measurepulsestopstep == 2)
		{
			WritePcMotorOffBit(&regs,true);

			pubcom.measurepulsestopstep = 3;
			pubcom.measurepulsestoptimer = pubtimermng.addtimer();
		}//��ʱһ��ʱ��
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

				//����������
				backendevent* ptempevent = new backendevent();
				ptempevent->cmd = 0x0a;//��̺
				ptempevent->status = 0x01;//ֹͣ

				//����һ���¼��������߳�
				QCoreApplication::postEvent(measurepulseobject, ptempevent);
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//����Ӧ��
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.isapply == true)
		{
			//�����޸ĵģ�����CAN����
			if (pubcom.applystep == 1)
			{
				pubcom.applycount = 0;

				//�������
				pubcom.applyparacmdpair.clear();


				//���ղ���
				QMap<tablekey, tablenode>::iterator i;
				for (i = pDlgset->tablemapprod.begin(); i
					!= pDlgset->tablemapprod.end(); ++i)
				{
					bool ok;

					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{

						if (i.value().id.toElement().text() == QString(
							"�����ٶ�"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetHMSpeed(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"�޸������ٶ�","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"���������ٶ�"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetHMSpeedHigh(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"���������ٶ�","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�Ի��ٶ�"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetJogSpeed(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"�Ի��ٶ�","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"���ٶԻ��ٶ�"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetJogSpeedHigh(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"���ٶԻ��ٶ�","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"���ٵ����ٶԻ�ʱ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							pubcom.LowToHighSpeedtime = i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"���ٶԻ��ٶ�","�޸��˹��ղ���",1);
						}						
						else if (i.value().id.toElement().text() == QString("̧����ʱ"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"̧����ʱ","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"������ʱ"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();
							//��ʼ��
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


							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"������ʱ","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"������ѡ��"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"������ѡ��","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"����Ի��ٶ�"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"����Ի��ٶ�","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"б��Ի��ٶ�"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}



							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"б��Ի��ٶ�","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��ͷ��������"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"��ͷ��������","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��/б����ǿ�ƿ�"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"��/б����ǿ�ƿ�","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"̧����ת��ʱ"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"̧����ת��ʱ","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"���ÿ���ʱ��"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;
							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"���ÿ���ʱ��","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�趨ӡ������ͣ��"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							pubcom.NeedStopAtWorkfinished = i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"�趨ӡ������ͣ��","�޸��˹��ղ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��̨��ʼ����"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;
							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���ղ���",
								"��̨��ʼ����","�޸��˹��ղ���",1);
						}
					}
				}
				//��������1
				//QMap<tablekey,tablenode>::iterator i;
				for (i = pDlgset->tablemapmachsub1.begin(); i
					!= pDlgset->tablemapmachsub1.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{

						if (i.value().id.toElement().text() == QString(
							"̧����ת�ٶȼ��������"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetCCWJogSpeed(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"̧����ת�ٶȼ��������","�޸��˻�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"̧/����������ʱ"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"̧/����������ʱ","�޸��˻�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��/����������ʱ"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"��/����������ʱ","�޸��˻�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��/���ϸ�������ʱ"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"��/���ϸ�������ʱ","�޸��˻�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�ϸ�����������ʱ"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}



							QList<int>::iterator it;

							pubcom.activeprintlistmutex.lockForRead();

							//��ʼ��
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"�ϸ�����������ʱ","�޸��˻�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��ͷ��������ť�Ƿ���"))
						{
							//�����޸����
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"��ͷ��������ť�Ƿ���","�޸��˻�������",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"�ŷ��������"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"�ŷ��������","�޸��˻�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��������ѡ����"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetPulseDirect(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"��������ѡ����","�޸��˻�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"ȫ�����������"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetMaxSpeed(
								i.value().node.toElement().text().toInt(&ok)
								* 1000);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"ȫ�����������","�޸��˻�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�Ͻ���̨��Ӧ��̨���"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							pubcom.Shangjiangcitaibianhao = i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"�Ͻ���̨��Ӧ��̨���","�޸��˻�������",1);
						}
					}
				}
				//��������2
				for (i = pDlgset->tablemapmachsub2.begin(); i
					!= pDlgset->tablemapmachsub2.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						for (int jj = 1; jj <= 16; ++jj)
						{
							QString ParaName = QString("��ͷ") + QString::number(jj) + QString("����λ��ƫ��");
							if (i.value().id.toElement().text() == ParaName)
							{
								//�����޸����
								if (i.value().flag.toElement().text().toInt(&ok)
									== 1)
								{
									i.value().flag.firstChild().setNodeValue(
										QString::number(0));
								}

								ksmc->SetMotorOffset(jj,i.value().node.toElement().text().toDouble(
									&ok));

								//�����¼��������û�ע��
								publiccaution.addevent(
									"��������",
									ParaName,"�޸��˻�������",1);
							}
						}
					}
				}

				//��������3
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
							QString ParaName = QString("��ͷ") + QString::number(jj) + QString("��Ӧ���ñ��");

							if (i.value().id.toElement().text() == ParaName
								&& (pubcom.PrintNum >= jj)
								&& (pubcom.activeprintlist.contains(jj-1)))
							{
								//�����޸����
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

								//�����¼��������û�ע��
								publiccaution.addevent(
									"��������",
									ParaName,"�޸��˻�������",1);
							}
						}
					}
				}

				//��������4
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
							QString ParaName = QString("����") + QString::number(jj) + QString("�����");

							if (i.value().id.toElement().text() == ParaName )
							{
								//�����޸����
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

								//�����¼��������û�ע��
								publiccaution.addevent(
									"��������",
									ParaName,"�޸��˻�������",1);
							}
						}
					}
				}

				//��������5
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
							QString ParaName = QString("��̨") + QString::number(jj) + QString("�����");

							if (i.value().id.toElement().text() == ParaName )
							{
								//�����޸����
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

								//�����¼��������û�ע��
								publiccaution.addevent(
									"��������",
									ParaName,"�޸��˻�������",1);
							}
						}
					}
				}

				//��������6
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
							QString ParaName = QString("��̨") + QString::number(jj) + QString("ģ���������");

							if (i.value().id.toElement().text() == ParaName )
							{
								//�����޸����
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

								//�����¼��������û�ע��
								publiccaution.addevent(
									"��������",
									ParaName,"�޸��˻�������",1);

							}
						}
					}
				}

				//���Բ�����ȫ����ɫ��
				for (i = pDlgset->tablemaptune.begin(); i
					!= pDlgset->tablemaptune.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{

						if (i.value().id.toElement().text() == QString("ȫ����ɫ��"))
						{
							//�����޸����
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

							//�̱߳���
							pubcom.activeprintlistmutex.lockForWrite();
							pubcom.activeprintlist.clear();

							//������ͷ����������ͷ����
							for (int ii = 0 + PRINT1BDID; ii < pubcom.PrintNum
								+ PRINT1BDID; ii++)
							{
								pubcom.bdlist.append(ii);

								if (pubcom.presetprintstatus[ii] != 1)
								{
									//���õ�ǰ�������ͷ����
									pubcom.activeprintlist.append(ii);
								}
							}
							pubcom.activeprintlistmutex.unlock();

							//ɾ����ͷ�Ĺ���
							for (int ii = PRINT1BDID;  ii <= PRINT16BDID ; ++ ii)
							{
								if (ii >= pubcom.PrintNum )
								{
									//ɾ������
									pubcom.delcauflag = true;
									publiccaution.delcaution(
										pubcom.bdcautionmap.value(ii));
									pubcom.delcauflag = false;
								}
								else
								{
									//ɾ���Ǽ�����ͷ����
									if (pubcom.presetprintstatus[ii] == 1)
									{
										pubcom.delcauflag = true;
										publiccaution.delcaution(
											pubcom.bdcautionmap.value(ii));
										pubcom.delcauflag = false;

									}
								}
							}

							//�������Ĳ���Ҫ����Ҫȷ���Ƿǹ���״̬�ſ�������
							ksmc->SetOffMotor(-1);
							ksmc->DelMotor(-1);

							//������ͷ�����������
							for (int ii = 0; ii < pubcom.PrintNum; ii++)
							{
								if (pubcom.activeprintlist.contains(ii) == true)
								{
									ksmc->AddMotor(ii + 1);
								}
							}


							//�������������Ҫ������������������
							int paraVal = 0;
							QDomNode para;

							//��������������
							para = GetParaByName("driver", "��һλ������");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,16,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "��һ�ٶ�����");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,17,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "��һ�ٶȻ���ʱ�䳣��");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,18,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "��һ�ٶȼ�����");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,19,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "��һŤ�ع���ʱ�䳣��");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,20,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "�ڶ�λ������");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,24,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "�ڶ��ٶ�����");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,25,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "�ڶ��ٶȻ���ʱ�䳣��");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,26,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "�ڶ��ٶȼ�����");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,27,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "�ڶ�Ť�ع���ʱ�䳣��");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,28,paraVal);

									}
								}
							}
							//��������������
							para = GetParaByName("driver", "�ڶ���������");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,48,paraVal);

									}
								}
							}
							//��������������
							para = GetParaByName("driver", "�����л�ģʽ");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,49,paraVal);

									}
								}
							}
							//��������������
							para = GetParaByName("driver", "�����л��ӳ�ʱ��");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,50,paraVal);

									}
								}
							}
							//��������������
							para = GetParaByName("driver", "�����л�ˮƽ");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,51,paraVal);

									}
								}
							}
							//��������������
							para = GetParaByName("driver", "�����л��ͺ�");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,52,paraVal);

									}
								}
							}

							//��������������
							para = GetParaByName("driver", "λ�û������л�ʱ��");
							paraVal = para.firstChildElement("value").text().toInt(&ok);

							if (pubcom.PrintNum > oldPrintNum)
							{
								for ( int index = oldPrintNum + 1; index <= pubcom.PrintNum ; index++ )
								{
									int networkcanset = -1;

									ksmc->GetDriverPar(index,10,networkcanset);
									//����Ϊ0���ʾ����ͨ����������
									if (networkcanset == 0)
									{
										ksmc->SetDriverPar(index,53,paraVal);

									}
								}
							}

							//��ʼ�����
							ksmc->InitMotor(-1);

							//�������
							ksmc->SetActiveMotor(-1);

							//��ͷ״̬���·���
							if (pubcom.statuschgstep == 0 
								&& pubcom.plcreqstartstep == 0)
							{
								pubcom.statuschgstep = 1;
							}

							pubcom.activeprintlistmutex.lockForRead();
							QList<int>::iterator it;

							//��ʼ��
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


							//ɾ���Ͱ����й�ϵ���ڴ����

							//ˢ�´�̨�ɼ�����
							backendevent* ptempevent = new backendevent();
							ptempevent->cmd = 0x07;//ˢ�´�̨�ɼ�����
							//����һ���¼��������߳�
							QCoreApplication::postEvent(interfaceobject,
								ptempevent);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"ȫ����ɫ��","�޸��˵��Բ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�ŷ��������תѡ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}
							pubcom.ServoDir = i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"�ŷ��������תѡ��","�޸��˵��Բ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"˫�����ò���"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"˫�����ò���","�޸��˵��Բ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"Ħ��ϵ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetPulserPerMeter(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"Ħ��ϵ��","�޸��˵��Բ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"���������ź���Ӧʱ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"���������ź���Ӧʱ��","�޸��˵��Բ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��ͷ�������"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetOffset(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"��ͷ�������","�޸��˵��Բ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"��ͷ����"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetToothNum2(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"��ͷ����","�޸��˵��Բ���",1);
						}

						else if (i.value().id.toElement().text() == QString(
							"��ͷ�ܳ�"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetGirth(
								i.value().node.toElement().text().toFloat(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"��ͷ�ܳ�","�޸��˵��Բ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�������ֱ���"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetPulsePerCircle(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"�������ֱ���","�޸��˵��Բ���",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�����ͺ���"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}
							pubcom.Follow = i.value().node.toElement().text().toFloat(&ok) * 0.01;

							//�����¼��������û�ע��
							publiccaution.addevent(
								"���Բ���",
								"�����ͺ���","�޸��˵��Բ���",1);
						}
					}
				}

				//��������
				for (i = pDlgset->tablemapdev.begin(); i
					!= pDlgset->tablemapdev.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						if (i.value().id.toElement().text() == QString(
							"�ŷ�����������ٱ�"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetServGearBoxScale(
								i.value().node.toElement().text().toDouble(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"�ŷ�����������ٱ�","�޸��˿�������",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"�ŷ������������������"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->SetToothNum1(
								i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"�ŷ������������������","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"can��ѡ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//����Ӧ�ò���
							candevice.close();
							if (candevice.open() == true)
							{
								publiccaution.delcaution(PCCAUTIONLOCATION,PCNOCANCATION);
							}


							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"can��ѡ��","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"can����"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//����Ӧ�ò���
							candevice.close();
							if (candevice.open() == true)
							{
								publiccaution.delcaution(PCCAUTIONLOCATION,PCNOCANCATION);
							}

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"can����","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"����ѡ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialName(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//����Ӧ�ò���
							modbusRTU_slave_SerialClose(&modbusRTU);
							//ȷ�ϴ��ڿ������´򿪣�����Ҫ�����ߣ��ȴ�event��������
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"����ѡ��","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"����ͨѶ����"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}
							modbusRTU_slave_SetSerialBaudRate(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//����Ӧ�ò���
							modbusRTU_slave_SerialClose(&modbusRTU);
							//ȷ�ϴ��ڿ������´򿪣�����Ҫ�����ߣ��ȴ�event��������
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
							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"����ͨѶ����","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"����У�鷽ʽ"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialParity(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//����Ӧ�ò���
							modbusRTU_slave_SerialClose(&modbusRTU);
							//ȷ�ϴ��ڿ������´򿪣�����Ҫ�����ߣ��ȴ�event��������
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"����У�鷽ʽ","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�����ֽ�λ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialByteSize(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//����Ӧ�ò���
							modbusRTU_slave_SerialClose(&modbusRTU);
							//ȷ�ϴ��ڿ������´򿪣�����Ҫ�����ߣ��ȴ�event��������
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
							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"�����ֽ�λ��","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"����ֹͣλ"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialStopBits(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//����Ӧ�ò���
							modbusRTU_slave_SerialClose(&modbusRTU);
							//ȷ�ϴ��ڿ������´򿪣�����Ҫ�����ߣ��ȴ�event��������
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"����ֹͣλ","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"RTS������"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							modbusRTU_slave_SetSerialfRtsControl(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

#ifdef HAVESERIAL

							//����Ӧ�ò���
							modbusRTU_slave_SerialClose(&modbusRTU);
							//ȷ�ϴ��ڿ������´򿪣�����Ҫ�����ߣ��ȴ�event��������
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

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"RTS������","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"���������ź��˲�����"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//����Ӧ�ò���
							ksmc->SetFollowFilter(i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"���������ź��˲�����","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"CANʧ�ܴ�����Ϊ��������"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//����Ӧ�ò���
							pubcom.canfailedcnt = i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"CANʧ�ܴ�����Ϊ��������","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"���㴫�����źſ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//����Ӧ�ò���
							pubcom.ZeroDetectWidth = i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"���㴫�����źſ��","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"������ٶο��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//����Ӧ�ò���
							pubcom.ZeroHighSpeedWidth = i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"������ٶο��","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"������ٶο��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//����Ӧ�ò���
							pubcom.ZeroLowSpeedWidth = i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"������ٶο��","�޸��˿�������",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"�Ƿ��¼������Ϣ"))
						{
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							//����Ӧ�ò���
							pubcom.bldbginforecflag = (bool)i.value().node.toElement().text().toInt(&ok);

							//�����¼��������û�ע��
							publiccaution.addevent(
								"��������",
								"�Ƿ��¼������Ϣ","�޸��˿�������",1);

						}
					}
				}

				//����������
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
							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							ksmc->GetDriverPar(index+1,10,networkcanset);
							//����Ϊ0���ʾ����ͨ����������
							if (networkcanset == 0)
							{
								ksmc->SetDriverPar(index+1,parIndex,i.value().node.toElement().text().toInt(&ok));

							}
							//�����¼��������û�ע��
							publiccaution.addevent(
								"����������",
								i.value().id.toElement().text(),"�޸�������������",1);

						}
					}
				}

				//MODBUS����
				for (i = pDlgset->tablemapmodbus.begin(); i
					!= pDlgset->tablemapmodbus.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						if (i.value().id.toElement().text() == QString(
							"ID���"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}
							set_modbusRTU_slave_ModbusID(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"MODBUS����",
								"ID���","�޸���MODBUS����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"1.5Tʱ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							set_modbusRTU_slave_t15(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"MODBUS����",
								"1.5Tʱ��","�޸���MODBUS����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"3.5Tʱ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							set_modbusRTU_slave_t35(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"MODBUS����",
								"3.5Tʱ��","�޸���MODBUS����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"Ӧ��ȴ�ʱ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							set_modbusRTU_slave_answaittime(&modbusRTU,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"MODBUS����",
								"Ӧ��ȴ�ʱ��","�޸���MODBUS����",1);

						}
					}
				}

				//PLCIO����
				for (i = pDlgset->tablemapplcio.begin(); i
					!= pDlgset->tablemapplcio.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						if (i.value().id.toElement().text() == QString(
							"���ػ���������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcEnableStartBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ���������λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ���������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							SetPcMotorAccBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ���������λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ���������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcMotorDecBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ���������λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ�����λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							SetPcErrBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ�����λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ�����λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							SetPcCauBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ�����λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ�����̺����λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcMotorOnBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ�����̺����λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ���ֹ̺ͣλλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcMotorOffBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ���ֹ̺ͣλλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ���������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcWorkFinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ���������λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ���������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcSetHeartBeatBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ���������λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ�����ˢ������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcFlushDataReqBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"���ػ�����ˢ������λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"PLC������������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcStartReqBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC������������λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"PLC����ֹͣλλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcStopBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC����ֹͣλλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"PLC����λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}



							SetPlcErrBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC����λλ��","�޸���PLCIO����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"PLC����λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcCauBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC����λλ��","�޸���PLCIO����",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC��������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcJinbuyunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC��������λλ��","�޸���PLCIO����",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLCӡ����������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcYinhuadaodaiyunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLCӡ����������λλ��","�޸���PLCIO����",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC�淿����1����λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcHongfangwangdai1yunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC�淿����1����λλ��","�޸���PLCIO����",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC�淿����2����λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcHongfangwangdai2yunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC�淿����2����λλ��","�޸���PLCIO����",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC�䲼����λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcLuobuyunxinBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC�䲼����λλ��","�޸���PLCIO����",1);
						}
						else if (i.value().id.toElement().text() == QString(
							"PLC������������λλ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPlcGongjiangcilikaiguanBitIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCIO����",
								"PLC������������λλ��","�޸���PLCIO����",1);
						}

					}
				}



				//PLCPARA����
				for (i = pDlgset->tablemapplcpara.begin(); i
					!= pDlgset->tablemapplcpara.end(); ++i)
				{
					bool ok;
					if (i.value().flag.toElement().text().toInt(&ok) == 1
						|| pubcom.refreshapply == true)
					{
						if (i.value().id.toElement().text() == QString(
							"���ػ����IO��λ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}

							SetPcOutIOWordIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCPARA����",
								"���ػ����IO��λ��","�޸���PLCPARA����",1);

						}
						else if (i.value().id.toElement().text() == QString(
							"���ػ�����˫��λ��"))
						{

							//�����޸����
							if (i.value().flag.toElement().text().toInt(&ok)
								== 1)
							{
								i.value().flag.firstChild().setNodeValue(
									QString::number(0));
							}


							SetPcWorkRecDWordIndex(&regs,i.value().node.toElement().text().toInt(&ok));

							//�����¼��������û�ע��
							publiccaution.addevent(
								"PLCPARA����",
								"���ػ�����˫��λ��","�޸���PLCPARA����",1);

						}
					}
				}

				if (pubcom.refreshapply == true)
				{
					pubcom.refreshapply = false;
				}

				//��ʼ�����ж��Ƿ�����ҪCAN���������
				if (pubcom.applyparacmdpair.isEmpty() == true)
				{
					pubcom.isapply = false;
					pubcom.applystep = 0;

					//����������
					backendevent* ptempevent = new backendevent();
					if (pubcom.applyansplace == 1)
					{
						ptempevent->cmd = 0x04;//����Ӧ������
					}
					else if (pubcom.applyansplace == 2)
					{
						ptempevent->cmd = 0x05;//����Ӧ������
					}

					ptempevent->data[0] = 0x00;

					ptempevent->status = 0x01;//����״̬����ʶ��������û�з���

					//����һ���¼��������߳�
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
			//����CAN����
			else if (pubcom.applystep == 2)
			{
				//���Դ��� > 1
				if (pubcom.applycount > 1)
				{
					pubcom.isapply = false;
					pubcom.applystep = 0;

					//����������
					backendevent* ptempevent = new backendevent();
					if (pubcom.applyansplace == 1)
					{
						ptempevent->cmd = 0x04;//����Ӧ������
					}
					else if (pubcom.applyansplace == 2)
					{
						ptempevent->cmd = 0x05;//����Ӧ������
					}

					ptempevent->data[0] = 0x00;

					if (pubcom.hasparasucceed == true)
					{
						ptempevent->status = 0x02;//����״̬����ʶ��������û�з���
					}
					else
					{
						ptempevent->status = 0x03;//����״̬,û�в������óɹ�
					}

					//����һ���¼��������߳�
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
							//����ʧ�ܾͷ���
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

										//�����¼��������û�ע��
										publiccaution.addevent(
											pubcom.bdcautionmap.value(it.key()),
											SENDCMDFAILEVENT,"����CAN����ʧ��");
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

							//ÿ����������10ms
							msleep(10);
						}
					}

					//ֻҪһ�����ͳɹ��ˣ���Ҫ����Ӧ�𣬽������Ӧ��׶�
					if (sendonesucceed == true)
					{
						pubcom.applystep = 3;
						pubcom.applytimer = pubtimermng.addtimer();
					}
					else
					{
						//step ������ 2���ͻ����������
						//do nothing;
					}
				}

			}//�ȴ�CAN�����
			else if (pubcom.applystep == 3)
			{

				//������
				QMap<int,QList<tbdonlineparacmdpair> >::iterator it;

				for (it = pubcom.applyparacmdpair.begin(); it
					!= pubcom.applyparacmdpair.end(); ++it)
				{
					QList<tbdonlineparacmdpair>::iterator itt;
					for (itt = it.value().begin();itt != it.value().end(); ++ itt)
					{
						//�ɹ����Ͳ���û���յ��Ž���
						if ( (*itt).sendcmd.sendsucceed == true 
							&& (*itt).recvcmd.recvsucceed == false )
						{
							//���ñ�����Ҫ�ر���
							if ((*itt).sendcmd.cmd.cmd == SETBUMPNOCMD)
							{
								QList<tcmd> cmdlist;
								int count = candevice.getcmd((*itt).recvcmd.cmd.id,
									(*itt).recvcmd.cmd.cmd, cmdlist);

								//���յ���
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

								//���յ���
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
					bool isallget = true;//�Ƿ�ȫ�����յ�����ָ���η��ͳɹ��ģ�����û�з��ͳɹ��Ĳ��ж�

					QMap<int,QList<tbdonlineparacmdpair> >::iterator it;

					for (it = pubcom.applyparacmdpair.begin(); it
						!= pubcom.applyparacmdpair.end(); ++it)
					{
						QList<tbdonlineparacmdpair>::iterator itt;
						for (itt = it.value().begin();itt != it.value().end(); ++ itt)
						{
							//�ɹ����ͲŽ���
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

						//�ж�ȫ���������Ƿ�ȫ��Ӧ����
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
							//�������Է���
							pubcom.applystep = 2;
						}
						else
						{
							pubcom.isapply = false;
							pubcom.applystep = 0;

							//����������
							backendevent* ptempevent = new backendevent();
							if (pubcom.applyansplace == 1)
							{
								ptempevent->cmd = 0x04;//����Ӧ������
							}
							else if (pubcom.applyansplace == 2)
							{
								ptempevent->cmd = 0x05;//����Ӧ������
							}

							ptempevent->data[0] = 0x00;

							ptempevent->status = 0x01;//����״̬��ȫ�����

							//����һ���¼��������߳�
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
						//�������Է���
						pubcom.applystep = 2;
					}
					else
					{
						pubcom.isapply = false;
						pubcom.applystep = 0;

						//����������
						backendevent* ptempevent = new backendevent();
						if (pubcom.applyansplace == 1)
						{
							ptempevent->cmd = 0x04;//����Ӧ������
						}
						else if (pubcom.applyansplace == 2)
						{
							ptempevent->cmd = 0x05;//����Ӧ������
						}

						ptempevent->data[0] = 0x00;

						ptempevent->status = 0x01;//����״̬��ȫ���ɹ�

						//����һ���¼��������߳�
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
		//�������ߴ�������
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.applystep == 0)
		{
			QMap<int,int>::iterator it;
			for (it = pubcom.bdonlinestep.begin();it != pubcom.bdonlinestep.end(); ++it)
			{
				//��ʼ��
				if (it.value() == 1)
				{
					//������ͷ�壬������Ǽ���İ��ӣ��Ͳ�Ҫ���԰汾��
					if (it.key() >= PRINT1BDID 
						&& it.key() <= PRINT16BDID)
					{
						//������̬
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

							//ֱ��Ӧ�ò����������汾����
							pubcom.bdonlinestep.insert(it.key(),4);
						}
					}
					else
					{
						pubcom.bdonlinecount.insert(it.key(),0);
						pubcom.bdonlinetimer.insert(it.key(),0);
						pubcom.bdonlinestep.insert(it.key(),2);
					}

				}//�汾����
				else if (it.value() == 2)
				{
					if (pubcom.bdonlinecount.value(it.key()) > 2)
					{
						//�������ò���
						pubcom.bdonlinestep.insert(it.key(),4);
					}
					else
					{
						//������+1
						pubcom.bdonlinecount.insert(it.key(),pubcom.bdonlinecount.value(it.key()) + 1);

						//������
						tcmd tempcmd;
						tempcmd.cmd = VERTESTCMD;
						tempcmd.id = PCCANBDID;
						tempcmd.para1 = 0;
						tempcmd.para2 = 0;

						//��������
						if (candevice.sendcmd(pubcom.bdnodemap.value(it.key()),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(it.key()),
								&tempcmd) == false)
							{
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(it.key()),
									SENDCMDFAILEVENT,"����CAN����ʧ��");

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

				}//���հ汾����CAN����Ӧ��
				else if(it.value() == 3)
				{
					tcmd tempcmd;

					//ʱ����
					if (pubtimermng.gettimerval(pubcom.bdonlinetimer.value(it.key())) <= OUTTIME)
					{
						if (candevice.getcmd(it.key(), VERTESTCMD, &tempcmd) == true)
						{
							if (pubcom.chkver(tempcmd.para1, tempcmd.para2) == true)
							{
								//OK,������

							}
							else//�汾������
							{
								//���ð汾����
								publiccaution.addcaution(pubcom.bdcautionmap.value(
									it.key()), VERTESTCMD);
							}
							pubtimermng.deltimer(pubcom.bdonlinetimer.value(it.key()));					
							pubcom.bdonlinestep.insert(it.key(), 4);
						}
					}
					else //��ʱ
					{
						pubtimermng.deltimer(pubcom.bdonlinetimer.value(it.key()));					
						pubcom.bdonlinestep.insert(it.key(), 4);
					}
				}//����Ӧ��
				else if (it.value() == 4)
				{
					//�������
					pubcom.bdonlineparacmdpair[it.key()].clear();

					//���ð��Ӳ���Ӧ��
					if(it.key() == BUMPIOBDID)
					{
						bool ok;
						QDomNode para;
						tbdonlineparacmdpair t;
						tcmd tempcmd;

						for (int i = 1; i < 16; ++i)
						{
							QString paraname = QString("����") + QString::number(i) + QString("�����");
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
					//��̨����
					else if(it.key() == MAGICIOBDID)
					{
						bool ok;
						QDomNode para;
						tbdonlineparacmdpair t;
						tcmd tempcmd;

						for (int i = 1; i < 16; ++i)
						{
							QString paraname = QString("��̨") + QString::number(i) + QString("�����");
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
							QString paraname = QString("��̨") + QString::number(i) + QString("ģ���������");
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
					//��ͷ����
					else if (it.key() >= PRINT1BDID 
						&& it.key() <= PRINT16BDID)
					{
						bool ok;
						QDomNode para;
						tbdonlineparacmdpair t;
						tcmd tempcmd;

						para = GetParaByName("prod", "̧����ʱ");

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


						para = GetParaByName("prod", "������ʱ");

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


						para = GetParaByName("prod", "����Ի��ٶ�");

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

						para = GetParaByName("prod", "б��Ի��ٶ�");

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

						para = GetParaByName("prod", "��ͷ��������");

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


						para = GetParaByName("prod", "��/б����ǿ�ƿ�");

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

						para = GetParaByName("prod", "̧����ת��ʱ");

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

						para = GetParaByName("prod", "���ÿ���ʱ��");

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

						para = GetParaByName("prod", "��̨��ʼ����");

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

						para = GetParaByName("machsub1", "̧/����������ʱ");

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


						para = GetParaByName("machsub1", "��/����������ʱ");

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

						para = GetParaByName("machsub1", "��/���ϸ�������ʱ");

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

						para = GetParaByName("machsub1", "�ϸ�����������ʱ");

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

						QString paraname = QString("��ͷ") + QString::number(it.key()+ 1) + QString("��Ӧ���ñ��");
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

						//ȫ����ɫ��
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

						//��ͷ�ĳ�ʼ״̬
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

					//��������б�Ϊ��
					if (pubcom.bdonlineparacmdpair[it.key()].isEmpty() == true)
					{
						pubcom.bdonlinestep.insert(it.key(),0);
					}
					else
					{
						pubcom.bdonlinestep.insert(it.key(),5);
					}

				}//��ʼ��
				else if (it.value() == 5)
				{
					pubcom.bdonlinecount.insert(it.key(),0);
					pubcom.bdonlinetimer.insert(it.key(),0);
					pubcom.bdonlinestep.insert(it.key(),6);
				}
				//���Ͳ���
				else if(it.value() == 6)
				{
					if (pubcom.bdonlinecount.value(it.key()) > 2)
					{
						//�˳���������
						pubcom.bdonlinestep.insert(it.key(),0);

						//д�¼�
					}
					else
					{
						//������+1
						pubcom.bdonlinecount.insert(it.key(),pubcom.bdonlinecount.value(it.key()) + 1);

						bool sendonesucceed = false;
						QList<tbdonlineparacmdpair>::iterator itt;
						for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
						{
							//����ʧ�ܾͷ���
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

										//�����¼��������û�ע��
										publiccaution.addevent(
											pubcom.bdcautionmap.value(it.key()),
											SENDCMDFAILEVENT,"����CAN����ʧ��");
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

							//ÿ����������10ms
							msleep(10);
						}

						//ֻҪһ�����ͳɹ��ˣ���Ҫ����Ӧ�𣬽������Ӧ��׶�
						if (sendonesucceed == true)
						{
							pubcom.bdonlinestep.insert(it.key(),7);
							pubcom.bdonlinetimer.insert(it.key(),pubtimermng.addtimer());
						}
						else
						{
							//step ������ 6���ͻ����������
							//do nothing;
						}
					}
				}
				//���ܲ���Ӧ��
				else if(it.value() == 7)
				{
					QList<tbdonlineparacmdpair>::iterator itt;
					for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
					{
						//�ɹ����Ͳ���û���յ��Ž���
						if ( (*itt).sendcmd.sendsucceed == true 
							&& (*itt).recvcmd.recvsucceed == false )
						{
							//���ڽ��ñ���������Ҫ���⴦��
							if ((*itt).sendcmd.cmd.cmd == SETBUMPNOCMD)
							{
								QList<tcmd> cmdlist;
								int count = candevice.getcmd((*itt).recvcmd.cmd.id,
									(*itt).recvcmd.cmd.cmd, cmdlist);

								//���յ���
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

								//���յ���
								if (count >= 1)
								{
									(*itt).recvcmd.recvsucceed = true;
								}
							}
						}
					}

					if (pubtimermng.gettimerval(pubcom.bdonlinetimer.value(it.key())) < OUTTIME)
					{
						bool isallget = true;//�Ƿ�ȫ�����յ�����ָ���η��͵��Ƿ�ȫ���յ�

						for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
						{
							//�ɹ����ͲŽ���
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

							//ȫ���Ƿ��յ�Ӧ��
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

								//��¼�¼�
							}
						}
						else
						{
							//do nothing
						}
					}
					else
					{
						//��ʱ
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

							//��¼�¼�
						}
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////
		//���涯������Ӧ��ת�������棬��̨����
		////////////////////////////////////////////////////////////////////////////

		for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
		{
			if ((*i) == MAGICIOBDID)
			{
				QList<tcmd> cmdlist;
				int count = candevice.getcmd((*i), MAGICFULLCMD, PCCANBDID,
					cmdlist);

				//���������
				if (count >= 1)
				{
					QList<tcmd>::iterator j;
					for (j = cmdlist.begin(); j != cmdlist.end(); ++j)
					{
						//������룬�����

					}
				}
			}
		}


		//PLC������
		if ( pubcom.PlcStartUpFlag == true )
		{
			pubcom.PlcStartUpFlag = false;

			if (pubcom.statuschgstep == 0 
				&& pubcom.plcreqstartstep == 0)
			{
				//׼��״̬�²Ŵ�����
				if(pubcom.machprintstatus == 1)
				{
					//������룬��������������ʶ
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
				//����״̬����Ŵ���ͣ��
				if(pubcom.machprintstatus == 2)
				{
					//����״̬��������
					pubcom.machprintstatus = 1;
					pubcom.plcreqstopstep = 1;
				}
			}

		}

		//�޸���ͷ����״̬
		//��ʼ��
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

		}//���͵���ͷ
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

				//����
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
					//û�гɹ���������
					if (pubcom.statuschgsendcmdmap.value(*it) == false)
					{
						tempcmd.para1 = *it;

						//��������
						if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
							&tempcmd) == false)
						{
							msleep(1);
							if (candevice.sendcmd(pubcom.bdnodemap.value(*it),
								&tempcmd) == false)
							{
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"����CAN����ʧ��");

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

		}//����ͷ����
		else if (pubcom.statuschgstep == 3)
		{
			tcmd tempcmd;
			tempcmd.cmd = SETMACHSTATUSCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = 0x00;
			tempcmd.para2 = 0x00;

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//�ȷ���
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				//�ɹ�����������
				if (pubcom.statuschgsendcmdmap.value(*it) == true
					&& pubcom.statuschganscmdmap.value(*it) == false)
				{
					//��������
					if (candevice.getcmd((*it), SETMACHSTATUSCMD, &tempcmd)
						== true)
					{
						//�Ƿ�׼����
						//if (tempcmd.para2 == 0x01)
						//{
						//	pubcom.printreadymap.insert(*it, true);
						//}
						pubcom.statuschganscmdmap.insert(*it, true);
					}
				}
			}
			pubcom.activeprintlistmutex.unlock();

			//ʱ����
			if (pubtimermng.gettimerval(pubcom.statuschgtimer) <= OUTTIME)
			{
				//�Ƿ�ȫ���յ�
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

					//����ʧ�ܱ�ʶ
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
						pubcom.statuschgstep = 4; //û��ʧ�ܵģ���ô������һ��
					}
					else
					{
						pubcom.statuschgstep = 2; //�ط�

						for (it = pubcom.statuschganscmdmap.begin(); it
							!= pubcom.statuschganscmdmap.end(); ++it)
						{
							if (it.value() == false)
							{
								//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
								pubcom.statuschgsendcmdmap.insert(it.key(),
									false);
							}
						}
					}
				}
				else //û��ȫ�����յ���������
				{
					//do nothing
				}
			}
			else //��ʱ
			{
				QMap<int, bool>::iterator it;

				pubtimermng.deltimer(pubcom.statuschgtimer);

				//����ʧ�ܱ�ʶ
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
							//���ý���ʧ�ܵķ��ͱ�ʶ���������·���
							pubcom.statuschgsendcmdmap.insert(it.key(), false);
						}
					}
				}
			}
		}//������ñ�ʶ
		else if (pubcom.statuschgstep == 4)
		{
			pubcom.statuschgcount = 0;
			pubcom.statuschgtimer = 0;
			pubcom.statuschgstep = 0;
		}

		//�����PLC�Ĺ��Ϻʹ���λ
		if (publiccaution.IOHasCaution() == true)
		{

			//����IO����
			if (pubcom.IOCautionStatus == false)
			{
				pubcom.IOCautionStatus = true;
				WritePcCauBit(&regs,true);
			} 
		} 
		else
		{
			//�˳�IO����
			if (pubcom.IOCautionStatus == true)
			{
				pubcom.IOCautionStatus = false;
				WritePcCauBit(&regs,false);
			}
		}


		if (publiccaution.IOHasErr() == true)
		{
			//����IO����
			if (pubcom.IOErrStatus == false)
			{
				pubcom.IOErrStatus = true;
                WritePcErrBit(&regs,true);

			}
		} 
		else
		{
			//�˳�IO����
			if (pubcom.IOErrStatus == true)
			{
				pubcom.IOErrStatus = false;
				WritePcErrBit(&regs,false);
			}
		}

		//����ʱ�Ƿ����·Ŵ�̨δ�Ӵ�
		{
			QList<int>::iterator it;
			bool magicflag = false;

			pubcom.activeprintlistmutex.lockForRead();

			//�ȷ���
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
		
		//д��ͷ�ʹ�̨��״̬���Ĵ���
		do{

			//�Ƿ���״̬���ֽ�
			regs.writearray[3*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.presetprintstatus[8] == 0x03))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.presetprintstatus[9] == 0x03))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.presetprintstatus[10] == 0x03))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.presetprintstatus[11] == 0x03))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.presetprintstatus[12] == 0x03))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.presetprintstatus[13] == 0x03))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.presetprintstatus[14] == 0x03))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.presetprintstatus[15] == 0x03))<<7);
			//�Ƿ���״̬���ֽ�
			regs.writearray[3*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.presetprintstatus[0] == 0x03))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.presetprintstatus[1] == 0x03))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.presetprintstatus[2] == 0x03))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.presetprintstatus[3] == 0x03))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.presetprintstatus[4] == 0x03))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.presetprintstatus[5] == 0x03))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.presetprintstatus[6] == 0x03))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.presetprintstatus[7] == 0x03))<<7);

			//��ͷ�·�״̬���ֽ�
			regs.writearray[4*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.updownstatus[8] == 0))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.updownstatus[9] == 0))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.updownstatus[10] == 0))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.updownstatus[11] == 0))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.updownstatus[12] == 0))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.updownstatus[13] == 0))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.updownstatus[14] == 0))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.updownstatus[15] == 0))<<7);
			//��ͷ�·�״̬���ֽ�
			regs.writearray[4*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.updownstatus[0] == 0))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.updownstatus[1] == 0))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.updownstatus[2] == 0))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.updownstatus[3] == 0))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.updownstatus[4] == 0))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.updownstatus[5] == 0))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.updownstatus[6] == 0))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.updownstatus[7] == 0))<<7);

			//��̨����״̬���ֽ�
			regs.writearray[5*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.magicstatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.magicstatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.magicstatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.magicstatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.magicstatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.magicstatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.magicstatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.magicstatus[15] == 1))<<7);
			//��̨����״̬���ֽ�
			regs.writearray[5*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.magicstatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.magicstatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.magicstatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.magicstatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.magicstatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.magicstatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.magicstatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.magicstatus[7] == 1))<<7);

			//����״̬���ֽ�
			regs.writearray[6*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.zerostatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.zerostatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.zerostatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.zerostatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.zerostatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.zerostatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.zerostatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.zerostatus[15] == 1))<<7);
			//����״̬���ֽ�
			regs.writearray[6*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.zerostatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.zerostatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.zerostatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.zerostatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.zerostatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.zerostatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.zerostatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.zerostatus[7] == 1))<<7);

			//��ת״̬���ֽ�
			regs.writearray[7*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.circlestatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.circlestatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.circlestatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.circlestatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.circlestatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.circlestatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.circlestatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.circlestatus[15] == 1))<<7);
			//��ת״̬���ֽ�
			regs.writearray[7*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.circlestatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.circlestatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.circlestatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.circlestatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.circlestatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.circlestatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.circlestatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.circlestatus[7] == 1))<<7);


			//������״̬���ֽ�
			regs.writearray[8*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.lockreleasestatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.lockreleasestatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.lockreleasestatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.lockreleasestatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.lockreleasestatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.lockreleasestatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.lockreleasestatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.lockreleasestatus[15] == 1))<<7);
			//������״̬���ֽ�
			regs.writearray[8*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.lockreleasestatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.lockreleasestatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.lockreleasestatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.lockreleasestatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.lockreleasestatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.lockreleasestatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.lockreleasestatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.lockreleasestatus[7] == 1))<<7);
			//�ϸ�����״̬���ֽ�
			regs.writearray[9*2] = ((unsigned char)((pubcom.PrintNum >8)&&(pubcom.colorpipestatus[8] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >9)&&(pubcom.colorpipestatus[9] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >10)&&(pubcom.colorpipestatus[10] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >11)&&(pubcom.colorpipestatus[11] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >12)&&(pubcom.colorpipestatus[12] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >13)&&(pubcom.colorpipestatus[13] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >14)&&(pubcom.colorpipestatus[14] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >15)&&(pubcom.colorpipestatus[15] == 1))<<7);
			//�ϸ�����״̬���ֽ�
			regs.writearray[9*2+1] = ((unsigned char)((pubcom.PrintNum >0)&&(pubcom.colorpipestatus[0] == 1))<<0)
									| ((unsigned char)((pubcom.PrintNum >1)&&(pubcom.colorpipestatus[1] == 1))<<1)
									| ((unsigned char)((pubcom.PrintNum >2)&&(pubcom.colorpipestatus[2] == 1))<<2)
									| ((unsigned char)((pubcom.PrintNum >3)&&(pubcom.colorpipestatus[3] == 1))<<3)
									| ((unsigned char)((pubcom.PrintNum >4)&&(pubcom.colorpipestatus[4] == 1))<<4)
									| ((unsigned char)((pubcom.PrintNum >5)&&(pubcom.colorpipestatus[5] == 1))<<5)
									| ((unsigned char)((pubcom.PrintNum >6)&&(pubcom.colorpipestatus[6] == 1))<<6)
									| ((unsigned char)((pubcom.PrintNum >7)&&(pubcom.colorpipestatus[7] == 1))<<7);
		}while(0);


		//��Ϊһ���澯�������û������粻�Ǻܽ���
		if (candevice.getcmdcount() > UNKNOWCMDNUM)
		{
			//�����¼��������û�ע��
			publiccaution.addevent(CANCAUTIONLOCATION, TOOMUCHUNUSEDCMDEVENT);
		}

		msleep(1);//��Ҫ�����߻���Ҫ�ģ���������ʱ��ķ��ڿ�ѭ��
	}
}


