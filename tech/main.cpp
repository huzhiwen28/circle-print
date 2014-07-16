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

#include "tech.h"

#include <QtGui>
#include <QApplication>
#include <QFile>
#include <QObject>
#include <iostream>
#include <QDomNode>
#include <QSettings>
#include <QStyle>
#include <QPlastiqueStyle>
#include <QWindowsStyle>
#include <QWindowsXPStyle>
#include <QCleanlooksStyle>
#include "welcom.h"
#include "canif.h"
#include "backend.h"
#include "backend2.h"
#include "DBBackend.h"

#include <QtSql>
#include <QTableView>
#include <QSqlTableModel>

#include "CMotor.h"
#include "DB.h"

#include "MemExport.h"
#include "set.h"
#include "password.h"
#include "opprint.h"
#include "caution.h"
#include "serialif.h"
#include "measurepulse.h"
#include "pcfingerprint.h"
#include "safenet.h"
#include "modbus.h"

double f_follow = 0.0;

QSettings settings("HTK", "cicle print");
const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraname);
QDomDocument* doc;

extern QString dlgbkcolor;
extern long screentimer;
canif candevice;
struct _modbusRTU modbusRTU;
struct _regs regs;

//���ܹ����
safenet safenetinst;

//�¼���Ϣ���ݶ���
QObject* backendobject;
QObject* interfaceobject;
QObject* a4nobject;
QObject* selectprintobject;
QObject* measurepulseobject;
tech* ptech;

//A4N���
CKSMCA4 *ksmc;

DB mydb;


extern QObject* setobject;
extern QObject* opobject;

//ָ��set�ĶԻ���
extern set* pDlgset;

//ָ����ͷ�����ĶԻ���
extern opprint* pDlgop;

//��ʱ�����������պ���
void WINAPI onTimeFunc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2)
{

}

//��������ʹ�õ��¼�������
KeyPressEater::KeyPressEater(QObject *parent) :
QObject(parent)
{

}
KeyPressEater::~KeyPressEater()
{

}

bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
	//��갴���¼�
	if ( event->type() == QEvent::MouseButtonDblClick
		|| event->type() == QEvent::MouseButtonPress
		|| event->type() == QEvent::MouseButtonRelease
		|| event->type() == QEvent::MouseMove)
	{
		screentimer = 0;
		return QObject::eventFilter(obj, event);
	}
	
#ifndef TEST
	else if(event->type() == QEvent::KeyPress)//���ˡ�esc������
	{
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		if (ke->key() == Qt::Key_Escape) {
			qDebug("esc");
			return true;
		}
		return QObject::eventFilter(obj, event);
	}
#endif
	
	else
	{
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}


int main(int argc, char *argv[])
{
	//����3��
	Sleep(3000);

	//QApplication::setStyle(new QCleanlooksStyle);
	QApplication a(argc, argv);

	//����õ�λ�ã������Ϊ�˱��ڳ�������DLL
	QStringList ttt = a.libraryPaths();
	ttt.append(QCoreApplication::applicationDirPath());
	ttt.append(QCoreApplication::applicationDirPath() + QString("/plugins"));
	ttt.append(QCoreApplication::applicationDirPath() + QString("/kerneldlls"));
	a.setLibraryPaths(ttt);

	KeyPressEater *pkeyPressEater = new KeyPressEater();
	a.installEventFilter(pkeyPressEater);

	a.setStyleSheet("QTableWidget  { color: black }");
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("GBK"));


#ifdef HAVEDOG
	if (safenetinst.open() != 0)
	{
		/*
		���δ�ҵ�����������Ŀ����� ID �����֤ ID ��Ӳ��������û����Ч�����֤��
		SFNTGetLicense �����ش���
		*/ 
		QMessageBox msgBox;
		msgBox.setText("û�м�⵽���ܹ���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;

	}

	if (safenetinst.AESreqack() != 0)
	{
		/*
		���δ�ҵ�����������Ŀ����� ID �����֤ ID ��Ӳ��������û����Ч�����֤��
		SFNTGetLicense �����ش���
		*/ 
		QMessageBox msgBox;
		msgBox.setText("û�м�⵽���ܹ���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;
	}


	//���������к�
	char fingerfrombios[40];
	char fingerfromdog[40];

	memset(fingerfrombios,0,40);
	memset(fingerfromdog,0,40);

	int fingerfrombioslen = 0;
	int fingerfromdoglen = 0;

	//ȡ����ʵ��ֵ
	pcfingerprint fingerprintinst;
	if (fingerprintinst.getpcfingerprint(fingerfrombios,&fingerfrombioslen) != 0)
	{
		QMessageBox msgBox;
		msgBox.setText("���������кŴ���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;

	}

	char licdate[20];
	memset(licdate,0,20);

	//�鿴���ܹ���ԭ��������
	if (safenetinst.readdata(licdate,0,9) == 0)
	{
		//����û������
		if ( licdate[0] == '1'
			&& licdate[1] == '1'
			&&licdate[2] == '1'
			&&licdate[3] == '1'
			&&licdate[4] == '1'
			&&licdate[5] == '1'
			&&licdate[6] == '1'
			&&licdate[7] == '1'
			&&licdate[8] == '1')
		{
			memset(licdate,0,20);

			//ȡ���ܹ�����
			int year,month,day;
			safenetinst.GetDogDate(&year,&month,&day);
			QDate newdate(year+1,month,day);

			//��һ��������������
			licdate[0] = '1';
			strncpy(licdate+1,newdate.toString("yyyyMMdd").toAscii(),8);

			//д����
			if (safenetinst.writedata(licdate,0,9) != 0)
			{
				QMessageBox msgBox;
				msgBox.setText("д���ܹ�����");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return 1;

			}

		}

	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText("�����ܹ�����");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;

	}

	//�鿴���ܹ�ԭ����ֵ���к�
	if (safenetinst.readdata(fingerfromdog,10,6) == 0)
	{
		//��һ������,���к�û������
		if (fingerfromdog[0] == '1'
			&& fingerfromdog[1] == '1'
			&& fingerfromdog[2] == '1'
			&& fingerfromdog[3] == '1'
			&& fingerfromdog[4] == '1'
			&& fingerfromdog[5] == '1'
			)
		{

			//���챣�浽���ܹ��Ĵ�����һ���ֽ�Ϊ���ȣ������ֽ�Ϊ����ֵ
			fingerfromdog[0] = (unsigned char) fingerfrombioslen;
			for (int i = 0;i < fingerfrombioslen; ++i)
			{
				fingerfromdog[i+1] = fingerfrombios[i];
			}
			//��ֵ���浽���ܹ�
			fingerfromdoglen = fingerfrombioslen +1;
			if (safenetinst.writedata(fingerfromdog,9,fingerfromdoglen) != 0)
			{
				QMessageBox msgBox;
				msgBox.setText("д���ܹ�����");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return 1;

			}

		} 
		else
		{
			//�ǵ�һ�����У�ȡ��ֵ�ͻ�����ʵ��ֵ�ȶ�
			if (safenetinst.readdata(fingerfromdog,9,fingerfrombioslen + 1) == 0)
			{
				bool sameflag = true;
				//���ȱȶ�
				if (fingerfromdog[0] == (unsigned char)fingerfrombioslen)
				{
					//�Ƚ�����
					for (int i = 0; i< fingerfrombioslen; ++i)
					{
						if (fingerfromdog[i+1] != fingerfrombios[i])
						{
							sameflag = false;
							break;
						}
					}
				}
				else
				{
					sameflag = false;
				}

				if (sameflag == false)
				{
					QMessageBox msgBox;
					msgBox.setText("���֤����");
					msgBox.setWindowTitle("����");
					msgBox.setStandardButtons(QMessageBox::Yes );
					QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
					tb->setText("ȷ��");
					msgBox.exec();

					return 1;
				}
			}
		}
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText("�����ܹ�����");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;
	}
#endif



	//�ļ��Ƿ���ȫXMLģ����Ӧ��������ſ�ʼ�ϵñ���
	QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
	if (!file.exists())
	{
		QMessageBox msgBox;
		msgBox.setText("�Ҳ��������ļ�");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;
	}

	doc = new QDomDocument("mydocument");
	if (!file.open(QIODevice::ReadWrite))
	{
		QMessageBox msgBox;
		msgBox.setText("�����ļ���ʧ��");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;
	}

	QString errorMsg;
	int errline;
	int errcol;

	if (!doc->setContent(&file, &errorMsg, &errline, &errcol))
	{
		std::cout << "errline,errcol:" << errline << "," << errcol;
		QMessageBox msgBox;
		msgBox.setText(errorMsg);
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		file.close();
		return 1;
	}
	file.close();

	QDomNode para;

	//ȡ����ͷ����
	para = GetParaByName("tune", "ȫ����ɫ��");
	bool ok;
	pubcom.PrintNum = para.firstChildElement("value").text().toInt(&ok);

	//ȫ�ֱ�����ʼ��������ȫ�ֱ���������XML�ļ������Ա������XML��ʼ������
	pubcom.init();

	//���ݿ��ʼ��
	if (mydb.init() == false)
	{
		QMessageBox msgBox;
		msgBox.setText("�Ҳ�����¼�ļ�");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;
	}

	//��̨��ʼ��
	DBBackend DBBackendinst;
	DBBackendinst.start();


	//�����Ƿ���ڣ���������ڣ���ôʹ��Ĭ�������Ȩ��
	if (settings.value("prod/pass").toString().isNull())
	{
		para = GetParaByName("defaultright", "����");
		if (para.isNull())
		{
			publiccaution.adddbginfo("���� para.isNull()");
			return 1;
		}
		settings.setValue("prod/pass", para.firstChildElement("pass").text());
		settings.setValue("prod/modright",
			para.firstChildElement("userright").text());
	}
	if (settings.value("mach/pass").toString().isNull())
	{
		para = GetParaByName("defaultright", "����");
		if (para.isNull())
		{
			publiccaution.adddbginfo("���� para.isNull()");
			return 1;
		}
		settings.setValue("mach/pass", para.firstChildElement("pass").text());
		settings.setValue("mach/modright",
			para.firstChildElement("userright").text());
	}
	if (settings.value("tune/pass").toString().isNull())
	{
		para = GetParaByName("defaultright", "����");
		if (para.isNull())
		{
			publiccaution.adddbginfo("���� para.isNull()");
			return 1;
		}
		settings.setValue("tune/pass", para.firstChildElement("pass").text());
		settings.setValue("tune/modright",
			para.firstChildElement("userright").text());
	}

	if (settings.value("screensaver/flag").toString().isNull())
	{
		para = GetParaByName("screensaver", "����");
		if (para.isNull())
		{
			publiccaution.adddbginfo("���� para.isNull()");
			return 1;
		}
		settings.setValue("screensaver/flag", para.firstChildElement("default").text());

		para = GetParaByName("screensaver", "�ȴ�ʱ��");
		if (para.isNull())
		{
			publiccaution.adddbginfo("�ȴ�ʱ�� para.isNull()");
			return 1;
		}
		settings.setValue("screensaver/time",
			para.firstChildElement("default").text());
	}


	//modbus ���ڲ�������
	para = GetParaByName("dev", "����ѡ��");
	modbusRTU_slave_SetSerialName(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "����ͨѶ����");
	modbusRTU_slave_SetSerialBaudRate(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "����У�鷽ʽ");
	modbusRTU_slave_SetSerialParity(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "�����ֽ�λ��");
	modbusRTU_slave_SetSerialByteSize(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "����ֹͣλ");
	modbusRTU_slave_SetSerialStopBits(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "RTS������");
	modbusRTU_slave_SetSerialfRtsControl(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "CANʧ�ܴ�����Ϊ��������");
	pubcom.canfailedcnt = para.firstChildElement("value").text().toInt(&ok);

	//modbus��������
	para = GetParaByName("modbus", "ID���");
	//����modbus id
	set_modbusRTU_slave_ModbusID(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("modbus", "1.5Tʱ��");
	//����1.5Tʱ�䶨��
	set_modbusRTU_slave_t15(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("modbus", "3.5Tʱ��");
	//����3.5Tʱ�䶨��
	set_modbusRTU_slave_t35(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("modbus", "Ӧ��ȴ�ʱ��");
	//����Ӧ����ʱ��
	set_modbusRTU_slave_answaittime(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	memset(&(modbusRTU.ov1),0,sizeof(OVERLAPPED));
	memset(&(modbusRTU.ov2),0,sizeof(OVERLAPPED));
	memset(&(modbusRTU.ov3),0,sizeof(OVERLAPPED));

	modbusRTU.ov1.hEvent = CreateEvent(NULL,true,false,NULL);
	modbusRTU.ov2.hEvent = CreateEvent(NULL,true,false,NULL);
	modbusRTU.ov3.hEvent = CreateEvent(NULL,true,false,NULL);


	//��ͷ״ֵ̬�ĸ�λ
	for(int i = 0; i < 16; ++i)
	{
		int x,y,z;
		mydb.GetValue(QString("x") + QString::number(i) ,x);
		mydb.GetValue(QString("y") + QString::number(i) ,y);
		mydb.GetValue(QString("z") + QString::number(i) ,z);
		pubcom.xstatus[i] = x;
		pubcom.ystatus[i] = y;
		pubcom.zstatus[i] = z;
	}

	//CAN����
	if ( candevice.open() == false)
	{
		QMessageBox msgBox;
		msgBox.setText("CANͨѶ������ʧ��");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		publiccaution.addcaution(PCCAUTIONLOCATION,PCNOCANCATION);
		return 1;
	}

	//����˶�����������
	ksmc = new CKSMCA4;
#ifdef HAVEA4NCARD

	//�����ܴ�
	if(pubcom.cardok == false)
	{
		QMessageBox msgBox;
		msgBox.setText("�˶�����ʧ��,�����´򿪳���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return 1;
	}
#endif
	//������ͷ�����������
	for (int ii = 0; ii < pubcom.PrintNum; ii++)
	{
		if (pubcom.activeprintlist.contains(ii) == true)
		{
			ksmc->AddMotor(ii + 1);
		}
	}

	para = GetParaByName("prod", "�����ٶ�");
	ksmc->SetHMSpeed(para.firstChildElement("value").text().toInt(&ok));

	//���ò���
	para = GetParaByName("prod", "���������ٶ�");
	ksmc->SetHMSpeedHigh(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("prod", "�Ի��ٶ�");
	ksmc->SetJogSpeed(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("prod", "���ٶԻ��ٶ�");
	ksmc->SetJogSpeedHigh(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("tune", "��ͷ�ܳ�");
	ksmc->SetGirth(para.firstChildElement("value").text().toFloat(&ok));

	para = GetParaByName("tune", "�����ͺ���");
	pubcom.Follow = para.firstChildElement("value").text().toFloat(&ok)*0.01;
	ksmc->SetFollowDelay(pubcom.Follow);

	para = GetParaByName("tune", "��ͷ�������");
	ksmc->SetOffset(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("machsub1", "ȫ�����������");
	ksmc->SetMaxSpeed(para.firstChildElement("value").text().toInt(&ok)*1000);

	para = GetParaByName("machsub1", "̧����ת�ٶȼ��������");
	ksmc->SetCCWJogSpeed(para.firstChildElement("value").text().toInt(&ok));

	ksmc->SetOutPut(-1, 0);

	para = GetParaByName("machsub1", "��������ѡ����");
	ksmc->SetPulseDirect(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("tune", "�������ֱ���");
	ksmc->SetPulsePerCircle(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("tune", "Ħ��ϵ��");
	ksmc->SetPulserPerMeter(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "�ŷ������������������");
	ksmc->SetToothNum1(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "���������ź��˲�����");
	ksmc->SetFollowFilter(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "�ŷ�����������ٱ�");
	ksmc->SetServGearBoxScale(para.firstChildElement("value").text().toDouble(&ok));

	para = GetParaByName("tune", "�ŷ��������תѡ��");
	pubcom.ServoDir = para.firstChildElement("value").text().toInt(&ok);

	para = GetParaByName("tune", "��ͷ����");
	ksmc->SetToothNum2(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("machsub1", "�Ͻ���̨��Ӧ��̨���");
	pubcom.Shangjiangcitaibianhao = para.firstChildElement("value").text().toInt(&ok);

	int paraVal = 0;

	//��������������
	para = GetParaByName("driver", "��һλ������");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			//ksmc->SetDriverPar(index+1,16,paraVal);
			ksmc->SetDriverPar(index+1,16,10);

		}
	}

	para = GetParaByName("driver", "��һ�ٶ�����");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			//ksmc->SetDriverPar(index+1,17,paraVal);
			ksmc->SetDriverPar(index+1,17,10);

		}
	}

	para = GetParaByName("driver", "��һ�ٶȻ���ʱ�䳣��");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			//ksmc->SetDriverPar(index+1,18,paraVal);
			ksmc->SetDriverPar(index+1,18,10);

		}
	}

	para = GetParaByName("driver", "��һ�ٶȼ�����");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			//ksmc->SetDriverPar(index+1,19,paraVal);
			ksmc->SetDriverPar(index+1,19,5);
		}
	}

	para = GetParaByName("driver", "��һŤ�ع���ʱ�䳣��");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,20,paraVal);

		}
	}

	para = GetParaByName("driver", "�ڶ�λ������");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,24,paraVal);

		}
	}

	para = GetParaByName("driver", "�ڶ��ٶ�����");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,25,paraVal);

		}
	}

	para = GetParaByName("driver", "�ڶ��ٶȻ���ʱ�䳣��");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,26,paraVal);

		}
	}

	para = GetParaByName("driver", "�ڶ��ٶȼ�����");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,27,paraVal);

		}
	}

	para = GetParaByName("driver", "�ڶ�Ť�ع���ʱ�䳣��");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,28,paraVal);

		}
	}

	para = GetParaByName("driver", "�ڶ���������");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,48,paraVal);

		}
	}

	para = GetParaByName("driver", "�����л�ģʽ");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,49,paraVal);

		}
	}

	para = GetParaByName("driver", "�����л��ӳ�ʱ��");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,50,paraVal);

		}
	}
	para = GetParaByName("driver", "�����л�ˮƽ");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,51,paraVal);

		}
	}
	para = GetParaByName("driver", "�����л��ͺ�");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,52,paraVal);

		}
	}

	para = GetParaByName("driver", "λ�û������л�ʱ��");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//����Ϊ0���ʾ����ͨ����������
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,53,paraVal);

		}
	}

	//��ʼ�����
	ksmc->InitMotor(-1);

	//�������
	ksmc->SetActiveMotor(-1);

	ksmc->StartPLC();

	para = GetParaByName("prod", "���ٵ����ٶԻ�ʱ��");
	pubcom.LowToHighSpeedtime = para.firstChildElement("value").text().toInt(&ok);


	//����ֵˢ�µ��Ĵ�����
	initRegs(&regs);

	//PLCIO����
	para = GetParaByName("plcio", "���ػ���������λλ��");
	SetPcEnableStartBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ���������λλ��");
	SetPcMotorAccBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ���������λλ��");
	SetPcMotorDecBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ�����λλ��");
	SetPcErrBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ�����λλ��");
	SetPcCauBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ�����̺����λλ��");
	SetPcMotorOnBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ���ֹ̺ͣλλ��");
	SetPcMotorOffBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ���������λλ��");
	SetPcWorkFinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ���������λλ��");
	SetPcSetHeartBeatBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "���ػ�����ˢ������λλ��");
	SetPcFlushDataReqBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC������������λλ��");
	SetPlcStartReqBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC����ֹͣλλ��");
	SetPlcStopBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC����λλ��");
	SetPlcErrBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC����λλ��");
	SetPlcCauBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC��������λλ��");
	SetPlcJinbuyunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLCӡ����������λλ��");
	SetPlcYinhuadaodaiyunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC�淿����1����λλ��");
	SetPlcHongfangwangdai1yunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC�淿����2����λλ��");
	SetPlcHongfangwangdai2yunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC�䲼����λλ��");
	SetPlcLuobuyunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC������������λλ��");
	SetPlcGongjiangcilikaiguanBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));


	//PLCPARA����
	para = GetParaByName("plcpara", "���ػ����IO��λ��");
	SetPcOutIOWordIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcpara", "���ػ�����˫��λ��");
	SetPcWorkRecDWordIndex(&regs,para.firstChildElement("value").text().toInt(&ok));


	//����IO����Ļص���������
	SetPlcStartReqFp(&regs,PlcStartReqHook);
	SetPlcStopFp(&regs,PlcStopHook);
	SetPlcErrFp(&regs,PlcErrHook);
	SetPlcCauFp(&regs,PlcCauHook);
	SetPlcJinbuyunxinFp(&regs,PlcJinbuyunxinHook);
	SetPlcYinhuadaodaiyunxinFp(&regs,PlcYinhuadaodaiyunxinHook);
	SetPlcHongfangwangdai1yunxinFp(&regs,PlcHongfangwangdai1yunxinHook);
	SetPlcHongfangwangdai2yunxinFp(&regs,PlcHongfangwangdai2yunxinHook);
	SetPlcLuobuyunxinFp(&regs,PlcLuobuyunxinHook);
	SetPlcGongjiangcilikaiguanFp(&regs,PlcGongjiangcilikaiguanHook);
	SetPlcGongjiangcilidaxiaogaibianFp(&regs,PlcGongjiangcilidaxiaogaibianHook);

	welcom welcominst;
#ifdef ONIPC
	welcominst.setWindowState( Qt::WindowFullScreen );
#endif
	welcominst.setWindowFlags(Qt::FramelessWindowHint);

	tech w;
	w.init();
	interfaceobject = &w;
	ptech = &w;
	w.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
	w.setWindowState(Qt::WindowFullScreen);
#endif

	w.setWindowFlags(Qt::FramelessWindowHint);
	w.show();

	welcominst.exec();

    //��ý�嶨ʱ����Ϊ����߹��ػ��̵߳��Ⱦ��ȶ����룬΢���ڲ�����ϣ����Ĳ�ȷ����
	MMRESULT timer_id;
	timer_id = timeSetEvent(1, 1, (LPTIMECALLBACK)onTimeFunc, DWORD(1), TIME_PERIODIC);

	//��̨�̣߳��ֱ��������ȼ�
	backend backendinst;
	backendinst.start();
	backendinst.setPriority(QThread::HighestPriority);
	backendobject = &backendinst;

	backend2 backend2inst;
	backend2inst.start();
    backend2inst.setPriority(QThread::HighestPriority);

	modbusmain modbusmaininst(&modbusRTU);
	modbusmaininst.start();
	modbusmaininst.setPriority(QThread::TimeCriticalPriority);

#ifdef HAVESERIAL

	//�򿪴���
	if (modbusRTU_slave_SerialOpen(&modbusRTU) == false)
	{
		QMessageBox msgBox;
		msgBox.setText("���ڴ�ʧ��");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		publiccaution.addcaution(SERIAL,SERIALNOEXIST);
	}

#endif
	a.exec();

#ifdef HAVESERIAL
	//�رմ���
	modbusRTU_slave_SerialClose(&modbusRTU);
	Sleep(100);
#endif

	//�ͷ��¼��ں˶���
	CloseHandle(modbusRTU.ov1.hEvent);
	CloseHandle(modbusRTU.ov2.hEvent);
	CloseHandle(modbusRTU.ov3.hEvent);

	//ȷ������ˢ�µ����ݿ��ļ���
	while ((mydb.SQLQueuechg == true) 
		|| (mydb.SQLQueuechg2 == true))
	{
		;
	}
	printf("database flush OK\n");

	pubcom.quitthread = true;

	modbusmaininst.wait();
	printf("modbusmaininst stop OK\n");

	backendinst.wait();
	printf("modbusmaininst stop OK\n");

	backend2inst.wait();
	printf("backend2inst stop OK\n");

	DBBackendinst.wait();
	printf("DBBackendinst stop OK\n");

	//�ͷŶ�ʱ��
	timeKillEvent(timer_id);        

#ifdef POWEROFF
	//�ػ�
	if (pubcom.PowerOff == true)
	{
		HANDLE hToken;
		TOKEN_PRIVILEGES tkp;
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (GetVersionEx(&osvi) == 0)
			return false;
		// Determine the platform
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			// Windows NT 3.51, Windows NT 4.0, Windows 2000,
			// Windows XP, or Windows .NET Server
			if (!OpenProcessToken(GetCurrentProcess(),
				TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
				return false;
			LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
			tkp.PrivilegeCount = 1; // one privilege to set
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
		}
		ExitWindowsEx(EWX_SHUTDOWN,0);
	}
#endif

	return  0;
}

