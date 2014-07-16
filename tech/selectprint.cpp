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

//采用setting保存部分配置
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

//网头状态和IO状态
//extern QLabel* PrintStatus[16][6];

int GetParaRow(const QString& groupname);

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);

//遍历XML树，取得某类中的一个参数
const QDomNode GetPara(const QString& groupname, const QString tag,
					   const QString& paraname);

//事件消息传递对象
extern QObject* backendobject;
extern QObject* interfaceobject;
extern QObject* a4nobject;
extern QObject* selectprintobject;

extern QString dlgbkcolor;

//A4N电机
extern CKSMCA4 *ksmc;

QString bubkcolor = QString("background-color:rgb(173,255,47)");
QString bubkcolor_red = QString("background-color:rgb( 255,0,0 )");

selectprint::selectprint(QWidget *parent)
: QDialog(parent)
{
	ui.setupUi(this);
	setStyleSheet(dlgbkcolor); //设置对话框的背景颜色（亮青色） 

	printstatuschgflag = false;
	ui.cancelButton->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.cancelButton_2->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

}

void selectprint::init()//初始化网头选择界面
{
	//QRect t = geometry();

	//setGeometry(QRect(t.left(),t.top(),1024,626));

	textframe = new QFrame( this );//边框,放置操作面板的IO用的
	textframe->setGeometry(QRect(670, 20, 340, 531));
	textframe->setFrameShadow(QFrame::Sunken);
	textframe->setFrameShape(QFrame::StyledPanel);
	textframe->setLineWidth(2);

	opprinttext = new QLabel( this );
	opprinttext->setText( "<font size = 5><b>状态说明：</b><br><br>网头有三种可选状态：休眠 准备 工作<br><br>1、“休眠”是指圆网不可印花且网头不<br><br>可手动操作<br><br>2、“准备”是指圆网不可印花但网头可<br><br>    手动操作<br><br>3、“工作”是指不但圆网可印花，而且<br><br>网头可手动操作<br><br><b>操作说明：</b><br><br>1、当网头当前状态为“休眠”时，您可<br><br>以点击“准备”按钮，激活该网头<br><br>2、当网头当前状态为“准备”时，您可<br><br>以点击“工作”按钮，使网头进行工作</font>"  );
	opprinttext->setGeometry(QRect(680, -30, 350, 520));//加载内容

	table[0] = new QTableWidget( this );
	table[0]->setRowCount( pubcom.PrintNum );
	table[0]->setColumnCount(4);
	table[0]->setGeometry(QRect(40, 20, 600, 32 * ( pubcom.PrintNum + 1 )));
	table[0]->setRowHeight(2, 20);
	table[0]->horizontalHeader()->setStretchLastSection(true);//去除列中多余空白
	table[0]->verticalHeader()->setStretchLastSection(true);//去除行中多余空白
	table[0]->horizontalHeader()->setResizeMode(QHeaderView::Stretch);//使列自动适应宽度
	table[0]->verticalHeader()->setResizeMode(QHeaderView::Stretch);//使行自动适应宽度
	table[0]->setStyleSheet("font-size: 18px");

	ctablehead[0] = new QTableWidgetItem("网头状态");
	table[0]->setHorizontalHeaderItem(0, ctablehead[0]);

	ctablehead[1] = new QTableWidgetItem("状态选择(休眠)");
	table[0]->setHorizontalHeaderItem(1, ctablehead[1]);

	ctablehead[2] = new QTableWidgetItem("状态选择(激活)");
	table[0]->setHorizontalHeaderItem(2, ctablehead[2]);

	ctablehead[3] = new QTableWidgetItem("状态选择(工作)");
	table[0]->setHorizontalHeaderItem(3, ctablehead[3]);

	for ( int i = 0; i < pubcom.PrintNum; i++ )
	{
		rtablehead[i] = new QTableWidgetItem((QString("网头") + QString::number(i+1)));
		table[0]->setVerticalHeaderItem(i, rtablehead[i]);

		UIprintstatus[i] = pubcom.presetprintstatus[i];

		switch (pubcom.presetprintstatus[i])
		{
			//休眠状态
		case 1:
			{
				lbprintstatus[i] = new QLabel();//在表格中添加图片
				lbprintstatus[i]->setPixmap(QPixmap(":/images/sleep.jpg"));
				table[0]->setCellWidget(i , 0 ,lbprintstatus[i]);

				sleeppb[i] = new pb(this);//在表格中添加按钮
				sleeppb[i]->setText("休眠");
				sleeppb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 1 ,sleeppb[i]);
				sleeppb[i]->setStyleSheet( bubkcolor );
				sleeppb[i]->setEnabled(false);
				connect( sleeppb[i], SIGNAL(buttonClicked(int)), this, SLOT(sleep_clicked(int)));

				preparepb[i] = new pb(this);//在表格中添加按钮
				preparepb[i]->setText("准备");
				table[0]->setCellWidget(i , 2 ,preparepb[i]);
				preparepb[i]->setStyleSheet( bubkcolor );
				preparepb[i]->setEnabled(true);
				preparepb[i]->setnum(i+1);
				connect( preparepb[i], SIGNAL(buttonClicked(int)), this, SLOT(prepare_clicked(int)));

				workpb[i] = new pb(this);//在表格中添加按钮
				workpb[i]->setText("工作");
				table[0]->setCellWidget(i , 3 ,workpb[i]);
				workpb[i]->setStyleSheet( bubkcolor );
				workpb[i]->setEnabled(true);
				workpb[i]->setnum(i+1);
				connect( workpb[i], SIGNAL(buttonClicked(int)), this, SLOT(work_clicked(int)));

				break;
			}

			//准备状态
		case 4:
			{
				lbprintstatus[i] = new QLabel();//在表格中添加图片
				lbprintstatus[i]->setPixmap(QPixmap(":/images/prepare.jpg"));
				table[0]->setCellWidget(i , 0 ,lbprintstatus[i]);

				sleeppb[i] = new pb(this);//在表格中添加按钮
				sleeppb[i]->setText("休眠");
				table[0]->setCellWidget(i , 1 ,sleeppb[i]);
				sleeppb[i]->setStyleSheet( bubkcolor );
				sleeppb[i]->setEnabled(false);
				sleeppb[i]->setnum(i+1);
				connect( sleeppb[i], SIGNAL(buttonClicked(int)), this, SLOT(sleep_clicked(int)));


				preparepb[i] = new pb(this);//在表格中添加按钮
				preparepb[i]->setText("准备");
				preparepb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 2 ,preparepb[i]);
				preparepb[i]->setStyleSheet( bubkcolor );
				preparepb[i]->setEnabled(false);
				connect( preparepb[i], SIGNAL(buttonClicked(int)), this, SLOT(prepare_clicked(int)));

				workpb[i] = new pb(this);//在表格中添加按钮
				workpb[i]->setText("工作");
				workpb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 3 ,workpb[i]);
				workpb[i]->setStyleSheet( bubkcolor );
				workpb[i]->setEnabled(true);
				connect( workpb[i], SIGNAL(buttonClicked(int)), this, SLOT(work_clicked(int)));

				break;
			}
			//工作状态
		case 3:
			{
				lbprintstatus[i] = new QLabel();//在表格中添加图片
				lbprintstatus[i]->setPixmap(QPixmap(":/images/work.jpg"));
				table[0]->setCellWidget(i , 0 ,lbprintstatus[i]);

				sleeppb[i] = new pb(this);//在表格中添加按钮
				sleeppb[i]->setText("休眠");
				sleeppb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 1 ,sleeppb[i]);
				sleeppb[i]->setStyleSheet( bubkcolor );
				sleeppb[i]->setEnabled(true);
				connect( sleeppb[i], SIGNAL(buttonClicked(int)), this, SLOT(sleep_clicked(int)));

				preparepb[i] = new pb(this);//在表格中添加按钮
				preparepb[i]->setText("准备");
				preparepb[i]->setnum(i+1);
				table[0]->setCellWidget(i , 2 ,preparepb[i]);
				preparepb[i]->setStyleSheet( bubkcolor );
				preparepb[i]->setEnabled(true);
				connect( preparepb[i], SIGNAL(buttonClicked(int)), this, SLOT(prepare_clicked(int)));

				workpb[i] = new pb(this);//在表格中添加按钮
				workpb[i]->setText("工作");
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
	//防止未处理完退出
	if(printstatuschgflag != true)
	{
		this->accept();
	}
}


//网头休眠
void selectprint::sleep_clicked(int num)
{
	//只有在网头准备空闲的时候才处理按钮
	if (printstatuschgflag != true)
	{
		interfaceevent* ptempevent = new interfaceevent();

		ptempevent->cmd = 0x07;
		ptempevent->status = 0x00;//命令状态
		ptempevent->data[0] = num-1;//网头编号
		ptempevent->data[1] = 1;//1休眠 3工作 4准备

		cmdprintnum = ptempevent->data[0];
		cmdstatus = ptempevent->data[1];

		printstatuschgflag = true;
		//发送一个事件给后台线程
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}

//网头准备
void selectprint::prepare_clicked(int num)
{
	//只有在网头准备空闲的时候才处理按钮
	if (printstatuschgflag != true)
	{
		interfaceevent* ptempevent = new interfaceevent();

		ptempevent->cmd = 0x07;
		ptempevent->status = 0x00;//命令状态
		ptempevent->data[0] = num-1;//网头编号
		ptempevent->data[1] = 4;//1休眠 3工作 4准备

		cmdprintnum = ptempevent->data[0];
		cmdstatus = ptempevent->data[1];

		printstatuschgflag = true;
		//发送一个事件给后台线程
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}
//网头工作
void selectprint::work_clicked(int num)
{
	//只有在网头准备空闲的时候才处理按钮
	if (printstatuschgflag != true)
	{
		interfaceevent* ptempevent = new interfaceevent();

		ptempevent->cmd = 0x07;
		ptempevent->status = 0x00;//命令状态
		ptempevent->data[0] = num-1;//网头编号
		ptempevent->data[1] = 3;//1休眠 3工作 4准备

		cmdprintnum = ptempevent->data[0];
		cmdstatus = ptempevent->data[1];

		printstatuschgflag = true;
		//发送一个事件给后台线程
		QCoreApplication::postEvent(backendobject, ptempevent);

		//网头调速
		QDomNode para = GetParaByName("machsub1", "网头调速允许按钮是否工作");
		bool ok;
		bool btwork = para.firstChildElement("value").text().toInt(&ok);

		if (btwork == true)
		{
			//只有在网头调速按钮空闲的时候才处理按钮
			if (pubcom.printchgspeedansflag != true)
			{
				interfaceevent* ptempevent = new interfaceevent();

				ptempevent->cmd = 0x0b;//网头操作命令
				ptempevent->status = 0x00;//命令状态
				ptempevent->data[0] = 0x00;

				pubcom.singleprintchgspeedid = num-1;

				//发送一个事件给界面线程
				QCoreApplication::postEvent(backendobject, ptempevent);
			}
		}

	}
}

void selectprint::customEvent(QEvent *e)
{
	if (e->type() == BACKEND_EVENT) //得到是后台的事件
	{
		backendevent* event = (backendevent*) e;

		//网头选择,网头有CAN命令反馈
		if (event->cmd == 0x07
			&& event->status == 0x01)
		{
			//如果返回的状态和预设的一致
			if (event->data[1] == cmdstatus)
			{
				//影响到配置
				if ((pubcom.presetprintstatus[cmdprintnum] == 1) 
					&& (cmdstatus == 3 || cmdstatus == 4))
				{
     				pubcom.activeprintlistmutex.lockForWrite();
					pubcom.activeprintlist.append(cmdprintnum);
					pubcom.activeprintlistmutex.unlock();


					ksmc->AddMotor(cmdprintnum + 1);

					//初始化马达
					ksmc->InitMotor(cmdprintnum + 1);

					//激活马达
					ksmc->SetActiveMotor(cmdprintnum + 1);

				}
				else if ((pubcom.presetprintstatus[cmdprintnum] == 3  || pubcom.presetprintstatus[cmdprintnum] == 4) 
					&& (cmdstatus == 1))
				{
					int index = pubcom.activeprintlist.indexOf(cmdprintnum);

					pubcom.activeprintlistmutex.lockForWrite();
					pubcom.activeprintlist.removeAt(index);
    				pubcom.activeprintlistmutex.unlock();

					//删除故障
					pubcom.delcauflag = true;
					publiccaution.delcaution(
						pubcom.bdcautionmap.value(cmdprintnum));
					pubcom.delcauflag = false;

					//对于马达的操作要慎重要确保是非工作状态才可以设置
					ksmc->SetOffMotor(cmdprintnum + 1);
					ksmc->DelMotor(cmdprintnum + 1);
				}

				//预设状态改变
				pubcom.presetprintstatus[cmdprintnum] = cmdstatus;

				//网头状态改变
				pubcom.printstatus[cmdprintnum] = cmdstatus;

				//保存到XML
				QString paraname = QString("网头") + QString::number(
					cmdprintnum + 1) + QString("状态");

				//网头准备状态修改
				QDomNode paranode = GetParaByName("print", paraname);
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(cmdstatus));

				//界面修改
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
					printstatuschgflag = false;

					return;
				}

				QTextStream out(&file);
				doc->save(out, 4);
			}
			else
			{
				//提示当前不能修改
				QMessageBox msgBox;
				msgBox.setText("状态设置失败");
				msgBox.setWindowTitle("失败");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();

			}

			printstatuschgflag = false;
		}
		//网头没有CAN命令反馈，强行改变状态
		else if (event->cmd == 0x07
			&& event->status == 0x02)
		{
			//影响到配置，只有强行休眠才有意义(隔离故障网头)，强行打开如果网头没有应答就没有意义了
			if ((pubcom.presetprintstatus[cmdprintnum] == 3  || pubcom.presetprintstatus[cmdprintnum] == 4) 
				&& (cmdstatus == 1))
			{
				int index = pubcom.activeprintlist.indexOf(cmdprintnum);

				pubcom.activeprintlistmutex.lockForWrite();
				pubcom.activeprintlist.removeAt(index);
				pubcom.activeprintlistmutex.unlock();

				//删除故障
				pubcom.delcauflag = true;
				publiccaution.delcaution(
					pubcom.bdcautionmap.value(cmdprintnum));
				pubcom.delcauflag = false;

				//对于马达的操作要慎重要确保是非工作状态才可以设置
				ksmc->SetOffMotor(cmdprintnum + 1);
				ksmc->DelMotor(cmdprintnum + 1);

				//预设状态改变
				pubcom.presetprintstatus[cmdprintnum] = cmdstatus;

				//保存到XML
				QString paraname = QString("网头") + QString::number(
					cmdprintnum + 1) + QString("状态");

				//网头准备状态修改
				QDomNode paranode = GetParaByName("print", paraname);
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(cmdstatus));

				//界面修改
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

					printstatuschgflag = false;

					return;
				}

				QTextStream out(&file);
				doc->save(out, 4);

				//提示当前不能修改
				QMessageBox msgBox;
				msgBox.setText("状态强行设置成功");
				msgBox.setWindowTitle("警告");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();

			}

			else if ((pubcom.presetprintstatus[cmdprintnum] == 1) 
				&& (cmdstatus == 3 || cmdstatus == 4))
			{
				pubcom.activeprintlistmutex.lockForWrite();
				pubcom.activeprintlist.append(cmdprintnum);
				pubcom.activeprintlistmutex.unlock();

				ksmc->AddMotor(cmdprintnum + 1);

				//初始化马达
				ksmc->InitMotor(cmdprintnum + 1);

				//激活马达
				ksmc->SetActiveMotor(cmdprintnum + 1);

				//预设状态改变
				pubcom.presetprintstatus[cmdprintnum] = cmdstatus;

				//保存到XML
				QString paraname = QString("网头") + QString::number(
					cmdprintnum + 1) + QString("状态");

				//网头准备状态修改
				QDomNode paranode = GetParaByName("print", paraname);
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(cmdstatus));
				//界面修改
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

					printstatuschgflag = false;

					return;
				}

				QTextStream out(&file);
				doc->save(out, 4);

				//提示当前不能修改
				QMessageBox msgBox;
				msgBox.setText("状态强行设置成功");
				msgBox.setWindowTitle("警告");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();

			}
			else
			{
				//预设状态改变
				pubcom.presetprintstatus[cmdprintnum] = cmdstatus;

				//保存到XML
				QString paraname = QString("网头") + QString::number(
					cmdprintnum + 1) + QString("状态");

				//网头准备状态修改
				QDomNode paranode = GetParaByName("print", paraname);
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(cmdstatus));

				//界面修改
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
					printstatuschgflag = false;

					return;
				}

				QTextStream out(&file);
				doc->save(out, 4);

				QMessageBox msgBox;
				msgBox.setText("状态强行设置成功");
				msgBox.setWindowTitle("警告");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();
			}
			/*
			else
			{
			//提示当前不能修改
			QMessageBox::warning(this, tr("失败"), tr("状态设置失败"),
			QMessageBox::Yes );
			}
			*/

			printstatuschgflag = false;
		}
		//当前不可以设置
		else if (event->cmd == 0x07
			&& event->status == 0x03)
		{
			printstatuschgflag = false;
			QMessageBox msgBox;
			msgBox.setText("已经有操作在进行");
			msgBox.setWindowTitle("警告");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();
		}

	}
}

//关机按钮
void selectprint::on_cancelButton_2_clicked()
{
	keyboard keyboardinst;
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//取消canel
	if (keyboardinst.flag == 1)
	{
		;
	}//确认
	else if (keyboardinst.flag == 2)
	{
		//工艺
		if (settings.value("prod/pass").toString() == keyboardinst.password
			&& !settings.value("prod/pass").toString().isNull())
		{
			pubcom.PowerOff = true;
			this->accept();
		}//机器
		else if (settings.value("mach/pass").toString()
			== keyboardinst.password
			&& !settings.value("mach/pass").toString().isNull())
		{
			pubcom.PowerOff = true;
			this->accept();

		}//调试
		else if (settings.value("tune/pass").toString()
			== keyboardinst.password
			&& !settings.value("tune/pass").toString().isNull())
		{
			pubcom.PowerOff = true;
			this->accept();

		}//一般后门
		else if (keyboardinst.password == BACKDOORPASS )
		{
			pubcom.PowerOff = true;
			this->accept();

		}//开发后门
		else if (keyboardinst.password == DEVPASS)
		{
			pubcom.PowerOff = true;
			this->accept();
		}
		//密码不正确，普通用户无法设置
		else
		{
			return;
		}
	}
}