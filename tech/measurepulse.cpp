#include "measurepulse.h"
#include <iostream>
#include <QRegExp>
#include <QMessageBox>
#include <QDomNode>
#include <QTextStream>
#include <QDomNamedNodeMap>
#include <QFile>
#include <QTableWidgetItem>
#include "CMotor.h"
#include "caution.h"
#include "serialif.h"
#include "custevent.h"
#include "keyboard2.h"

extern serialif serialport;

//A4N���
extern CKSMCA4 *ksmc;
extern QDomDocument* doc;
extern QTableWidgetItem* FricationNodeItem;
extern QObject* backendobject;

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);

measurepulse::measurepulse(QWidget *parent)
: QDialog(parent)
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","����Ħ��ϵ������ҳ��","�û�����Ħ��ϵ������ҳ��",1);

	ui.setupUi(this);

	lineEdit[0] = new le(ui.groupBox);
	lineEdit[0]->setnum(0);
	lineEdit[0]->setGeometry(QRect(290,20,113,31));
	connect(lineEdit[0],SIGNAL(leClicked(int)),this,SLOT(lineEdit_clicked(int)));

	lineEdit[1] = new le(ui.groupBox_2);
	lineEdit[1]->setnum(1);
	lineEdit[1]->setGeometry(QRect(290,20,113,31));
	connect(lineEdit[1],SIGNAL(leClicked(int)),this,SLOT(lineEdit_clicked(int)));

	lineEdit[2] = new le(ui.groupBox_3);
	lineEdit[2]->setnum(2);
	lineEdit[2]->setGeometry(QRect(290,20,113,31));
	connect(lineEdit[2],SIGNAL(leClicked(int)),this,SLOT(lineEdit_clicked(int)));


	ui.pushButton_2->setEnabled(true);
	ui.pushButton_3->setEnabled(false);
	ui.pushButton_4->setEnabled(true);
	ui.pushButton_5->setEnabled(false);
	ui.pushButton_6->setEnabled(true);
	ui.pushButton_7->setEnabled(false);
	lineEdit[0]->setEnabled(false);
	lineEdit[1]->setEnabled(false);
	lineEdit[2]->setEnabled(false);
}

measurepulse::~measurepulse()
{

}

void measurepulse::lineEdit_clicked(int num)
{
	keyboard2 keyboardinst;
	keyboardinst.setText(lineEdit[num]->text());
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//ȡ��canel
	if (keyboardinst.flag == 1)
	{
		;
	}//ȷ��
	else if (keyboardinst.flag == 2)
	{
		lineEdit[num]->clear();
		lineEdit[num]->insert(keyboardinst.enter);
		std::cout <<  keyboardinst.enter.toStdString()  << std::endl;
	}
}

void measurepulse::on_pushButton_clicked()
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","��һ�ο�ʼ","�û���һ�ο�ʼ",1);

	//��¼������Ŀ
	double count = 0;
	ksmc->GetEncoderCount(count);
	encoder[0] = count;

	ui.pushButton->setEnabled(false);
	ui.pushButton_8->setEnabled(false);

	step = 1;

	interfaceevent* ptempevent = new interfaceevent();

	ptempevent->cmd = 0x0a;//��̺��ʼ�˶�
	ptempevent->status = 0x00;//����״̬

	//����һ���¼�����̨�߳�
	QCoreApplication::postEvent(backendobject, ptempevent);

}
void measurepulse::on_pushButton_2_clicked()
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","��һ��ֹͣ","�û���һ��ֹͣ",1);

	//ֹͣ
	ui.pushButton_2->setEnabled(true);

	interfaceevent* ptempevent = new interfaceevent();


	ptempevent->cmd = 0x0a;//��̺�˶�ֹͣ
	ptempevent->status = 0x01;//����״̬

	//����һ���¼�����̨�߳�
	QCoreApplication::postEvent(backendobject, ptempevent);
}

void measurepulse::on_pushButton_3_clicked()
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","�ڶ��ο�ʼ","�û��ڶ��ο�ʼ",1);

	//У���ϴεĲ���
	QRegExp rx("^\\d+\\.?\\d*$");
	if ( rx.indexIn(lineEdit[0]->text()) == -1)
	{
		QMessageBox msgBox;
		msgBox.setText("�����ʽ���ԣ�����������");
		msgBox.setWindowTitle("�������");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();
		return;
	}

	bool ok;

	if (lineEdit[0]->text().toDouble(&ok) == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("��������Ϊ0������������");
		msgBox.setWindowTitle("�������");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();
		return;
	}

	//��¼������Ŀ
	double count = 0;
	ksmc->GetEncoderCount(count);

	//��ֵС��С�ھ�ֵ��˵�����ܷ�ת��
	if (count < encoder[0])
	{
		//������ת�������Ǳ���������,�������岻���ܳ���3000000000
		if (count < 1000000000 && encoder[0] > 4000000000)
		{
			encoder[0] = 4294967295 - encoder[0] + count;
		} 
		else
		{
			encoder[0] = count - encoder[0];
		}
	}
	else
	{
		encoder[0] = count - encoder[0];
	}

	//У��������
	if (encoder[0] == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("����������û�б仯�������²���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		ui.pushButton->setEnabled(true);
		ui.pushButton_2->setEnabled(true);
		ui.pushButton_3->setEnabled(false);
		ui.pushButton_4->setEnabled(true);
		ui.pushButton_5->setEnabled(false);
		ui.pushButton_6->setEnabled(true);
		ui.pushButton_7->setEnabled(false);
		lineEdit[0]->setEnabled(false);
		lineEdit[1]->setEnabled(false);
		lineEdit[2]->setEnabled(false);
		lineEdit[0]->clear();

		//����
		return;
	}

	//��¼������Ŀ
	count = 0;
	ksmc->GetEncoderCount(count);
	encoder[1] = count;

	step = 2;
	interfaceevent* ptempevent = new interfaceevent();
	ptempevent->cmd = 0x0a;//��̺��ʼ�˶�
	ptempevent->status = 0x00;//����״̬

	//����һ���¼�����̨�߳�
	QCoreApplication::postEvent(backendobject, ptempevent);

	//����
	ui.pushButton_3->setEnabled(false);
	lineEdit[0]->setEnabled(false);
	ui.pushButton_8->setEnabled(false);

}
void measurepulse::on_pushButton_4_clicked()
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","�ڶ���ֹͣ","�û��ڶ���ֹͣ",1);

	interfaceevent* ptempevent = new interfaceevent();


	ptempevent->cmd = 0x0a;//��̺�˶�ֹͣ
	ptempevent->status = 0x01;//����״̬

	//����һ���¼�����̨�߳�
	QCoreApplication::postEvent(backendobject, ptempevent);

	ui.pushButton_4->setEnabled(false);


}
void measurepulse::on_pushButton_5_clicked()
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","�����ο�ʼ","�û������ο�ʼ",1);

	//У���ϴεĲ���
	QRegExp rx("^\\d+\\.?\\d*$");
	if ( rx.indexIn(lineEdit[1]->text()) == -1)
	{
		QMessageBox msgBox;
		msgBox.setText("�����ʽ���ԣ�����������");
		msgBox.setWindowTitle("�������");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		return;
	}
	bool ok;

	if (lineEdit[1]->text().toDouble(&ok) == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("��������Ϊ0������������");
		msgBox.setWindowTitle("�������");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();
		return;
	}

	//��¼������
	double count = 0;
	ksmc->GetEncoderCount(count);

	//��ֵС��С�ھ�ֵ��˵�����ܷ�ת��
	if (count < encoder[1])
	{
		//������ת�������Ǳ���������,�������岻���ܳ���3000000000
		if (count < 1000000000 && encoder[1] > 4000000000)
		{
			encoder[1] = 4294967295 - encoder[1] + count;
		} 
		else
		{
			encoder[1] = count - encoder[1];
		}
	}
	else
	{
		encoder[1] = count - encoder[1];
	}

	//У��������
	if (encoder[1] == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("����������û�б仯�������²���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		ui.pushButton->setEnabled(false);
		ui.pushButton_2->setEnabled(true);
		ui.pushButton_3->setEnabled(true);
		ui.pushButton_4->setEnabled(true);
		ui.pushButton_5->setEnabled(false);
		ui.pushButton_6->setEnabled(true);
		ui.pushButton_7->setEnabled(false);
		lineEdit[0]->setEnabled(false);
		lineEdit[1]->setEnabled(false);
		lineEdit[2]->setEnabled(false);
		lineEdit[1]->clear();

		//����
		return;
	}
	//��¼������Ŀ
	count = 0;
	ksmc->GetEncoderCount(count);
	encoder[2] = count;

	step = 3;

	interfaceevent* ptempevent = new interfaceevent();
	ptempevent->cmd = 0x0a;//��̺��ʼ�˶�
	ptempevent->status = 0x00;//����״̬

	//����һ���¼�����̨�߳�
	QCoreApplication::postEvent(backendobject, ptempevent);

	//����
	ui.pushButton_5->setEnabled(false);
	lineEdit[1]->setEnabled(false);
	ui.pushButton_8->setEnabled(false);

}
void measurepulse::on_pushButton_6_clicked()
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","������ֹͣ","�û�������ֹͣ",1);

	interfaceevent* ptempevent = new interfaceevent();


	ptempevent->cmd = 0x0a;//��̺�˶�ֹͣ
	ptempevent->status = 0x01;//����״̬

	//����һ���¼�����̨�߳�
	QCoreApplication::postEvent(backendobject, ptempevent);

	//ֹͣ
	ui.pushButton_6->setEnabled(true);

}
void measurepulse::on_pushButton_7_clicked()
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","��ɲ��Բ��˳�","�û���ɲ��Բ��˳�",1);

	//У���ϴεĲ���
	QRegExp rx("^\\d+\\.?\\d*$");
	if ( rx.indexIn(lineEdit[2]->text()) == -1)
	{
		QMessageBox msgBox;
		msgBox.setText("�����ʽ���ԣ�����������");
		msgBox.setWindowTitle("�������");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		return;
	}
	bool ok;

	if (lineEdit[2]->text().toDouble(&ok) == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("��������Ϊ0������������");
		msgBox.setWindowTitle("�������");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();
		return;
	}

	double count = 0;
	ksmc->GetEncoderCount(count);

	//��ֵС��С�ھ�ֵ��˵�����ܷ�ת��
	if (count < encoder[2])
	{
		//������ת�������Ǳ���������,�������岻���ܳ���3000000000
		if (count < 1000000000 && encoder[2] > 4000000000)
		{
			encoder[2] = 4294967295 - encoder[2] + count;
		} 
		else
		{
			encoder[2] = count - encoder[2];
		}
	}
	else
	{
		encoder[2] = count - encoder[2];
	}


	//У��������
	if (encoder[2] == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("����������û�б仯�������²���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		ui.pushButton->setEnabled(false);
		ui.pushButton_2->setEnabled(false);
		ui.pushButton_3->setEnabled(false);
		ui.pushButton_4->setEnabled(false);
		ui.pushButton_5->setEnabled(true);
		ui.pushButton_6->setEnabled(false);
		ui.pushButton_7->setEnabled(false);
		lineEdit[0]->setEnabled(false);
		lineEdit[1]->setEnabled(false);
		lineEdit[2]->setEnabled(false);
		lineEdit[2]->clear();

		//����
		return;
	}

	
	//3�ε�Ħ��ϵ��У��
	//һ���Ա�ʶ
	bool validcheck = true;
	meaure[0] = lineEdit[0]->text().toDouble(&ok);
	meaure[1] = lineEdit[1]->text().toDouble(&ok);
	meaure[2] = lineEdit[2]->text().toDouble(&ok);



	double frications[3];

	for (int i = 0;i < 3; ++i)
	{
		if (meaure[i] != 0)
		{
			frications[i] = encoder[i] / meaure[i];
			std::cout << "encoder,measure,frication:" <<  encoder[i] << "," << meaure[i] << "," << frications[i];
		} 
		else
		{
			frications[i] = 0;

		}
	}

	if((frications[0] - frications[1]) > 0)
	{
		if (((frications[0] - frications[1]) /frications[0]) > 0.005 )
		{
			validcheck = false;
		}
	}
	else
	{
		if (((frications[1] - frications[0]) /frications[0]) > 0.005 )
		{
			validcheck = false;

		}
	}

	if((frications[0] - frications[2]) > 0)
	{
		if (((frications[0] - frications[2]) /frications[0]) > 0.005 )
		{
			validcheck = false;
		}
	}
	else
	{
		if (((frications[2] - frications[0]) /frications[0]) > 0.005 )
		{
			validcheck = false;

		}
	}


	if((frications[2] - frications[1]) > 0)
	{
		if (((frications[2] - frications[1]) /frications[2]) > 0.005 )
		{
			validcheck = false;
		}
	}
	else
	{
		if (((frications[1] - frications[2]) /frications[2]) > 0.005 )
		{
			validcheck = false;
		}
	}

	if (validcheck == false)
	{
		QMessageBox msgBox;
		msgBox.setText("����һ���Բ��У������²���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		ui.pushButton->setEnabled(true);
		ui.pushButton_2->setEnabled(false);
		ui.pushButton_3->setEnabled(false);
		ui.pushButton_4->setEnabled(false);
		ui.pushButton_5->setEnabled(false);
		ui.pushButton_6->setEnabled(false);
		ui.pushButton_7->setEnabled(false);
		lineEdit[0]->setEnabled(false);
		lineEdit[1]->setEnabled(false);
		lineEdit[2]->setEnabled(false);
		lineEdit[0]->clear();
		lineEdit[1]->clear();
		lineEdit[2]->clear();
		return;

	}

    long frication = (long)(((frications[0] + frications[1] + frications[2])/3 ) * 1000)  ; 

	//Ħ��ϵ����Χ���
	QDomNode paranode = GetParaByName("tune", "Ħ��ϵ��");
	float minval,maxval;

	minval = paranode.firstChildElement("rangemin").text().toFloat(&ok);
	maxval = paranode.firstChildElement("rangemax").text().toFloat(&ok);

	//У�������Ƿ񳬱�
	if (frication > maxval
		|| frication < minval)
	{
		QMessageBox msgBox;
		msgBox.setText("Ħ��ϵ�����ݳ�����Χ�����²��ԣ�");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		ui.pushButton->setEnabled(true);
		ui.pushButton_2->setEnabled(false);
		ui.pushButton_3->setEnabled(false);
		ui.pushButton_4->setEnabled(false);
		ui.pushButton_5->setEnabled(false);
		ui.pushButton_6->setEnabled(false);
		ui.pushButton_7->setEnabled(false);
		lineEdit[0]->setEnabled(false);
		lineEdit[1]->setEnabled(false);
		lineEdit[2]->setEnabled(false);
		lineEdit[0]->clear();
		lineEdit[1]->clear();
		lineEdit[2]->clear();

		return;
	}


	//�����Ӧ��
	paranode.firstChildElement("value").firstChild().setNodeValue(
		QString::number(frication));
	ksmc->SetPulserPerMeter(frication);

	//ˢ�µ�����
	FricationNodeItem->setText(QString::number(frication));

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
		return;
	}

	QTextStream out(&file);
	doc->save(out, 4);

	//����
	this->accept();
}
void measurepulse::on_pushButton_8_clicked()
{
	publiccaution.addevent("Ħ��ϵ������ҳ��","�˳�Ħ��ϵ������ҳ��","�û��˳�Ħ��ϵ������ҳ��",1);

	//ֱ���˳�
	this->accept();
}

void measurepulse::customEvent(QEvent *e)
{
	if (e->type() == BACKEND_EVENT) //�õ��Ǻ�̨���¼�
	{
		backendevent* event = (backendevent*) e;

		//��̺��ʼ
		if (event->cmd == 0x0a && event->status == 0x00)
		{
			switch(step) {
			case 1:
				ui.pushButton_2->setEnabled(true);

				break;
			case 2:
				ui.pushButton_4->setEnabled(true);

				break;
			case 3:
				ui.pushButton_6->setEnabled(true);

				break;
			default:
				;
			}

		}//��ֹ̺ͣ
		else if (event->cmd == 0x0a && event->status == 0x01)
		{
			switch(step) {
			case 1:
				ui.pushButton_3->setEnabled(true);
				lineEdit[0]->setEnabled(true);
				ui.pushButton_8->setEnabled(true);

				break;
			case 2:
				ui.pushButton_5->setEnabled(true);
				lineEdit[1]->setEnabled(true);
				ui.pushButton_8->setEnabled(true);

				break;
			case 3:
				ui.pushButton_7->setEnabled(true);
				lineEdit[2]->setEnabled(true);
				ui.pushButton_8->setEnabled(true);

				break;
			default:
				;
			}
		}
	}
}