/*
* backend2.cpp
*
*  Created on: 2009-8-8
*      Author: ��־��
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

//ȫ�ֵ�can�豸
extern canif candevice;
extern QObject* setobject;
extern timermng pubtimermng;
extern QObject* opobject;


//A4N���
extern CKSMCA4 *ksmc;

//���ݿ�
extern DB mydb;

void restoreTaskRun(struct restore* pTask);
void savelocTaskRun(struct saveloc* pTask);

//����ά��ʱ��
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
		//������
		candevice.refresh();

		//���㴦��
		{
			QList<int>::iterator it;

			//���̱߳���
			pubcom.activeprintlistmutex.lockForRead();
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{

				//������ת
				if (pubcom.actionzerostep.value(*it) == 1)
				{
					//������ʱ��
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

					//������ת
					ksmc->MoveHomeHigh(*it + 1, 1);

					//��ʶ
					pubcom.actionzerostep.insert(*it, 2);
				}
				//�ȴ���λ�ź�
				else if (pubcom.actionzerostep.value(*it) == 2)
				{
					//����CAN����
					QList<tcmd> cmdlist;
					int count = candevice.getcmd((*it), ZEROLOCCMD, cmdlist);

					//�������λ�ź�
					if (count >= 1)
					{
						double dd = 0.0;
						ksmc->GetLocation(*it + 1, dd);

						pubcom.actionzeroloc1.insert(*it, dd);

						//Ӧ������
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"����CAN����ʧ��");

							}
						}

						//ɾ���ϵĶ�ʱ��
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//��ʶ
						pubcom.actionzerostep.insert(*it, 3);

					}
					//�ȴ��Ƿ�ʱ
					else if (pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) > ZEROOUTTIME)
					{
						//ɾ���ϵĶ�ʱ��
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//ֹͣת��
						ksmc->MoveStop(*it + 1);

						//��ʶ,�˳�����
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

					//������ʱ��
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());
				}
				//��ʱ��ָ���·��꣬�������ϲ�ѯ�Ƿ�λ�������ѯ������һ�ε�
				else if(pubcom.actionzerostep.value(*it) == 4)
				{
					if(pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) >= 2)
					{
						pubcom.actionzerostep.insert(*it, 5);
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));
					}
				}
				//��ת��λ
				else if(pubcom.actionzerostep.value(*it) == 5)
				{
					int status = 0;
					ksmc->GetMotorInPosStatus(*it+1,status);
					if ( status == 1 )
					{
						//��ʶ
						pubcom.actionzerostep.insert(*it, 6);
					}
				}
				//��ת��ĳλ��
				else if(pubcom.actionzerostep.value(*it) == 6)
				{
					double PulsePerUnit = 0.0;
					ksmc->GetPulsePerUnit(PulsePerUnit);

					double dd = pubcom.actionzeroloc1.value(*it) - PulsePerUnit * (pubcom.ZeroLowSpeedWidth + pubcom.ZeroDetectWidth);

					//�ƶ����ӽ����ظ���λ��
					ksmc->MoveAbsPTP(*it + 1, dd);

					//��ʶ
					pubcom.actionzerostep.insert(*it, 7);

					//������ʱ��
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

				}
				//��ʱ��ָ���·���
				else if(pubcom.actionzerostep.value(*it) == 7)
				{
					if(pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) >= 2)
					{
						pubcom.actionzerostep.insert(*it, 8);
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));
					}
				}
				//��ת��λ
				else if(pubcom.actionzerostep.value(*it) == 8)
				{
					int status = 0;
					ksmc->GetMotorInPosStatus(*it+1,status);
					if ( status == 1 )
					{
						//��ʶ
						pubcom.actionzerostep.insert(*it, 9);
					}
				}//���ٷ�ת
				else if (pubcom.actionzerostep.value(*it) == 9)
				{
					//������ʱ��
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

					//��ת
					ksmc->MoveHome(*it + 1, 0);

					//��ʶ
					pubcom.actionzerostep.insert(*it, 10);
				}
				//����λ�ź�
				else if (pubcom.actionzerostep.value(*it) == 10)
				{
					//����CAN����
					QList<tcmd> cmdlist;
					int count = candevice.getcmd((*it), ZEROLOCCMD, cmdlist);

					//���������
					if (count >= 1)
					{
						double dd = 0.0;
						ksmc->GetLocation(*it + 1, dd);
						pubcom.actionzeroloc2.insert(*it, dd);

						//Ӧ������
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"����CAN����ʧ��");
							}
						}

						//ɾ����ʱ��
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//��ʶ
						pubcom.actionzerostep.insert(*it, 11);

					}
					//�ȴ��Ƿ�ʱ
					else if (pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) > ZEROOUTTIME)
					{
						//ɾ���ϵĶ�ʱ��
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//ֹͣת��
						ksmc->MoveStop(*it + 1);

						//��ʶ,�˳�����
						pubcom.actionzerostep.insert(*it, 0);
					}
				}//��ת��ĳλ��
				else if (pubcom.actionzerostep.value(*it) == 11)
				{
					double PulsePerUnit = 0.0;
					ksmc->GetPulsePerUnit(PulsePerUnit);

					double dd = pubcom.actionzeroloc1.value(*it) + PulsePerUnit * (pubcom.ZeroHighSpeedWidth);

					ksmc->MoveAbsPTP(*it + 1, dd);

					//��ʶ
					pubcom.actionzerostep.insert(*it, 12);

					//������ʱ��
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

				}
				//��ʱ��ָ���·���
				else if(pubcom.actionzerostep.value(*it) == 12)
				{
					if(pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) >= 2)
					{
						pubcom.actionzerostep.insert(*it, 13);
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));
					}
				}
				//��ת��λ
				else if (pubcom.actionzerostep.value(*it) == 13)
				{
					int status = 0;
					ksmc->GetMotorInPosStatus(*it+1,status);
					if ( status == 1 )
					{
						//��ʶ
						pubcom.actionzerostep.insert(*it, 14);
					}

				}
				//��ת��ĳλ��
				else if (pubcom.actionzerostep.value(*it) == 14)
				{
					double PulsePerUnit = 0.0;
					ksmc->GetPulsePerUnit(PulsePerUnit);

					double dd = pubcom.actionzeroloc1.value(*it) + PulsePerUnit * (pubcom.ZeroLowSpeedWidth);

					//�ƶ����ӽ����ظ���λ��
					ksmc->MoveAbsPTP(*it + 1, dd);

					//��ʶ
					pubcom.actionzerostep.insert(*it, 15);

					//������ʱ��
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

				}
				//��ʱ�ȶ�������
				else if(pubcom.actionzerostep.value(*it) == 15)
				{
					if(pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) >= 2)
					{
						pubcom.actionzerostep.insert(*it, 16);
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));
					}
				}

				//��ת��λ
				else if (pubcom.actionzerostep.value(*it) == 16)
				{
					int status = 0;
					ksmc->GetMotorInPosStatus(*it+1,status);
					if ( status == 1 )
					{
						//��ʶ
						pubcom.actionzerostep.insert(*it, 17);
					}
				}
				//������ת
				else if (pubcom.actionzerostep.value(*it) == 17)
				{
					//�����µĶ�ʱ��
					pubcom.actionzerotimer.insert(*it, pubtimermng.addtimer());

					//��ת
					ksmc->MoveHome(*it + 1, 1);

					//��ʶ
					pubcom.actionzerostep.insert(*it, 18);
				}//����λ�ź�
				else if (pubcom.actionzerostep.value(*it) == 18)
				{
					//����CAN����
					QList<tcmd> cmdlist;
					int count = candevice.getcmd((*it), ZEROLOCCMD, cmdlist);

					//���������
					if (count >= 1)
					{
						double dd = 0.0;
						ksmc->GetLocation(*it + 1, dd);
						pubcom.actionzeroloc3.insert(*it, dd);

						//Ӧ������
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
								//�����¼��������û�ע��
								publiccaution.addevent(
									pubcom.bdcautionmap.value(*it),
									SENDCMDFAILEVENT,"����CAN����ʧ��");
							}
						}

						//ɾ���ϵĶ�ʱ��
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//��ʶ
						pubcom.actionzerostep.insert(*it, 19);

					}
					//�ȴ��Ƿ�ʱ
					else if (pubtimermng.gettimerval(
						pubcom.actionzerotimer.value(*it)) > ZEROOUTTIME)
					{
						//ɾ���ϵĶ�ʱ��
						pubtimermng.deltimer(pubcom.actionzerotimer.value(*it));

						//ֹͣת��
						ksmc->MoveStop(*it + 1);

						//��ʶ,�˳�����
						pubcom.actionzerostep.insert(*it, 0);

					}
				}
				else if (pubcom.actionzerostep.value(*it) == 19)
				{
					//��λ��

					int cir = 0;
					ksmc->GetPulsePerBigCircle(cir);

					if (cir > 0)
					{
						double a1,a2,a3;
						a1 = pubcom.actionzeroloc1.value(*it);
						a2 = pubcom.actionzeroloc2.value(*it);
						a3 = pubcom.actionzeroloc3.value(*it);

						//��λλ��,2����ƽ��
						double dd = (a2 + a3)/2;

						//������ͷ��ƫ�������µĵ�����
						double offset = 0;
						ksmc->GetOffset(offset);

						//��1����ͷ�ľ��룬���ǵ���װƫ��
						double Moffset1 = 0;   //1����ͷ�İ�װƫ��
						double Moffset2 = 0;   //������ͷ�İ�װƫ��
						ksmc->GetMotor1Offset(Moffset1);
						ksmc->GetMotorOffset(*it + 1,Moffset2);

						//offset = (*it) * offset - Moffset1 + Moffset2;
						offset = - Moffset1 + Moffset2;

						//Բ���ܳ�
						//double girth = 0;
						//ksmc->GetGirth(girth);

						//ȡ�࣬С�����2λ����
						//offset = offset - girth * (int)(offset/girth);
						//offset = offset - girth * (int)(offset/girth);

						//��λ���ȵ�����
						double pulseperunit = 0;
						ksmc->GetPulsePerUnit(pulseperunit);

						//ƫ������
						offset = offset * pulseperunit;

						//��Ȧ������
						int bigpulsenum = 0;
						ksmc->GetPulsePerBigCircle(bigpulsenum);

						//�����������ת(����̺��������һ��)
						if(pubcom.ServoDir == 0)
							dd = dd - offset - 2*bigpulsenum;
						else
							dd = dd - offset + 2*bigpulsenum;


						//�ƶ�����λ
						ksmc->MoveAbsPTP(*it + 1, dd);

						//������ͷ��λ��
						ksmc->SetPrintLocation(*it + 1,dd);
					}

					//��ʶ
					pubcom.actionzerostep.insert(*it, 0);
				}
			}
			pubcom.activeprintlistmutex.unlock();

		}

		////////////////////////////////////////////////////////////////////////////
		//ͣ�������
		////////////////////////////////////////////////////////////////////////////
		if (pubcom.plcreqstopstep == 1)
		{
			printf("��ʼͣ��\n");
			//���ֹͣ��ʱ��
			pubcom.plcreqstoptimer = pubtimermng.addtimer();
			pubcom.plcreqstopstep = 2;
		}
		else if (pubcom.plcreqstopstep == 2)
		{
			//�ȴ�0�ٶ��Ƿ��Ѿ���ʱ
			if (pubtimermng.gettimerval(pubcom.plcreqstoptimer) > MOTORSTOPTIME)
			{
				pubtimermng.deltimer(pubcom.plcreqstoptimer);

				//�澯������������ɸ澯
				pubcom.plcreqstopstep = 0; //ֹͣ��������
				printf("ͣ����ʱ\n");
			}
			else
			{
				//����������ٶȣ������Ƿ��Ѿ�ֹͣ
				if ( (abs(pubcom.motorspeed*10) <= 2) )//ֹͣ����ʱ���ٶ�0.2
				{
					zerospeedtimer = pubtimermng.addtimer();
					pubcom.plcreqstopstep = 3;
					pubtimermng.deltimer(pubcom.plcreqstoptimer);
				}
			}
		}
		else if(pubcom.plcreqstopstep == 3)
		{
			//ά��0�ٶ�ʱ���Ƿ��Ѿ�OK
			if (pubtimermng.gettimerval(zerospeedtimer) > 20)
			{
				pubtimermng.deltimer(zerospeedtimer);

				printf("�ɹ�ͣ��\n");
				QList<int>::iterator it;

				pubcom.activeprintlistmutex.lockForRead();

				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					if (pubcom.presetprintstatus[*it] == 3)
					{
						//ֹͣ����
						ksmc->DisableMasterFollowing(*it + 1);
						//printf("ֹͣ����%d\n",*it + 1);


						int networkcanset = -1;

						ksmc->GetDriverPar(*it+1,10,networkcanset);
						//����Ϊ0���ʾ����ͨ����������
						if (networkcanset == 0)
						{
							ksmc->SetDriverPar(*it+1,16,10);
							ksmc->SetDriverPar(*it+1,17,10);
							ksmc->SetDriverPar(*it+1,18,10);
							ksmc->SetDriverPar(*it+1,19,5);
						}

						//��������λ������
						if(pubcom.savelocTask[*it].step == 0)
						{
							pubcom.savelocTask[*it].step = 1;
						}
						//�˳���������λ��
						//ksmc->RememberPrintLoction(*it + 1);

					}
				}
				pubcom.activeprintlistmutex.unlock();
				pubcom.plcreqstopstep = 4;
			}
		}
		//���͵���ͷ
		else if (pubcom.plcreqstopstep == 4)
		{
			//���
			pubcom.sendprintcmdmap.clear();

			//����
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
						//�����¼��������û�ע��
						publiccaution.addevent(
							pubcom.bdcautionmap.value(*it),
							SENDCMDFAILEVENT,"����CAN����ʧ��");

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
			//ʱ����
			if (pubtimermng.gettimerval(pubcom.plcreqstoptimer) <= OUTTIME)
			{
				//����˽���
				if(pubcom.sendprintcmdmap.isEmpty())
				{
					pubcom.plcreqstopstep = 0;
					pubtimermng.deltimer(pubcom.plcreqstoptimer);
				}
				else//��������
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
						if (pubcom.presetprintstatus[*it] == 3)
						{
							//��������
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
		//����λ����������
		for(int iiii = 0;iiii < pubcom.PrintNum;iiii++)
		{
			savelocTaskRun(&(pubcom.savelocTask[iiii]));
		}


		msleep(1);//��Ҫ�����߻���Ҫ�ģ���������ʱ��ķ��ڿ�ѭ��
	}
}


