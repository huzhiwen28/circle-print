#include <iostream>
#include <QDomNode>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QMessageBox>
#include <QIcon>
#include <QDomNodeList>
#include <QTextStream>
#include <QDomNamedNodeMap>
#include <QReadWriteLock>
#include <QFile>
#include <QTime>

#include <QLabel>
#include <QBitmap>
#include <QPixmap>
#include <QBrush>
#include <QPainter>
#include <QSize>
#include <QFrame>
#include <QLabel>

#include <QtGui/QHeaderView>

#include"opprint.h"
#include "help.h"
#include "canif.h"
#include "selectprint.h"
#include "canif.h"
#include "tech.h"
#include "custevent.h"
#include "CMotor.h"
#include "caution.h"
#include "keyboard.h"
#include "keyboard2.h"
#include "set.h"

//����setting���沿������
extern QSettings settings;
extern QString dlgbkcolor;

extern QString choosestyle2;
extern QString unchoosestyle;
extern QString preparestyle;

extern QDomDocument* doc;
extern char screemsaverflag;
extern long screemsavertime;
extern QPicture topbarpic;
extern com pubcom;

//��ͷ״̬��IO״̬
//extern QLabel* PrintStatus[16][6];

int GetParaRow(const QString& groupname);

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);

//����XML����ȡ��ĳ���е�һ������
const QDomNode GetPara(const QString& groupname, const QString tag,
					   const QString& paraname);

//�¼���Ϣ���ݶ���
extern QObject* backendobject;
extern QObject* interfaceobject;
extern QObject* a4nobject;
extern QObject* selectprintobject;

extern QString dlgbkcolor;

//A4N���
extern CKSMCA4 *ksmc;

QString bubkcolor = QString("background-color:rgb(173,255,47)");
QString bubkcolor_red = QString("background-color:rgb( 255,0,0 )");

selectprint::selectprint(QWidget *parent)
: QDialog(parent)
{
	ui.setupUi(this);
	setStyleSheet(dlgbkcolor); //���öԻ���ı�����ɫ������ɫ�� 

	printstatuschgflag = false;
	ui.cancelButton->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.cancelButton_2->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

}

void selectprint::init()//��ʼ����ͷѡ�����
{
	//QRect t = geometry();

	//setGeometry(QRect(t.left(),t.top(),1024,626));

	textframe = new QFrame( this );//�߿�,���ò�������IO�õ�
	textframe->setGeometry(QRect(670, 20, 340, 531));
	textframe->setFrameShadow(QFrame::Sunken);
	textframe->setFrameShape(QFrame::StyledPanel);
	textframe->setLineWidth(2);

	opprinttext = new QLabel( this );
	opprinttext->setText( "<font size = 5><b>״̬˵����</b><br><br>��ͷ�����ֿ�ѡ״̬������ ׼�� ����<br><br>1�������ߡ���ָԲ������ӡ������ͷ��<br><br>���ֶ�����<br><br>2����׼������ָԲ������ӡ������ͷ��<br><br>    �ֶ�����<br><br>3������������ָ����Բ����ӡ��������<br><br>��ͷ���ֶ�����<br><br><b>����˵����</b><br><br>1������ͷ��ǰ״̬Ϊ�����ߡ�ʱ������<br><br>�Ե����׼������ť���������ͷ<br><br>2������ͷ��ǰ״̬Ϊ��׼����ʱ������<br><br>�Ե������������ť��ʹ��ͷ���й���</font>"  );
	opprinttext->setGeometry(QRect(680, -30, 350, 520));//��������

	table[0] = new QTableWidget( this );
	table[0]->setRowCount( pubcom.PrintNum );
	table[0]->setColumnCount(4);
	table[0]->setGeometry(QRect(40, 20, 600, 32 * ( pubcom.PrintNum + 1 )));
	table[0]->setRowHeight(2, 20);
	table[0]->horizontalHeader()->setStretchLastSection(true);//ȥ�����ж���հ�
	table[0]->verticalHeader()->setStretchLastSection(true);//ȥ�����ж���հ�
	table[0]->horizontalHeader()->setResizeMode(QHeaderView::Stretch);//ʹ���Զ���Ӧ���
	table[0]->verticalHeader()->setResizeMode(QHeaderView::Stretch);//ʹ���Զ���Ӧ���
	table[0]->setStyleSheet("font-size: 18px");

	ctablehead[0] = new QTableWidgetItem("��ͷ״̬");
	table[0]->setHorizontalHeaderItem(0, ctablehead[0]);

	ctablehead[1] = new QTableWidgetItem("״̬ѡ��(����)");
	table[0]->setHorizontalHeaderItem(1, ctablehead[1]);

	ctablehead[2] = new QTableWidgetItem("״̬ѡ��(����)");
	table[0]->setHorizontalHeaderItem(2, ctablehead[2]);

	ctablehead[3] = new QTableWidgetItem("״̬ѡ��(����)");
	table[0]->setHorizontalHeaderItem(3, ctablehead[3]);

	for ( int i = 0; i < pubcom.PrintNum; i++ )
	{
		rtablehead[i] = new QTableWidgetItem((QString("��ͷ") + QString::number(i+1)));
		table[0]->setVerticalHeaderItem(i, rtablehead[i]);

		UIprintstatus[i] = pubcom.presetprintstatus[i];

		switch (pubcom.presetprintstatus[i])
		{
			//����״̬
		case 1:
			{
				lbprintstatus[i] = new QLabel();//�ڱ�������ͼƬ
				lbprintstatus[i]->setPixmap(QPixmap(":/images/sleep.jpg"));
				table[0]->setCellWidget(i , 0 ,lbprintstatus[i]);

				sleeppb[i] = new pb(this);//�ڱ������Ӱ�ť
				sleeppb[i]->setText("����");
				sleeppb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 1 ,sleeppb[i]);
				sleeppb[i]->setStyleSheet( bubkcolor );
				sleeppb[i]->setEnabled(false);
				connect( sleeppb[i], SIGNAL(buttonClicked(int)), this, SLOT(sleep_clicked(int)));

				preparepb[i] = new pb(this);//�ڱ������Ӱ�ť
				preparepb[i]->setText("׼��");
				table[0]->setCellWidget(i , 2 ,preparepb[i]);
				preparepb[i]->setStyleSheet( bubkcolor );
				preparepb[i]->setEnabled(true);
				preparepb[i]->setnum(i+1);
				connect( preparepb[i], SIGNAL(buttonClicked(int)), this, SLOT(prepare_clicked(int)));

				workpb[i] = new pb(this);//�ڱ������Ӱ�ť
				workpb[i]->setText("����");
				table[0]->setCellWidget(i , 3 ,workpb[i]);
				workpb[i]->setStyleSheet( bubkcolor );
				workpb[i]->setEnabled(true);
				workpb[i]->setnum(i+1);
				connect( workpb[i], SIGNAL(buttonClicked(int)), this, SLOT(work_clicked(int)));

				break;
			}

			//׼��״̬
		case 4:
			{
				lbprintstatus[i] = new QLabel();//�ڱ�������ͼƬ
				lbprintstatus[i]->setPixmap(QPixmap(":/images/prepare.jpg"));
				table[0]->setCellWidget(i , 0 ,lbprintstatus[i]);

				sleeppb[i] = new pb(this);//�ڱ������Ӱ�ť
				sleeppb[i]->setText("����");
				table[0]->setCellWidget(i , 1 ,sleeppb[i]);
				sleeppb[i]->setStyleSheet( bubkcolor );
				sleeppb[i]->setEnabled(false);
				sleeppb[i]->setnum(i+1);
				connect( sleeppb[i], SIGNAL(buttonClicked(int)), this, SLOT(sleep_clicked(int)));


				preparepb[i] = new pb(this);//�ڱ������Ӱ�ť
				preparepb[i]->setText("׼��");
				preparepb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 2 ,preparepb[i]);
				preparepb[i]->setStyleSheet( bubkcolor );
				preparepb[i]->setEnabled(false);
				connect( preparepb[i], SIGNAL(buttonClicked(int)), this, SLOT(prepare_clicked(int)));

				workpb[i] = new pb(this);//�ڱ������Ӱ�ť
				workpb[i]->setText("����");
				workpb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 3 ,workpb[i]);
				workpb[i]->setStyleSheet( bubkcolor );
				workpb[i]->setEnabled(true);
				connect( workpb[i], SIGNAL(buttonClicked(int)), this, SLOT(work_clicked(int)));

				break;
			}
			//����״̬
		case 3:
			{
				lbprintstatus[i] = new QLabel();//�ڱ�������ͼƬ
				lbprintstatus[i]->setPixmap(QPixmap(":/images/work.jpg"));
				table[0]->setCellWidget(i , 0 ,lbprintstatus[i]);

				sleeppb[i] = new pb(this);//�ڱ������Ӱ�ť
				sleeppb[i]->setText("����");
				sleeppb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 1 ,sleeppb[i]);
				sleeppb[i]->setStyleSheet( bubkcolor );
				sleeppb[i]->setEnabled(true);
				connect( sleeppb[i], SIGNAL(buttonClicked(int)), this, SLOT(sleep_clicked(int)));

				preparepb[i] = new pb(this);//�ڱ������Ӱ�ť
				preparepb[i]->setText("׼��");
				preparepb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 2 ,preparepb[i]);
				preparepb[i]->setStyleSheet( bubkcolor );
				preparepb[i]->setEnabled(true);
				connect( preparepb[i], SIGNAL(buttonClicked(int)), this, SLOT(prepare_clicked(int)));

				workpb[i] = new pb(this);//�ڱ������Ӱ�ť
				workpb[i]->setText("����");
				workpb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 3 ,workpb[i]);
				workpb[i]->setStyleSheet( bubkcolor );
				workpb[i]->setEnabled(false);
				connect( workpb[i], SIGNAL(buttonClicked(int)), this, SLOT(work_clicked(int)));

				break;
			}
		}
	}
}



selectprint::~selectprint()
{

}

void selectprint::on_cancelButton_clicked()
{
	//��ֹδ�������˳�
	if(printstatuschgflag != true)
	{
		this->accept();
	}
}


//��ͷ����
void selectprint::sleep_clicked(int num)
{
	//ֻ������ͷ׼�����е�ʱ��Ŵ���ť
	if (printstatuschgflag != true)
	{
		interfaceevent* ptempevent = new interfaceevent();

		ptempevent->cmd = 0x07;
		ptempevent->status = 0x00;//����״̬
		ptempevent->data[0] = num-1;//��ͷ���
		ptempevent->data[1] = 1;//1���� 3���� 4׼��

		cmdprintnum = ptempevent->data[0];
		cmdstatus = ptempevent->data[1];

		printstatuschgflag = true;
		//����һ���¼�����̨�߳�
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}

//��ͷ׼��
void selectprint::prepare_clicked(int num)
{
	//ֻ������ͷ׼�����е�ʱ��Ŵ���ť
	if (printstatuschgflag != true)
	{
		interfaceevent* ptempevent = new interfaceevent();

		ptempevent->cmd = 0x07;
		ptempevent->status = 0x00;//����״̬
		ptempevent->data[0] = num-1;//��ͷ���
		ptempevent->data[1] = 4;//1���� 3���� 4׼��

		cmdprintnum = ptempevent->data[0];
		cmdstatus = ptempevent->data[1];

		printstatuschgflag = true;
		//����һ���¼�����̨�߳�
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}
//��ͷ����
void selectprint::work_clicked(int num)
{
	//ֻ������ͷ׼�����е�ʱ��Ŵ���ť
	if (printstatuschgflag != true)
	{
		interfaceevent* ptempevent = new interfaceevent();

		ptempevent->cmd = 0x07;
		ptempevent->status = 0x00;//����״̬
		ptempevent->data[0] = num-1;//��ͷ���
		ptempevent->data[1] = 3;//1���� 3���� 4׼��

		cmdprintnum = ptempevent->data[0];
		cmdstatus = ptempevent->data[1];

		printstatuschgflag = true;
		//����һ���¼�����̨�߳�
		QCoreApplication::postEvent(backendobject, ptempevent);

		//��ͷ����
		QDomNode para = GetParaByName("machsub1", "��ͷ��������ť�Ƿ���");
		bool ok;
		bool btwork = para.firstChildElement("value").text().toInt(&ok);

		if (btwork == true)
		{
			//ֻ������ͷ���ٰ�ť���е�ʱ��Ŵ���ť
			if (pubcom.printchgspeedansflag != true)
			{
				interfaceevent* ptempevent = new interfaceevent();

				ptempevent->cmd = 0x0b;//��ͷ��������
				ptempevent->status = 0x00;//����״̬
				ptempevent->data[0] = 0x00;

				pubcom.singleprintchgspeedid = num-1;

				//����һ���¼��������߳�
				QCoreApplication::postEvent(backendobject, ptempevent);
			}
		}

	}
}

void selectprint::customEvent(QEvent *e)
{
	if (e->type() == BACKEND_EVENT) //�õ��Ǻ�̨���¼�
	{
		backendevent* event = (backendevent*) e;

		//��ͷѡ��,��ͷ��CAN�����
		if (event->cmd == 0x07
			&& event->status == 0x01)
		{
			//������ص�״̬��Ԥ���һ��
			if (event->data[1] == cmdstatus)
			{
				//Ӱ�쵽����
				if ((pubcom.presetprintstatus[cmdprintnum] == 1) 
					&& (cmdstatus == 3 || cmdstatus == 4))
				{
     				pubcom.activeprintlistmutex.lockForWrite();
					pubcom.activeprintlist.append(cmdprintnum);
					pubcom.activeprintlistmutex.unlock();


					ksmc->AddMotor(cmdprintnum + 1);

					//��ʼ�����
					ksmc->InitMotor(cmdprintnum + 1);

					//�������
					ksmc->SetActiveMotor(cmdprintnum + 1);

				}
				else if ((pubcom.presetprintstatus[cmdprintnum] == 3  || pubcom.presetprintstatus[cmdprintnum] == 4) 
					&& (cmdstatus == 1))
				{
					int index = pubcom.activeprintlist.indexOf(cmdprintnum);

					pubcom.activeprintlistmutex.lockForWrite();
					pubcom.activeprintlist.removeAt(index);
    				pubcom.activeprintlistmutex.unlock();

					//ɾ������
					pubcom.delcauflag = true;
					publiccaution.delcaution(
						pubcom.bdcautionmap.value(cmdprintnum));
					pubcom.delcauflag = false;

					//�������Ĳ���Ҫ����Ҫȷ���Ƿǹ���״̬�ſ�������
					ksmc->SetOffMotor(cmdprintnum + 1);
					ksmc->DelMotor(cmdprintnum + 1);
				}

				//Ԥ��״̬�ı�
				pubcom.presetprintstatus[cmdprintnum] = cmdstatus;

				//��ͷ״̬�ı�
				pubcom.printstatus[cmdprintnum] = cmdstatus;

				//���浽XML
				QString paraname = QString("��ͷ") + QString::number(
					cmdprintnum + 1) + QString("״̬");

				//��ͷ׼��״̬�޸�
				QDomNode paranode = GetParaByName("print", paraname);
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(cmdstatus));

				//�����޸�
				switch(cmdstatus) {
				case 1:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/sleep.jpg"));
					sleeppb[cmdprintnum]->setEnabled(false);
					workpb[cmdprintnum]->setEnabled(true);
					preparepb[cmdprintnum]->setEnabled(true);
					break;

				case 3:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/work.jpg"));
					sleeppb[cmdprintnum]->setEnabled(true);
					workpb[cmdprintnum]->setEnabled(false);
					preparepb[cmdprintnum]->setEnabled(true);
					break;

				case 4:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/prepare.jpg"));
					sleeppb[cmdprintnum]->setEnabled(false);
					workpb[cmdprintnum]->setEnabled(true);
					preparepb[cmdprintnum]->setEnabled(false);
					break;
				}

				//����XML�ļ�
				QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
				if (!file.open(QFile::WriteOnly))
				{
					QMessageBox msgBox;
					msgBox.setText("���ļ�aaa.xmlʧ��");
					msgBox.setWindowTitle("����");
					msgBox.setStandardButtons(QMessageBox::Yes );
					QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
					tb->setText("ȷ��");
					msgBox.exec();
					printstatuschgflag = false;

					return;
				}

				QTextStream out(&file);
				doc->save(out, 4);
			}
			else
			{
				//��ʾ��ǰ�����޸�
				QMessageBox msgBox;
				msgBox.setText("״̬����ʧ��");
				msgBox.setWindowTitle("ʧ��");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

			}

			printstatuschgflag = false;
		}
		//��ͷû��CAN�������ǿ�иı�״̬
		else if (event->cmd == 0x07
			&& event->status == 0x02)
		{
			//Ӱ�쵽���ã�ֻ��ǿ�����߲�������(���������ͷ)��ǿ�д������ͷû��Ӧ���û��������
			if ((pubcom.presetprintstatus[cmdprintnum] == 3  || pubcom.presetprintstatus[cmdprintnum] == 4) 
				&& (cmdstatus == 1))
			{
				int index = pubcom.activeprintlist.indexOf(cmdprintnum);

				pubcom.activeprintlistmutex.lockForWrite();
				pubcom.activeprintlist.removeAt(index);
				pubcom.activeprintlistmutex.unlock();

				//ɾ������
				pubcom.delcauflag = true;
				publiccaution.delcaution(
					pubcom.bdcautionmap.value(cmdprintnum));
				pubcom.delcauflag = false;

				//�������Ĳ���Ҫ����Ҫȷ���Ƿǹ���״̬�ſ�������
				ksmc->SetOffMotor(cmdprintnum + 1);
				ksmc->DelMotor(cmdprintnum + 1);

				//Ԥ��״̬�ı�
				pubcom.presetprintstatus[cmdprintnum] = cmdstatus;

				//���浽XML
				QString paraname = QString("��ͷ") + QString::number(
					cmdprintnum + 1) + QString("״̬");

				//��ͷ׼��״̬�޸�
				QDomNode paranode = GetParaByName("print", paraname);
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(cmdstatus));

				//�����޸�
				switch(cmdstatus) {
				case 1:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/sleep.jpg"));
					sleeppb[cmdprintnum]->setEnabled(false);
					workpb[cmdprintnum]->setEnabled(true);
					preparepb[cmdprintnum]->setEnabled(true);
					break;

				case 3:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/work.jpg"));
					sleeppb[cmdprintnum]->setEnabled(true);
					workpb[cmdprintnum]->setEnabled(false);
					preparepb[cmdprintnum]->setEnabled(true);
					break;

				case 4:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/prepare.jpg"));
					sleeppb[cmdprintnum]->setEnabled(false);
					workpb[cmdprintnum]->setEnabled(true);
					preparepb[cmdprintnum]->setEnabled(false);
					break;
				}

				//����XML�ļ�
				QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
				if (!file.open(QFile::WriteOnly))
				{
					QMessageBox msgBox;
					msgBox.setText("���ļ�aaa.xmlʧ��");
					msgBox.setWindowTitle("����");
					msgBox.setStandardButtons(QMessageBox::Yes );
					QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
					tb->setText("ȷ��");
					msgBox.exec();

					printstatuschgflag = false;

					return;
				}

				QTextStream out(&file);
				doc->save(out, 4);

				//��ʾ��ǰ�����޸�
				QMessageBox msgBox;
				msgBox.setText("״̬ǿ�����óɹ�");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

			}

			else if ((pubcom.presetprintstatus[cmdprintnum] == 1) 
				&& (cmdstatus == 3 || cmdstatus == 4))
			{
				pubcom.activeprintlistmutex.lockForWrite();
				pubcom.activeprintlist.append(cmdprintnum);
				pubcom.activeprintlistmutex.unlock();

				ksmc->AddMotor(cmdprintnum + 1);

				//��ʼ�����
				ksmc->InitMotor(cmdprintnum + 1);

				//�������
				ksmc->SetActiveMotor(cmdprintnum + 1);

				//Ԥ��״̬�ı�
				pubcom.presetprintstatus[cmdprintnum] = cmdstatus;

				//���浽XML
				QString paraname = QString("��ͷ") + QString::number(
					cmdprintnum + 1) + QString("״̬");

				//��ͷ׼��״̬�޸�
				QDomNode paranode = GetParaByName("print", paraname);
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(cmdstatus));
				//�����޸�
				switch(cmdstatus) {
				case 1:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/sleep.jpg"));
					sleeppb[cmdprintnum]->setEnabled(false);
					workpb[cmdprintnum]->setEnabled(true);
					preparepb[cmdprintnum]->setEnabled(true);
					break;

				case 3:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/work.jpg"));
					sleeppb[cmdprintnum]->setEnabled(true);
					workpb[cmdprintnum]->setEnabled(false);
					preparepb[cmdprintnum]->setEnabled(true);
					break;

				case 4:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/prepare.jpg"));
					sleeppb[cmdprintnum]->setEnabled(false);
					workpb[cmdprintnum]->setEnabled(true);
					preparepb[cmdprintnum]->setEnabled(false);
					break;
				}

				//����XML�ļ�
				QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
				if (!file.open(QFile::WriteOnly))
				{
					QMessageBox msgBox;
					msgBox.setText("���ļ�aaa.xmlʧ��");
					msgBox.setWindowTitle("����");
					msgBox.setStandardButtons(QMessageBox::Yes );
					QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
					tb->setText("ȷ��");
					msgBox.exec();

					printstatuschgflag = false;

					return;
				}

				QTextStream out(&file);
				doc->save(out, 4);

				//��ʾ��ǰ�����޸�
				QMessageBox msgBox;
				msgBox.setText("״̬ǿ�����óɹ�");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

			}
			else
			{
				//Ԥ��״̬�ı�
				pubcom.presetprintstatus[cmdprintnum] = cmdstatus;

				//���浽XML
				QString paraname = QString("��ͷ") + QString::number(
					cmdprintnum + 1) + QString("״̬");

				//��ͷ׼��״̬�޸�
				QDomNode paranode = GetParaByName("print", paraname);
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(cmdstatus));

				//�����޸�
				switch(cmdstatus) {
				case 1:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/sleep.jpg"));
					sleeppb[cmdprintnum]->setEnabled(false);
					workpb[cmdprintnum]->setEnabled(true);
					preparepb[cmdprintnum]->setEnabled(true);
					break;

				case 3:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/work.jpg"));
					sleeppb[cmdprintnum]->setEnabled(true);
					workpb[cmdprintnum]->setEnabled(false);
					preparepb[cmdprintnum]->setEnabled(true);
					break;

				case 4:
					lbprintstatus[cmdprintnum]->setPixmap(QPixmap(":/images/prepare.jpg"));
					sleeppb[cmdprintnum]->setEnabled(false);
					workpb[cmdprintnum]->setEnabled(true);
					preparepb[cmdprintnum]->setEnabled(false);
					break;
				}

				//����XML�ļ�
				QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
				if (!file.open(QFile::WriteOnly))
				{
					QMessageBox msgBox;
					msgBox.setText("���ļ�aaa.xmlʧ��");
					msgBox.setWindowTitle("����");
					msgBox.setStandardButtons(QMessageBox::Yes );
					QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
					tb->setText("ȷ��");
					msgBox.exec();
					printstatuschgflag = false;

					return;
				}

				QTextStream out(&file);
				doc->save(out, 4);

				QMessageBox msgBox;
				msgBox.setText("״̬ǿ�����óɹ�");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
			}
			/*
			else
			{
			//��ʾ��ǰ�����޸�
			QMessageBox::warning(this, tr("ʧ��"), tr("״̬����ʧ��"),
			QMessageBox::Yes );
			}
			*/

			printstatuschgflag = false;
		}
		//��ǰ����������
		else if (event->cmd == 0x07
			&& event->status == 0x03)
		{
			printstatuschgflag = false;
			QMessageBox msgBox;
			msgBox.setText("�Ѿ��в����ڽ���");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
		}

	}
}

//�ػ���ť
void selectprint::on_cancelButton_2_clicked()
{
	keyboard keyboardinst;
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//ȡ��canel
	if (keyboardinst.flag == 1)
	{
		;
	}//ȷ��
	else if (keyboardinst.flag == 2)
	{
		//����
		if (settings.value("prod/pass").toString() == keyboardinst.password
			&& !settings.value("prod/pass").toString().isNull())
		{
			pubcom.PowerOff = true;
			this->accept();
		}//����
		else if (settings.value("mach/pass").toString()
			== keyboardinst.password
			&& !settings.value("mach/pass").toString().isNull())
		{
			pubcom.PowerOff = true;
			this->accept();

		}//����
		else if (settings.value("tune/pass").toString()
			== keyboardinst.password
			&& !settings.value("tune/pass").toString().isNull())
		{
			pubcom.PowerOff = true;
			this->accept();

		}//һ�����
		else if (keyboardinst.password == BACKDOORPASS )
		{
			pubcom.PowerOff = true;
			this->accept();

		}//��������
		else if (keyboardinst.password == DEVPASS)
		{
			pubcom.PowerOff = true;
			this->accept();
		}
		//���벻��ȷ����ͨ�û��޷�����
		else
		{
			return;
		}
	}
}