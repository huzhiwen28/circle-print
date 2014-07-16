/****************************************************************************
**
** Trolltech hereby grants a license to use the Qt/Eclipse Integration
** plug-in (the software contained herein), in binary form, solely for the
** purpose of creating code to be used with Trolltech's Qt software.
**
** Qt Designer is licensed under the terms of the GNU General Public
** License versions 2.0 and 3.0 ("GPL License"). Trolltech offers users the
** right to use certain no GPL licensed software under the terms of its GPL
** Exception version 1.2 (http://trolltech.com/products/qt/gplexception).
**
** THIS SOFTWARE IS PROVIDED BY TROLLTECH AND ITS CONTRIBUTORS (IF ANY) "AS
** IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
** PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
** OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** Since we now have the GPL exception I think that the "special exception
** is no longer needed. The license text proposed above (other than the
** special exception portion of it) is the BSD license and we have added
** the BSD license as a permissible license under the exception.
**
****************************************************************************/

#include "welcom.h"
#include <QEvent>
#include <iostream>
#include <Windows.h>
#include <QTimer>
#include <QDateTime>
#include "canif.h"
#include <QMessageBox>
#include "canidcmd.h"
#include "caution.h"
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QPainter>
#include <QPushButton>
#include <QImage>
#include <QAbstractButton>
#include <iostream>
#include <QTextStream>
#include <QFile>
#include <QSettings>
#include <QDateTime>
#include <QMessageBox>
#include <QDomNodeList>
#include <QTextStream>
#include "CMotor.h"
#include "caution.h"


//ȫ�ֵ�can�豸
extern canif candevice;
const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);

//A4N���
extern CKSMCA4 *ksmc;


welcom::welcom(QWidget *parent) :
QDialog(parent)
{
	pubcom.initflag = 0;
	timercnt = 0;
	ui.setupUi(this);
	ui.label->setPixmap(QPixmap(":/images/init.PNG").scaledToWidth(1024));

	//������ʱ��
	timer = &mytimer;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));
	timer->start(100); //100ms��ʱ

	//������ô��ʱ����ʾLOGO
	QTimer::singleShot(200, this, SLOT(system_init()));
}

welcom::~welcom()
{

}

void welcom::mousePressEvent(QMouseEvent *event)
{
	//���������ϣ�ֻҪ�û�������棬�ͻ��˳�LOGO���棬����������
	if (pubcom.initflag == 1)
	{
		this->accept();
	}
}

void welcom::system_init()
{
	pubcom.machprintstatus = 0;

	//ͨѶ����
	tcmd tempcmd;
	tempcmd.cmd = COMTESTCMD;
	tempcmd.id = PCCANBDID;
	tempcmd.para1 = 0;
	tempcmd.para2 = 0;

	//�㲥�ķ�ʽ���԰����Ƿ�����
	if (candevice.sendcmd(ALLID, &tempcmd) == false)
	{
		//ϵͳ���ϣ���������ʧ��
		publiccaution.addevent("CANͨѶ","ͨѶ����ʧ��","��ʼ��ͨѶ����ʧ��");

		//��Ϊ����ʧ�ܣ��϶����°��ӵ�ͨѶ����
		QList<int>::iterator i;
		for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
		{
			pubcom.nodeonlinemap.insert((*i), false);

			if ((*i) >= PRINT1BDID && (*i) <= PRINT16BDID)
			{
				if (pubcom.presetprintstatus[*i] != 1)
				{
					//���ð��Ӳ����߹���
					publiccaution.addcaution(pubcom.bdcautionmap.value((*i)),
						COMCAUTION);
				}
			}
			else
			{
				//���ð��Ӳ����߹���
				publiccaution.addcaution(pubcom.bdcautionmap.value((*i)),
					COMCAUTION);
			}
		}
	}
	else
	{
		Sleep(100); //����100ms

		//������
		candevice.refresh();

		//�鿴�Ƿ���Ӧ�������,�����жϳ������Ƿ�����
		QList<int>::iterator i;
		for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
		{
			QList<tcmd> cmdlist;
			int count = candevice.getcmd((*i), COMTESTCMD, 0x00,0x01,
				cmdlist);

			//��Ӧ��
			if (count >= 1)
			{
				pubcom.nodeonlinemap.insert((*i), true);
			}
			//else if(count > 1)
			//{
			//	//���ϣ��ظ�ID
			//	publiccaution.addcaution(pubcom.bdcautionmap.value(
			//		((*i))), REPIDCAUTION);
			//}
			else
			{
				pubcom.nodeonlinemap.insert((*i), false);

				//������ͷ���Ƿ��ǹ��ϣ��ÿ���ǰ��ͷ�Ƿ񼤻�
				if ((*i) >= PRINT1BDID && (*i) <= PRINT16BDID)
				{
					if (pubcom.presetprintstatus[*i] != 1)
					{
						//���ð��Ӳ����߹���
						publiccaution.addcaution(
							pubcom.bdcautionmap.value((*i)), COMCAUTION);
					}
				}
				else
				{
					//���ð��Ӳ����߹���
					publiccaution.addcaution(pubcom.bdcautionmap.value((*i)),
						COMCAUTION);
				}
			}
		}

		//�ڳɹ�ͨѶ��Ű汾���ԣ�����û������
		tempcmd.cmd = VERTESTCMD;
		tempcmd.id = PCCANBDID;
		tempcmd.para1 = 0;
		tempcmd.para2 = 1;

		//�㲥��ʽ���Ͱ汾����
		if (candevice.sendcmd(ALLID, &tempcmd) == false)
		{
			//ϵͳ���ϣ���������ʧ��
		}
		else
		{
			Sleep(100); //����100ms

			//������
			candevice.refresh();

			//�鿴���Ӱ汾�Ƿ�����
			QList<int>::iterator i;
			for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
			{
				//ֻ�а������߲�ȷ���Ƿ����������
				if (pubcom.nodeonlinemap.value(*i) == true && candevice.getcmd(
					(*i), VERTESTCMD, &tempcmd) == true)
				{
					//�Ƚϰ汾���
					//�汾����
					if (pubcom.chkver(tempcmd.para1, tempcmd.para2) == true)
					{
						//OK,������
					}
					else//�汾������
					{
						//������ͷ���Ƿ��ǹ��ϣ��ÿ���ǰ��ͷ�Ƿ񼤻�
						if ((*i) >= PRINT1BDID && (*i) <= PRINT16BDID)
						{
							if (pubcom.presetprintstatus[*i] != 1)
							{
								//���ð汾����
								publiccaution.addcaution(
									pubcom.bdcautionmap.value((*i)),
									VERTESTCMD);
							}
						}
						else
						{
							//���ð汾����
							publiccaution.addcaution(pubcom.bdcautionmap.value(
								(*i)), VERTESTCMD);
						}
					}
				}
			}
		}
	}


	//ˢ�²���
	{
		bool ok;
		QDomNode para;
		tbdonlineparacmdpair t;
		tcmd tempcmd;

		//����
		pubcom.bdonlineparacmdpair[BUMPIOBDID].clear();

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
			pubcom.bdonlineparacmdpair[BUMPIOBDID].append(t);

		}

		//��̨
		pubcom.bdonlineparacmdpair[MAGICIOBDID].clear();

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
			pubcom.bdonlineparacmdpair[MAGICIOBDID].append(t);

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
			pubcom.bdonlineparacmdpair[MAGICIOBDID].append(t);
		}

		//��ͷ
		for (int ii = 0; ii < pubcom.PrintNum; ++ii)
		{
			//�������
			pubcom.bdonlineparacmdpair[ii].clear();

			//�������ߵ���ͷ�ŷ��Ͳ���
			if (pubcom.nodeonlinemap.value(ii) == true)
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

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x03;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);


				para = GetParaByName("prod", "������ʱ");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x04;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x04;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "����Ի��ٶ�");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x01;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x01;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "б��Ի��ٶ�");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x02;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x02;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("machsub1", "��ͷ��������ť�Ƿ���");
				bool btwork = para.firstChildElement("value").text().toInt(&ok);
				if (btwork == true)
				{

					para = GetParaByName("prod", "��ͷ��������");

					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = 0x05;
					if (para.firstChildElement("value").text().toInt(&ok) == 0)
					{
						tempcmd.para2 = 2;
						//printf("��ͷ���ٽ�ֹ\n");
					}
					else
					{
						tempcmd.para2 = 1;
						//printf("��ͷ��������\n");
					}

					t.sendcmd.canid = pubcom.bdnodemap.value(ii);
					t.sendcmd.cmd = tempcmd; 
					t.sendcmd.sendsucceed = false;

					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = ii;
					tempcmd.para1 = 0x05;
					tempcmd.para2 = 0x00;

					t.recvcmd.cmd = tempcmd;
					t.recvcmd.recvsucceed = false;
					pubcom.bdonlineparacmdpair[ii].append(t);
				}


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

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x07;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "̧����ת��ʱ");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x08;
				tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x08;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "���ÿ���ʱ��");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x06;
				tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x06;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "��̨��ʼ����");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x09;
				tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x09;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("machsub1", "̧/����������ʱ");

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x02;
				tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x02;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);


				para = GetParaByName("machsub1", "��/����������ʱ");

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x03;
				tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x03;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("machsub1", "��/���ϸ�������ʱ");

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x04;
				tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x04;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("machsub1", "�ϸ�����������ʱ");

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x05;
				tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x05;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				QString paraname = QString("��ͷ") + QString::number(ii+ 1) + QString("��Ӧ���ñ��");
				para = GetParaByName("machsub3", paraname);

				tempcmd.cmd = SETBUMPNOCMD ;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = ii;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok) - 1;

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETBUMPNOCMD;
				tempcmd.id = ii;
				tempcmd.para1 = ii;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				//ȫ����ɫ��
				tempcmd.cmd = SETPRINTMACHPARACMD ;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x01;
				tempcmd.para2 = pubcom.PrintNum;

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x01;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);
			}
		}

		//����
		QMap<int,QList<tbdonlineparacmdpair> >::iterator it;
		for (it = pubcom.bdonlineparacmdpair.begin(); it != pubcom.bdonlineparacmdpair.end(); ++it)
		{
			QList<tbdonlineparacmdpair>::iterator itt;
			for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
			{
				//����ʧ�ܾͷ���
				if ((*itt).recvcmd.recvsucceed == false)
				{
					if (candevice.sendcmd((*itt).sendcmd.canid,
						&((*itt).sendcmd.cmd)) == false)
					{
						//�����¼��������û�ע��
						publiccaution.addevent(
							pubcom.bdcautionmap.value(it.key()),
							SENDCMDFAILEVENT,"����CAN����ʧ��");
					}
					else
					{
						(*itt).sendcmd.sendsucceed = true;
					}
				}

				//ÿ����������10ms
				Sleep(10);
			}
		}

		//����
		Sleep(50);

		//������
		candevice.refresh();

		for (it = pubcom.bdonlineparacmdpair.begin(); it != pubcom.bdonlineparacmdpair.end(); ++it)
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
		}
	}


	//��ͷ��״̬����������ͺͽ���
	for (int ii = 0; ii < pubcom.PrintNum; ++ii)
	{
		//������ͷ״̬����
		tempcmd.cmd = SETMACHSTATUSCMD;
		tempcmd.id = PCCANBDID;
		tempcmd.para1 = ii;
		tempcmd.para2 = pubcom.presetprintstatus[ii];

		//���ͽ���
		if (candevice.sendcmd(pubcom.bdnodemap.value(ii), &tempcmd) == true)
		{
			Sleep(50); //����50ms

			//������
			candevice.refresh();

			if (candevice.getcmd(ii, SETMACHSTATUSCMD, &tempcmd) == true)
			{
				//���ݷ���ֵ������ͷ״̬
				if (tempcmd.para2 == 0x01)
				{
					pubcom.printstatus[ii] = tempcmd.para1;
					continue;
				}
			}
		}
	}

	
	//ˢ�²���
	{
		bool ok;
		QDomNode para;
		tbdonlineparacmdpair t;
		tcmd tempcmd;

		//��ͷ
		for (int ii = 0; ii < pubcom.PrintNum; ++ii)
		{
			//�������
			pubcom.bdonlineparacmdpair[ii].clear();

			//�������ߵ���ͷ�ŷ��Ͳ���
			if (pubcom.nodeonlinemap.value(ii) == true)
			{
				bool ok;
				QDomNode para;
				tbdonlineparacmdpair t;
				tcmd tempcmd;


				para = GetParaByName("machsub1", "��ͷ��������ť�Ƿ���");
				bool btwork = para.firstChildElement("value").text().toInt(&ok);
				if (btwork == true)
				{

					para = GetParaByName("prod", "��ͷ��������");

					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = 0x05;
					if (para.firstChildElement("value").text().toInt(&ok) == 0)
					{
						tempcmd.para2 = 2;
						printf("��ͷ���ٽ�ֹ\n");
					}
					else
					{
						tempcmd.para2 = 1;
						printf("��ͷ��������\n");
					}

					t.sendcmd.canid = pubcom.bdnodemap.value(ii);
					t.sendcmd.cmd = tempcmd; 
					t.sendcmd.sendsucceed = false;

					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = ii;
					tempcmd.para1 = 0x05;
					tempcmd.para2 = 0x00;

					t.recvcmd.cmd = tempcmd;
					t.recvcmd.recvsucceed = false;
					pubcom.bdonlineparacmdpair[ii].append(t);
				}
			}
		}

		//����
		QMap<int,QList<tbdonlineparacmdpair> >::iterator it;
		for (it = pubcom.bdonlineparacmdpair.begin(); it != pubcom.bdonlineparacmdpair.end(); ++it)
		{
			QList<tbdonlineparacmdpair>::iterator itt;
			for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
			{
				//����ʧ�ܾͷ���
				if ((*itt).recvcmd.recvsucceed == false)
				{
					if (candevice.sendcmd((*itt).sendcmd.canid,
						&((*itt).sendcmd.cmd)) == false)
					{
						//�����¼��������û�ע��
						publiccaution.addevent(
							pubcom.bdcautionmap.value(it.key()),
							SENDCMDFAILEVENT,"����CAN����ʧ��");
					}
					else
					{
						(*itt).sendcmd.sendsucceed = true;
					}
				}

				//ÿ����������10ms
				Sleep(10);
			}
		}

		//����
		Sleep(50);

		//������
		candevice.refresh();

		for (it = pubcom.bdonlineparacmdpair.begin(); it != pubcom.bdonlineparacmdpair.end(); ++it)
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
		}
	}


	//������Ͻ������״̬
	pubcom.machprintstatus = 1;

	pubcom.initflag = 1;

}


//��ʱ���������
void welcom::timeupdate()
{
	if (timercnt < 50)
	{
		timercnt++;
	}
	else if (pubcom.initflag == 1)
	{
		this->accept();
	}
}