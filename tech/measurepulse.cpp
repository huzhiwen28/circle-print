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

//A4N电机
extern CKSMCA4 *ksmc;
extern QDomDocument* doc;
extern QTableWidgetItem* FricationNodeItem;
extern QObject* backendobject;

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);

measurepulse::measurepulse(QWidget *parent)
: QDialog(parent)
{
	publiccaution.addevent("摩擦系数测试页面","进入摩擦系数测试页面","用户进入摩擦系数测试页面",1);

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

	//取消canel
	if (keyboardinst.flag == 1)
	{
		;
	}//确认
	else if (keyboardinst.flag == 2)
	{
		lineEdit[num]->clear();
		lineEdit[num]->insert(keyboardinst.enter);
		std::cout <<  keyboardinst.enter.toStdString()  << std::endl;
	}
}

void measurepulse::on_pushButton_clicked()
{
	publiccaution.addevent("摩擦系数测试页面","第一次开始","用户第一次开始",1);

	//记录脉冲数目
	double count = 0;
	ksmc->GetEncoderCount(count);
	encoder[0] = count;

	ui.pushButton->setEnabled(false);
	ui.pushButton_8->setEnabled(false);

	step = 1;

	interfaceevent* ptempevent = new interfaceevent();

	ptempevent->cmd = 0x0a;//橡毯开始运动
	ptempevent->status = 0x00;//命令状态

	//发送一个事件给后台线程
	QCoreApplication::postEvent(backendobject, ptempevent);

}
void measurepulse::on_pushButton_2_clicked()
{
	publiccaution.addevent("摩擦系数测试页面","第一次停止","用户第一次停止",1);

	//停止
	ui.pushButton_2->setEnabled(true);

	interfaceevent* ptempevent = new interfaceevent();


	ptempevent->cmd = 0x0a;//橡毯运动停止
	ptempevent->status = 0x01;//命令状态

	//发送一个事件给后台线程
	QCoreApplication::postEvent(backendobject, ptempevent);
}

void measurepulse::on_pushButton_3_clicked()
{
	publiccaution.addevent("摩擦系数测试页面","第二次开始","用户第二次开始",1);

	//校验上次的测试
	QRegExp rx("^\\d+\\.?\\d*$");
	if ( rx.indexIn(lineEdit[0]->text()) == -1)
	{
		QMessageBox msgBox;
		msgBox.setText("输入格式不对，请重新输入");
		msgBox.setWindowTitle("输入错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();
		return;
	}

	bool ok;

	if (lineEdit[0]->text().toDouble(&ok) == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("输入数据为0，请重新输入");
		msgBox.setWindowTitle("输入错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();
		return;
	}

	//记录脉冲数目
	double count = 0;
	ksmc->GetEncoderCount(count);

	//新值小于小于旧值，说明可能翻转了
	if (count < encoder[0])
	{
		//真正翻转，而不是编码器回退,回退脉冲不可能超过3000000000
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

	//校验编码计数
	if (encoder[0] == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("编码器计数没有变化，请重新测试");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
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

		//返回
		return;
	}

	//记录脉冲数目
	count = 0;
	ksmc->GetEncoderCount(count);
	encoder[1] = count;

	step = 2;
	interfaceevent* ptempevent = new interfaceevent();
	ptempevent->cmd = 0x0a;//橡毯开始运动
	ptempevent->status = 0x00;//命令状态

	//发送一个事件给后台线程
	QCoreApplication::postEvent(backendobject, ptempevent);

	//启动
	ui.pushButton_3->setEnabled(false);
	lineEdit[0]->setEnabled(false);
	ui.pushButton_8->setEnabled(false);

}
void measurepulse::on_pushButton_4_clicked()
{
	publiccaution.addevent("摩擦系数测试页面","第二次停止","用户第二次停止",1);

	interfaceevent* ptempevent = new interfaceevent();


	ptempevent->cmd = 0x0a;//橡毯运动停止
	ptempevent->status = 0x01;//命令状态

	//发送一个事件给后台线程
	QCoreApplication::postEvent(backendobject, ptempevent);

	ui.pushButton_4->setEnabled(false);


}
void measurepulse::on_pushButton_5_clicked()
{
	publiccaution.addevent("摩擦系数测试页面","第三次开始","用户第三次开始",1);

	//校验上次的测试
	QRegExp rx("^\\d+\\.?\\d*$");
	if ( rx.indexIn(lineEdit[1]->text()) == -1)
	{
		QMessageBox msgBox;
		msgBox.setText("输入格式不对，请重新输入");
		msgBox.setWindowTitle("输入错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		return;
	}
	bool ok;

	if (lineEdit[1]->text().toDouble(&ok) == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("输入数据为0，请重新输入");
		msgBox.setWindowTitle("输入错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();
		return;
	}

	//记录脉冲数
	double count = 0;
	ksmc->GetEncoderCount(count);

	//新值小于小于旧值，说明可能翻转了
	if (count < encoder[1])
	{
		//真正翻转，而不是编码器回退,回退脉冲不可能超过3000000000
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

	//校验编码计数
	if (encoder[1] == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("编码器计数没有变化，请重新测试");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
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

		//返回
		return;
	}
	//记录脉冲数目
	count = 0;
	ksmc->GetEncoderCount(count);
	encoder[2] = count;

	step = 3;

	interfaceevent* ptempevent = new interfaceevent();
	ptempevent->cmd = 0x0a;//橡毯开始运动
	ptempevent->status = 0x00;//命令状态

	//发送一个事件给后台线程
	QCoreApplication::postEvent(backendobject, ptempevent);

	//启动
	ui.pushButton_5->setEnabled(false);
	lineEdit[1]->setEnabled(false);
	ui.pushButton_8->setEnabled(false);

}
void measurepulse::on_pushButton_6_clicked()
{
	publiccaution.addevent("摩擦系数测试页面","第三次停止","用户第三次停止",1);

	interfaceevent* ptempevent = new interfaceevent();


	ptempevent->cmd = 0x0a;//橡毯运动停止
	ptempevent->status = 0x01;//命令状态

	//发送一个事件给后台线程
	QCoreApplication::postEvent(backendobject, ptempevent);

	//停止
	ui.pushButton_6->setEnabled(true);

}
void measurepulse::on_pushButton_7_clicked()
{
	publiccaution.addevent("摩擦系数测试页面","完成测试并退出","用户完成测试并退出",1);

	//校验上次的测试
	QRegExp rx("^\\d+\\.?\\d*$");
	if ( rx.indexIn(lineEdit[2]->text()) == -1)
	{
		QMessageBox msgBox;
		msgBox.setText("输入格式不对，请重新输入");
		msgBox.setWindowTitle("输入错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		return;
	}
	bool ok;

	if (lineEdit[2]->text().toDouble(&ok) == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("输入数据为0，请重新输入");
		msgBox.setWindowTitle("输入错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();
		return;
	}

	double count = 0;
	ksmc->GetEncoderCount(count);

	//新值小于小于旧值，说明可能翻转了
	if (count < encoder[2])
	{
		//真正翻转，而不是编码器回退,回退脉冲不可能超过3000000000
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


	//校验编码计数
	if (encoder[2] == 0)
	{
		QMessageBox msgBox;
		msgBox.setText("编码器计数没有变化，请重新测试");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
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

		//返回
		return;
	}

	
	//3次的摩擦系数校验
	//一致性标识
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
		msgBox.setText("数据一致性不行，请重新测试");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
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

	//摩擦系数范围检查
	QDomNode paranode = GetParaByName("tune", "摩擦系数");
	float minval,maxval;

	minval = paranode.firstChildElement("rangemin").text().toFloat(&ok);
	maxval = paranode.firstChildElement("rangemax").text().toFloat(&ok);

	//校验数据是否超标
	if (frication > maxval
		|| frication < minval)
	{
		QMessageBox msgBox;
		msgBox.setText("摩擦系数数据超出范围请重新测试！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
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


	//保存和应用
	paranode.firstChildElement("value").firstChild().setNodeValue(
		QString::number(frication));
	ksmc->SetPulserPerMeter(frication);

	//刷新到界面
	FricationNodeItem->setText(QString::number(frication));

	//保存XML文件
	QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
	if (!file.open(QFile::WriteOnly))
	{
		QMessageBox msgBox;
		msgBox.setText("打开文件aaa.xml失败");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();
		return;
	}

	QTextStream out(&file);
	doc->save(out, 4);

	//返回
	this->accept();
}
void measurepulse::on_pushButton_8_clicked()
{
	publiccaution.addevent("摩擦系数测试页面","退出摩擦系数测试页面","用户退出摩擦系数测试页面",1);

	//直接退出
	this->accept();
}

void measurepulse::customEvent(QEvent *e)
{
	if (e->type() == BACKEND_EVENT) //得到是后台的事件
	{
		backendevent* event = (backendevent*) e;

		//橡毯开始
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

		}//橡毯停止
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