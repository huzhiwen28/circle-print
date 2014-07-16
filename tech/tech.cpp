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
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <Windows.h>
#include <QBitmap>
#include <QMatrix>
#include <math.h>
#include "tech.h"
#include "caution.h"
#include "help.h"
#include "keyboard.h"
#include "keyboard2.h"
#include "set.h"
#include "password.h"
#include "opprint.h"
#include "custevent.h"
#include "canif.h"
#include "motortest.h"
#include "cmotor.h"
#include "DB.h"
#include "selectprint.h"
#include "canif.h"
#include "serialif.h"
#include "safenet.h"
#include "modbus.h"

//全局的can设备
extern canif candevice;

//加密狗检测
extern safenet safenetinst;

static const QSize resultSize(200, 200);
QColor bgColorForName(const QString &name);

//采用setting保存部分配置
extern QSettings settings;

extern struct _modbusRTU modbusRTU;
extern struct _regs regs;


//风格字符串
QString choosestyle = QString("background-color: #00AA00; font: bold 14px;");//选择
QString unchoosestyle = QString("font: bold 14px;");//没有选择
QString preparestyle = QString("background-color: blue;font: bold 14px;");//准备
QString datetimestyle = QString("color: #0000FF; font-size: 24px");//日期
extern QString cautionstyle;

QString sudustyle = QString("color: #FFFF00");
QString chanliangstyle = QString("color: #0000FF");
QString choosestyle2 = QString("background-color: #00FF00; font: bold 14px;");
QString dlgbkcolor = QString("QDialog { background-color:rgb(253,251,202)}");

QPicture topbarpic;
QPicture topbarredpic;
QPicture topbargraypic;

char screemsaverflag;
long screemsavertime;

extern QDomDocument* doc;

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);

long screentimer;

timermng pubtimermng;

QObject* setobject = NULL;
QObject* opobject = NULL;

//指向set的对话框
set* pDlgset;

//指向网头操作的对话框
opprint* pDlgop;

//马达测试界面
motortest* pDlgmotortest;

//A4N电机
extern CKSMCA4 *ksmc;


//数据库
extern DB mydb;

extern QObject* backendobject;
extern QObject* interfaceobject;
extern QObject* a4nobject;
extern QObject* selectprintobject;

char cautionbkflag = 0;
char cautionbkchgflag = 0;
char datetimechgflag = 0;
char cautionstrchgflag = 0;
//当前显示的故障id
char curcautionindex = 0;

void emptyfunc(...)
{
}

tech::tech(QWidget *parent) :
QDialog(parent)
{
	ui.setupUi(this);

	bool ok;
	paintEventcnt = 0;

	//启动定时器
	timer = &mytimer;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));
	timer->start(100); //100ms定时

	timecount = 0;
	speed = 0;
	image2 = new QImage();
	image2->load(":/DSC_1422.PNG");

	imgmagic1 = new QImage();
	imgmagic1->load(":/images/citai1.PNG");

	imgmagic2 = new QImage();
	imgmagic2->load(":/images/citai2.PNG");

	imgbump1 = new QImage();
	imgbump1->load(":/images/jb1.PNG");

	imgbump2 = new QImage();
	imgbump2->load(":/images/jb2.PNG");

	backgroundimg = new QImage();
	backgroundimg->load(":/images/1.PNG", "PNG");
	int imgh = backgroundimg->height();
	int imgw = backgroundimg->width();

	for(int i = 0;i < 16;i++)
	{
		upflag[i] = 0;
		updistance[i] = 0;
	}
	upspeed = 1;
	upmax = 6;

	rolldir = 0;

	screenwork = 0;

	chgspeedbtflag = false; //改变速度按钮是否按了？

	printopflag = false;

	oldmotorspeed = 0;

	f.setPointSize(30);
	f2.setPointSize(15);
	greenpen = new QPen(Qt::green, 4);
	graypen = new QPen(Qt::gray, 4);
	blackpen = new QPen(Qt::black, 4);

	//当前故障指示
	curcautionindex = 0;

	//当前故障计时器
	cautiondispcount = 0;
	//ui.lcdNumber->setStyleSheet(sudustyle);
	//ui.lcdNumber_2->setStyleSheet(chanliangstyle);

	//屏保
	screemsaverflag = settings.value("screensaver/flag").toInt(&ok);
	screemsavertime = settings.value("screensaver/time").toInt() * 60 * 10;
	screentimer = 0;

	QPainter painter;

	QGradientStops luobubrush;
	luobubrush.append(QGradientStop(0, Qt::white));
	luobubrush.append(QGradientStop(0.1666, QColor(211, 88, 8, 255)));
	luobubrush.append(QGradientStop(0.3333, Qt::white));
	luobubrush.append(QGradientStop(0.5, QColor(211, 88, 8, 255)));
	luobubrush.append(QGradientStop(0.6666, Qt::white));
	luobubrush.append(QGradientStop(0.8333, QColor(211, 88, 8, 255)));
	luobubrush.append(QGradientStop(1, Qt::white));

	QGradientStops xiangtanbrush;
	xiangtanbrush.append(QGradientStop(0, Qt::white));
	xiangtanbrush.append(QGradientStop(0.1666, Qt::darkBlue));
	xiangtanbrush.append(QGradientStop(0.3333, Qt::white));
	xiangtanbrush.append(QGradientStop(0.5, Qt::darkBlue));
	xiangtanbrush.append(QGradientStop(0.6666, Qt::white));
	xiangtanbrush.append(QGradientStop(0.8333, Qt::darkBlue));
	xiangtanbrush.append(QGradientStop(1, Qt::white));

	QGradientStops yuanwangbrush;
	yuanwangbrush.append(QGradientStop(0, Qt::white));
	yuanwangbrush.append(QGradientStop(0.1666, Qt::black));
	yuanwangbrush.append(QGradientStop(0.3333, Qt::white));
	yuanwangbrush.append(QGradientStop(0.5, Qt::black));
	yuanwangbrush.append(QGradientStop(0.6666, Qt::white));
	yuanwangbrush.append(QGradientStop(0.8333, Qt::black));
	yuanwangbrush.append(QGradientStop(1, Qt::white));

	QConicalGradient kk(0, 0, 0);
	kk.setStops(luobubrush);

	painter.begin(&luobupic); // paint in picture
	painter.setBrush(kk);
	painter.drawEllipse(-29.70, -29.70, 59.40, 59.40); // draw an ellipse
	//painter.setBrush(QBrush(Qt::black));
	//painter.drawEllipse(20,20, 160,160);
	painter.end();

	painter.begin(&hongfangpic); // paint in picture
	painter.setBrush(kk);
	painter.drawEllipse(-20.00, -20.00, 40, 40); // draw an ellipse
	//painter.setBrush(QBrush(Qt::black));
	//painter.drawEllipse(20,20, 160,160);
	painter.end();

	kk.setStops(xiangtanbrush);
	painter.begin(&xiangtanpic); // paint in picture
	painter.setBrush(kk);
	painter.drawEllipse(-20.00, -20.00, 40, 40); // draw an ellipse
	//painter.setBrush(QBrush(Qt::black));
	//painter.drawEllipse(20,20, 160,160);
	painter.end();

	kk.setStops(yuanwangbrush);
	painter.begin(&yuanwangpic); // paint in picture
	painter.setBrush(kk);
	painter.drawEllipse(-10.00, -10.00, 20, 20); // draw an ellipse
	//painter.setBrush(QBrush(Qt::black));
	//painter.drawEllipse(20,20, 160,160);
	painter.end();

	painter.begin(&baibipic); // paint in picture
	painter.setBrush(Qt::NoBrush);
	painter.setPen(QPen(Qt::black, 1));
	painter.drawLine(-9, -23, 4, -23);
	painter.drawLine(4, -23, 16, 65);
	painter.drawLine(16, 65, 5, 66);
	painter.drawLine(5, 66, -9, -23);
	painter.setBrush(Qt::blue);
	painter.setPen(QPen(Qt::blue, 1));
	painter.translate(-2, -16);
	painter.drawEllipse(-4.50, -4.50, 9, 9); // draw an ellipse
	painter.translate(2, 16);

	painter.translate(4, 59);
	painter.drawEllipse(-3, -3, 6, 6); // draw an ellipse
	painter.translate(-4, -59);

	painter.translate(15, 58);
	painter.drawEllipse(-3, -3, 6, 6); // draw an ellipse
	painter.translate(-15, -58);

	//painter.setBrush(QBrush(Qt::black));
	//painter.drawEllipse(20,20, 160,160);
	painter.end();

	QLinearGradient topbarbrush(QPointF(0, 0), QPointF(1024, 50));
	topbarbrush.setColorAt(0, QColor(51, 103, 153));
	topbarbrush.setColorAt(1, QColor(202, 204, 255));

	painter.begin(&topbarpic); // paint in picture
	painter.setBrush(topbarbrush);
	painter.setPen(QPen(Qt::black, 3));
	painter.drawRect(0, 0, 1024, 50);
	painter.drawPixmap(0, 0, QPixmap(":/images/logo.PNG", "PNG"));
	painter.end();

	QBrush solidbrush;
	solidbrush.setStyle(Qt::SolidPattern);
	solidbrush.setColor(Qt::red);

	// paint in picture
	painter.begin(&topbarredpic); 
	painter.setBrush(solidbrush);
	painter.setPen(QPen(Qt::red, 3));
	painter.drawRect(0, 0, 1024, 50);
	painter.end();

	solidbrush.setColor(Qt::gray);

	// paint in picture
	painter.begin(&topbargraypic);
	painter.setBrush(solidbrush);
	painter.setPen(QPen(Qt::gray, 3));
	painter.drawRect(0, 0, 1024, 50);
	painter.end();

	backgroundflag = 0;

	ampify = 0.4f;

	//path1.lineTo(0,25);
	path1.lineTo(0, 8);
	path1.arcTo(0, 0, 16, 16, 180, 90);

	path2.moveTo(8, 16);
	path2.arcTo(0, 0, 16, 16, 270, 90);
	path2.lineTo(16, 0);
	//path1.lineTo(50,0);
	QFont myFont;
	myFont.setPointSize(8);

	path3.addText(-2, -3, myFont, "N");
	path3.addText(16, -4, myFont, "S");

	path4[0].addRect(0 * ampify, 0 * ampify, 10 * ampify, 5 * ampify);
	path4[1].addRect(0 * ampify, 5 * ampify, 10 * ampify, 5 * ampify);
	path4[2].addRect(0 * ampify, 10 * ampify, 10 * ampify, 5 * ampify);
	path4[3].addRect(0 * ampify, 15 * ampify, 10 * ampify, 5 * ampify);
	path4[4].addRect(0 * ampify, 20 * ampify, 10 * ampify, 5 * ampify);
	path4[5].addRect(0 * ampify, 25 * ampify, 10 * ampify, 5 * ampify);
	path4[6].addRect(0 * ampify, 30 * ampify, 10 * ampify, 5 * ampify);
	path4[7].addRect(0 * ampify, 35 * ampify, 10 * ampify, 5 * ampify);
	path4[8].addRect(0 * ampify, 40 * ampify, 10 * ampify, 5 * ampify);
	path4[9].addRect(0 * ampify, 45 * ampify, 10 * ampify, 5 * ampify);

	for (int i = 0; i < 16; i++)
	{
		mybump[i].init();
		mybump[i].setbumpid(i);
	}
	myx = 0;
	myy = 0;

	jinbuangel = 0;
	hongfangangel1 = 0;
	hongfangangel2 = 0;
	baibiangel = 0;
	baibidir = 1;

	QBitmap tempbmp;
	tempbmp.load(":/images/action_mask.PNG");

	ui.pushButton_3->setIcon(QIcon(":/images/xiawangbt.PNG"));
	ui.pushButton_3->setIconSize(QSize(88, 88));
	ui.pushButton_3->setMask(tempbmp.scaledToHeight(88));
	ui.pushButton_3->hide();


	ui.pushButton_4->setIcon(QIcon(":/images/citaikaiqibt.PNG"));
	ui.pushButton_4->setIconSize(QSize(88, 88));
	ui.pushButton_4->setMask(tempbmp.scaledToHeight(88));
	ui.pushButton_4->hide();

	ui.pushButton_7->setIcon(QIcon(":/images/duiling.PNG"));
	ui.pushButton_7->setIconSize(QSize(88, 88));
	ui.pushButton_7->setMask(tempbmp.scaledToHeight(88));
	ui.pushButton_7->hide();

	ui.pushButton_16->setIcon(QIcon(":/images/taiwangbt.PNG"));
	ui.pushButton_16->setIconSize(QSize(88, 88));
	ui.pushButton_16->setMask(tempbmp.scaledToHeight(88));
	ui.pushButton_16->hide();

	ui.pushButton_17->setIcon(QIcon(":/images/citaiguanbibt.PNG"));
	ui.pushButton_17->setIconSize(QSize(88, 88));
	ui.pushButton_17->setMask(tempbmp.scaledToHeight(88));
	ui.pushButton_17->hide();

	ui.pushButton_18->setIcon(QIcon(":/images/fanzhuan.PNG"));
	ui.pushButton_18->setIconSize(QSize(88, 88));
	ui.pushButton_18->setMask(tempbmp.scaledToHeight(88));
	ui.pushButton_18->hide();

	ui.pushButton_6->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_8->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_5->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_10->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_12->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_13->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_2->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:24px 楷体_GB2312; color:rgb(0,0,127)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_9->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:24px 楷体_GB2312; color:rgb(0,0,127)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	ui.pushButton_11->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:24px 楷体_GB2312; color:rgb(0,0,127)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_14->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:60px 楷体_GB2312; color:rgb(0,0,128)} QPushButton:hover {background-color: rgb(208,208,208);  color:rgb(0,0,128); border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_15->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:60px 楷体_GB2312; color:rgb(0,0,128)} QPushButton:hover {background-color: rgb(208,208,208);  color:rgb(0,0,128); border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.label_7->setStyleSheet("font:28px bold 楷体_GB2312");
	ui.label_9->setStyleSheet("font:28px bold 楷体_GB2312");
	ui.label_10->setStyleSheet("font:28px bold 楷体_GB2312");
	ui.label_8->setStyleSheet("font:28px bold 楷体_GB2312 ");
	ui.label_11->setStyleSheet("font:28px bold 楷体_GB2312 ");

	ui.pushButton_3->setStyleSheet(
		" QPushButton:hover {background-image: url(:/images/up_normal.png);background-color: rgb(208,208,208);  color:rgb(0,255,0); border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	printchgspeedflag = false;

	//网头上抬和下放状态 0：下放状态      1：上抬状态
	updownstatus = 0;

	//磁台的磁力显示
	for(int i =0; i < 16; i++)
	{
		magicpercent[i] = new le(this);
		magicpercent[i]->setText(QString::number(pubcom.magicpercent[i]) + "%");
		magicpercent[i]->setGeometry(175+i*31,600,30,20);
		magicpercent[i]->setVisible(false);
	}

	//磁台的磁力显示
	for(int i =0; i < pubcom.PrintNum; i++)
	{
		magicpercent[i]->setVisible(true);
	}

	//状态图标
	PrintStatuspix[0] = new QPixmap(":/images/xiumian.PNG","PNG");
	PrintStatuspix[1] = new QPixmap(":/images/shezhi.PNG","PNG");
	PrintStatuspix[2] = new QPixmap(":/images/shoudong.PNG","PNG");
	PrintStatuspix[3] = new QPixmap(":/images/zhunbei.PNG","PNG");
	PrintStatuspix[4] = new QPixmap(":/images/yunxing.PNG","PNG");
	PrintStatuspix[5] = new QPixmap(":/images/jiting.PNG","PNG");
	PrintStatuspix[6] = new QPixmap(":/images/guzhang.PNG","PNG");

	for (int i = 0; i < 7; i++)
	{
		PrintStatusImg[i] = (*(PrintStatuspix[i])).scaled(QSize(30,30),Qt::KeepAspectRatio,Qt::SmoothTransformation);
	}

	//状态显示
	for(int i =0; i < 16; i++)
	{
		UIprintstatus[i] = 1;
		PrintStatusLb[i] = new QLabel(this);
		PrintStatusLb[i]->setGeometry(160+i*31,400,30,30);
		PrintStatusLb[i]->setPixmap(PrintStatusImg[0]);
		PrintStatusLb[i]->setVisible(false);
	}

	for(int i =0; i < pubcom.PrintNum; i++)
	{
		PrintStatusLb[i]->setVisible(true);
	}

	//对零图标
	zeropix[0] = new QPixmap(":/images/zero0.PNG", "PNG");
	zeropix[1] = new QPixmap(":/images/zero1.PNG", "PNG");

	//对零显示
	for(int i =0; i < 16; i++)
	{
		zerolb[i] = new QLabel(this);
		zerolb[i]->setGeometry(170+i*31,440,30,30);
		zerolb[i]->setPixmap(*(zeropix[0]));
		zerolb[i]->setVisible(false);
	}

	for(int i =0; i < pubcom.PrintNum; i++)
	{
		zerolb[i]->setVisible(true);
	}

	//从历史记录中恢复
	mydb.GetValue(QString("setworknumber") ,setworknumber);
	mydb.GetValue(QString("curworknumber") ,curworknumber);

	unsigned int PulsePerMeter = 0;

	ksmc->GetPulsePerMeter(PulsePerMeter);

	sumcount = (unsigned long long)curworknumber * (unsigned long long)PulsePerMeter * (unsigned long long)ENCODERMETER;

	newcount = 0;
	oldcount = 0;

	ui.lcdNumber_2->display( curworknumber );
	ui.lcdNumber_3->display( setworknumber );

	ui.movebackground->setPixmap(QPixmap::fromImage((*backgroundimg)));

	movelabel = new mylabel(this);
	movelabel->setGeometry(QRect(0,400,1024,205));

	topbar = new QLabel(this);
	topbar->setGeometry(QRect(0,0,1024,50));
	topbar->setPicture(topbarpic);

	datetime = new QLabel(topbar);
	datetime->setGeometry(QRect(790,10,231,31));
	datetime->setStyleSheet(datetimestyle);

	Cautiondisplay = new QLabel(topbar);
	Cautiondisplay->setGeometry(QRect(400,0,400,50));
	Cautiondisplay->setStyleSheet(cautionstyle);

	LicOverDisplay = new QLabel(this);
	LicOverDisplay->setGeometry(0,51,900,50);
	LicOverDisplay->setStyleSheet(cautionstyle);

	savecurworknumbertimer = 0;

	circleflag = false;

	OldDataTime = QString("");

	HisCautionStatus = false;
	HisCautionStatus2 = false;

#ifdef TEST
	pb_testcircle = new QPushButton(this);
	pb_testcircle->setGeometry(QRect(130, 300, 111, 51));
	pb_testcircle->setText("自转");
	pb_testcircle->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	connect(pb_testcircle,SIGNAL(clicked()),this,SLOT(action_testcircle()));

	pb_testfollow = new QPushButton(this);
	pb_testfollow->setGeometry(QRect(330, 300, 111, 51));
	pb_testfollow->setText("跟随");
	pb_testfollow->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	connect(pb_testfollow,SIGNAL(clicked()),this,SLOT(action_testfollow()));

	//是否进入测试反转标识
	InTestCircle  = false;

	//是否进入跟随
	InFollow = false;

	//是否在一个自转动作中
	InOneCircleAction = false;

#endif
	licovertimestatus = 0;
	hislicovertimestatus = 0;
#ifdef HAVEDOG
	//加密狗查询设定的值
	int multi = 32767 / 600;
	dogquerytimerset = rand() / multi + 1; //+1是防止出现时间为0

	//加密狗查询计时器
	dogquerytimer = 0;
	//加密狗查询失败计数
	dogqueryfailcount = 0;

	char tdate[8];
	safenetinst.readdata(&dogdateflag,0,1);
	safenetinst.readdata(tdate,1,8);

	licensedata = QDate::fromString(QString::fromAscii(tdate,8),"yyyyMMdd");

	checklictimer = 0;

	int year,month,day;
	safenetinst.GetDogDate(&year,&month,&day);

	if (dogdateflag == '1')
	{
		if (licensedata < QDate(year,month,day) &&
			QDate(year,month,day) < licensedata.addMonths(1))
		{
			licovertimestatus = 1;
		}
		else if (licensedata.addMonths(1) < QDate(year,month,day))
		{
			licovertimestatus = 2;
		}
	}


#endif

}

tech::~tech()
{
	delete ksmc;

}

//定时处理的任务
void tech::timeupdate()
{
	//定时器刷新
	pubtimermng.flush();

	//can数据时间戳刷新
	candevice.refreshtime();

	//刷新寄存器区心跳计数
	FlushRegsReadHeatbeat(&regs);

	if (pubcom.PowerOff == true)
	{
		this->accept();
	}

	//串口心跳标识
	if (SerialHeartBeatFlag == true)
	{
		WritePcSetHeartBeatBit(&regs,true);
		SerialHeartBeatFlag = false;
	}
	else
	{
		WritePcSetHeartBeatBit(&regs,false);
		SerialHeartBeatFlag = true;
	}


#ifdef HAVEDOG
	//加密狗查询计时器
	if (dogquerytimer < dogquerytimerset)
	{
		dogquerytimer ++;
	}
	else
	{
		if (safenetinst.AESreqack() != 0)
		{
			dogqueryfailcount ++;
		}
		else//只要成功一次，即刻清为0
		{
			dogqueryfailcount = 0;
		}
		//生成新的随机查询时间
		int multi = 32767 / 600;
		dogquerytimerset = rand() / multi + 1; //+1是防止出现时间为0
	}


	if (dogqueryfailcount >= 3 )
	{
		timer->disconnect(this);

		//确保数据刷新到数据库文件中
		while (mydb.SQLQueuechg == true 
			|| mydb.SQLQueuechg2 == true)
		{
			;
		}

		pubcom.quitthread = true;

		QMessageBox msgBox;
		msgBox.setText("没有检测到加密狗！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		this->accept();

	}



	//查看日期
	if (dogdateflag == '1')
	{
		//查看日期是否到期
		if (checklictimer < 36000)
		{
			checklictimer ++;
		} 
		else
		{
			checklictimer = 0;
			int year,month,day;
			safenetinst.GetDogDate(&year,&month,&day);
			if (licensedata < QDate(year,month,day) &&
				QDate(year,month,day) < licensedata.addMonths(1))
			{
				hislicovertimestatus = licovertimestatus;
				licovertimestatus = 1;
			}
			else if (licensedata.addMonths(1) < QDate(year,month,day))
			{
				hislicovertimestatus = licovertimestatus;
				licovertimestatus = 2;
			}
		}
	} 

#endif



	//故障背景闪烁计数器
	if (publiccaution.count() >= 1
		&& pubcom.delcauflag == false)
	{
		//故障闪烁
		if (timecount == 10)
		{
			if (cautionbkflag == 1)
			{
				cautionbkflag = 0;
			}
			else
			{
				cautionbkflag = 1;
			}
			timecount = 0;
			cautionbkchgflag = 1;
		}
		else
		{
			timecount++;
		}

		HisCautionStatus = true;
	}
	else
	{
		if (HisCautionStatus == true)
		{
			cautionbkchgflag = 1;
		}

		HisCautionStatus = false;
	}

	//故障字符显示计时器
	if (publiccaution.count() >= 1
		&& pubcom.delcauflag == false)
	{
		if (cautiondispcount == 20)
		{
			cautiondispcount = 0;

			if(curcautionindex < publiccaution.count()-1)
			{
				curcautionindex++;
			}
			else
			{
				curcautionindex = 0;
			}
			cautionstrchgflag = 1;
		}
		else
		{
			cautiondispcount++;
		}
		HisCautionStatus2 = true;
	}
	else
	{
		if (HisCautionStatus2 == true)
		{
			cautionstrchgflag = 1;
		}
		curcautionindex = 0;
		HisCautionStatus2 = false;
	}

	//刷新日期和时间
	if (OldDataTime != QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
	{
		datetimechgflag = 1;
		OldDataTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	}

	if ( hislicovertimestatus != 0 && licovertimestatus == 0)
	{
		LicOverDisplay->clear();
	}
	else if (hislicovertimestatus != 1 && licovertimestatus == 1)
	{
		LicOverDisplay->setText("产品许可证到期,请拨厂家电话021-56035777！");
	}
	//许可证过期退出
	else if(hislicovertimestatus != 2 && licovertimestatus == 2)
	{
		timer->disconnect(this);

		//确保数据刷新到数据库文件中
		while (mydb.SQLQueuechg == true 
			|| mydb.SQLQueuechg2 == true)
		{
			;
		}

		pubcom.quitthread = true;

		QMessageBox msgBox;
		msgBox.setText("产品使用期限到，请联系厂家！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		this->accept();
	}

	//圆网每毫米的脉冲数
	double pulseperunit = 0.0;
	ksmc->GetPulsePerUnit(pulseperunit);

		//初始化后才能开始读运动卡
	if (pubcom.initflag == 1)
	{

#ifdef TEST
		//在自转的时候，不获取主马达的速度
		if (InTestCircle == true 
			|| InOneCircleAction == true)
		{

		} 
		else
		{
			ksmc->GetMainMotorSpeed(pubcom.motorspeed);
		}
#else
		ksmc->GetMainMotorSpeed(pubcom.motorspeed);
#endif
		//主马达速度改变，发送速度给网头
		if(fabs(pubcom.motorspeed - oldmotorspeed) >= 1)
		{
			pubcom.MotorSpeedChgFlag = true;
			oldmotorspeed = pubcom.motorspeed;
		}
#ifdef TEST
		if(newcount == 0 )
#else
		if(newcount == 0 
			&& pubcom.machprintstatus == 2 
			&& pubcom.motorspeed > 1) //橡毯速度最小速度必须为1以上才可以采集
#endif
		{
			//老计数
			ksmc->GetEncoderCount(newcount);
		}
#ifdef TEST
		else
#else
		else if (pubcom.machprintstatus == 2
			&& pubcom.motorspeed > 1)//橡毯速度最小速度必须为1以上才可以采集
#endif
		{
			oldcount = newcount;
			ksmc->GetEncoderCount(newcount);


			//新值小于小于旧值，说明可能翻转了
			if (newcount < oldcount)
			{
				//真正翻转，而不是编码器回退,回退脉冲不可能超过3000000000
				if (newcount < 1000000000 && oldcount > 4000000000)
				{
					//位移距离校验，在100ms情况下面，按最大200m/分钟，最大位移为200*1000/60 *0.1 = 333毫米
					//if(abs((4294967295 - oldcount + newcount)/pulseperunit) < 2000)
					//{
						//4294967295是32位整形的最大值
						sumcount += 4294967295 - oldcount + newcount;
					//}
				}
				//编码器回退不纳入计算
				/*
				else
				{
				sumcount += newcount - oldcount;
				}
				*/
			}
			else
			{
				//位移距离校验，在100ms情况下面，按最大200m/分钟，最大位移为200*1000/60 *0.1 = 333毫米
				//if(abs((newcount - oldcount)/pulseperunit) < 2000)
				{
					sumcount += newcount - oldcount;
				}
			}
		}
		unsigned int PulsePerMeter = 0;

		ksmc->GetPulsePerMeter(PulsePerMeter);

		if ( PulsePerMeter != 0)
		{
			if ( curworknumber != ((int)(sumcount/PulsePerMeter)/ENCODERMETER) )
			{
				curworknumber = ((int)(sumcount/PulsePerMeter)/ENCODERMETER);

				//是否要保存变量
				savecurworknumber = true;
			}
		}

		ui.lcdNumber_2->display( (curworknumber) ); 

		//产量是否完成
		if (curworknumber >= setworknumber)
		{
			WritePcWorkFinBit(&regs,true);
		}
		else
		{
			WritePcWorkFinBit(&regs,false);
		}
	}

	//工作量写
	WritePcWorkRecDWord(&regs,curworknumber);

	//10秒保存一次当前工作量，便于下次记起
	if (savecurworknumbertimer < 100)
	{
		savecurworknumbertimer++;
	}
	else
	{
		savecurworknumbertimer = 0;

		//是否有改变，有改变则保存
		if (savecurworknumber == true)
		{
			mydb.SaveValue("curworknumber",curworknumber);
			savecurworknumber = false;
		}
	}


	//主电机速度
	if (pubcom.CurrentWnd == 0)
	{

#ifndef TEST
		//主电机速度为 米/分钟 ，最大的速度120米/分钟，对应30度
		//最后得出的速度为 ××度/分钟
		speed = ((float)pubcom.motorspeed/120)*30;

		//解决低速不转的问题
		if(speed > 0.2 && speed < 1.2)
		{
			speed = 1;
		}
		else if (speed < -0.2 && speed > -1.2)
		{
			speed = -1;
		}
#else
		if (InTestCircle == true 
			|| InOneCircleAction == true)
		{

		} 
		else
		{
			//主电机速度为 米/分钟 ，最大的速度120米/分钟，对应30度
			//最后得出的速度为 ××度/分钟
			speed = ((float)pubcom.motorspeed/120)*30;

			//解决低速不转的问题
			if(speed > 0.2 && speed < 1.2)
			{
				speed = 1;
			}
			else if (speed < -0.2 && speed > -1.2)
			{
				speed = -1;
			}
		}
#endif

		//旋转角度
		if (rolldir == 0)
		{
			if ((angel + speed) >= 360)
			{
				angel = 0;
			}
			else
			{
				angel += speed;
			}
		}
		else
		{
			if ((angel - speed) <= -360)
			{
				angel = 0;
			}
			else
			{
				angel -= speed;
			}
		}


		//圆网
		for (int i = 0; i < pubcom.PrintNum; i++)
		{
			double tempspeed = 0.0;


			if (ksmc->GetMotorSpeed(i+1,tempspeed) == true)
			{
				//马达速度
				//每分钟脉冲数 / 每米脉冲数 得出 每分钟的米数 即速度
				//最大的速度为120米/分钟 ，最大的速度的时候，对应的角速度为 30度
				//最终的速度为 ××度/每分钟 
				motorspeed[i] = ((float)tempspeed * 1000 * 60 / (pulseperunit * 1000)) /120 * 30;

#ifdef TEST
				//当在自转的时候，或者在动作中的时候，将网头1的速度显示在界面上
				if (i == 0 
					&& ( InTestCircle == true || InOneCircleAction == true)
					)
				{
					pubcom.motorspeed = -(tempspeed * 1000 * 60 / (pulseperunit * 1000));
				}
#endif
			}
			//解决低速不转的问题
			if(motorspeed[i] > 0.2 && motorspeed[i] < 1.2)
			{
				motorspeed[i] = 1;
			}
			else if (motorspeed[i] < -0.2 && motorspeed[i] > -1.2)
			{
				motorspeed[i] = -1;
			}

			//旋转角度
			if (rolldir == 0)
			{
				if ((motorangel[i] + motorspeed[i]) >= 360)
				{
					motorangel[i] = 0;
				}
				else
				{
					motorangel[i] += motorspeed[i];
				}
			}
			else
			{
				if ((motorangel[i] - motorspeed[i]) <= -360)
				{
					motorangel[i] = 0;
				}
				else
				{
					motorangel[i] -= motorspeed[i];
				}
			}
		}

		//上下网头动作触发
		for(int i = 0; i < 16;i++)
		{
			if(pubcom.updownstatus[i] == 1 && upflag[i] == 0)
			{
				upflag[i] = 1;
			}
			else if(pubcom.updownstatus[i] == 0 && upflag[i] == 2)
			{
				upflag[i] = 3;
			}
		}

		for(int i= 0;i < 16;i++)
		{
			//上抬网
			if (upflag[i] == 1)
			{
				if ((updistance[i] + upspeed) <= upmax)
				{
					updistance[i] += upspeed;
				}
				else
				{
					upflag[i] = 2;
				}
			}

			//下降网
			if (upflag[i] == 3)
			{
				if ((updistance[i] - upspeed) >= 0)
				{
					updistance[i] -= upspeed;
				}
				else
				{
					upflag[i] = 0;
				}
			}
		}


		//故障背景变化了
		if (publiccaution.count() >= 1
			&& pubcom.delcauflag == false)
		{
			if (cautionbkchgflag == 1)
			{
				if (cautionbkflag == 1)
				{
					topbar->setPicture(topbargraypic);
				}
				else
				{
					topbar->setPicture(topbarredpic);
				}
				cautionbkchgflag = 0;
			}
		}
		else
		{
			if (cautionbkchgflag == 1)
			{
				topbar->setPicture(topbarpic);
				cautionbkchgflag = 0;
			}
		}

		//故障字符变化了
		if (publiccaution.count() >= 1
			&& pubcom.delcauflag == false)
		{
			if (cautionstrchgflag == 1)
			{
				t_caution caution;
				publiccaution.getcautionbyindex(curcautionindex,caution);
				Cautiondisplay->setText(pubcom.cautionlocationidstrmap.value(caution.location) + pubcom.cautionidstrmap.value(caution.code));
				cautionstrchgflag = 0;
			}
		}
		else
		{
			if (cautionstrchgflag == 1)
			{
				Cautiondisplay->setText("");
				cautionstrchgflag = 0;
			}
		}


		//日期变化了
		if (datetimechgflag == 1)
		{
			datetimechgflag = 0;
			datetime->setText(QDateTime::currentDateTime().toString(
				"yyyy-MM-dd hh:mm:ss"));
		}

		//只有屏保开的时候才处理
		if (screemsaverflag == 1)
		{
			screentimer++;
			if (screentimer == screemsavertime)
			{
				screenwork = 1;
				SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
			}

			if (screentimer < screemsavertime)
			{
				if (screenwork == 1)
				{
					screenwork = 0;
					SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1);
				}
			}
		}

		if (backflushtimer < 10)
		{
			backflushtimer++;
		}
		else
		{
			backflushtimer = 0;
		}

		if(pubcom.PlcLuobuyunxinUpFlag == true)
		{
			if (baibidir == 1)
			{
				if (baibiangel < 10)
					baibiangel++;
				else
					baibidir = 0;
			}
			else
			{
				if (baibiangel > -5)
					baibiangel--;
				else
					baibidir = 1;
			}
		}

		if(pubcom.PlcJinbuyunxinUpFlag == true)
		{
			if ((jinbuangel + 10) >= 360)
			{
				jinbuangel = 0;
			}
			else
			{
				jinbuangel += 10;
			}
		}

		if(pubcom.PlcHongfangwangdai1yunxinUpFlag == true)
		{
			if ((hongfangangel1 + 10) >= 360)
			{
				hongfangangel1 = 0;
			}
			else
			{
				hongfangangel1 += 10;
			}
		}

		if(pubcom.PlcHongfangwangdai2yunxinUpFlag == true)
		{
			if ((hongfangangel2 + 10) >= 360)
			{
				hongfangangel2 = 0;
			}
			else
			{
				hongfangangel2 += 10;
			}
		}

		//显示产量
		ui.lcdNumber_3->display(setworknumber);

		ui.lcdNumber->display(pubcom.motorspeed);

		//磁台百分比
		for(int i =0; i < pubcom.PrintNum; i++)
		{
			magicpercent[i]->setText(QString::number(pubcom.magicpercent[i]) + "%");
			//magicpercent[i]->setText(QString::number(100) + "%");
		}

		//网头状态
		for(int i =0; i < pubcom.PrintNum; i++)
		{
			if (pubcom.presetprintstatus[i] != UIprintstatus[i])
			{
				UIprintstatus[i] = pubcom.presetprintstatus[i];

				switch (UIprintstatus[i])
				{
				case 1:
					PrintStatusLb[i]->setPixmap(PrintStatusImg[0]);
					break;

				case 3:
					PrintStatusLb[i]->setPixmap(PrintStatusImg[4]);
					break;

				case 4:
					PrintStatusLb[i]->setPixmap(PrintStatusImg[3]);
					break;
				}
			}
		}

		//对零状态
		for(int i =0; i < pubcom.PrintNum; i++)
		{
			if(pubcom.zerostatus[i] == 1 )
			{
				zerolb[i]->setPixmap(*(zeropix[1]));
			}
			else
			{
				zerolb[i]->setPixmap(*(zeropix[0]));
			}
		}

		QDomNode para = GetParaByName("machsub1", "网头调速允许按钮是否工作");
		bool ok;
		bool btwork = para.firstChildElement("value").text().toInt(&ok);

		if (btwork == false)
		{
			ui.pushButton_13->setEnabled(false);
		}
		else
		{
			ui.pushButton_13->setEnabled(true);
		}

		QDomNode paranode = GetParaByName("prod", QString("网头调速允许"));

		if (paranode.firstChildElement("value").text().toInt(&ok)
			== 1)
		{
			ui.pushButton_13->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(37,218,41) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(0,255,0);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(0,255,0);} ");
			ui.pushButton_13->setText(QString("调速开"));

		}
		else
		{
			ui.pushButton_13->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
			ui.pushButton_13->setText(QString("调速关"));
		}


		QPoint leftup;
		QPoint rightdown;

		//当前鼠标位置
		QPoint curMouseLocation;
		curMouseLocation = QCursor::pos();


		leftup = ui.pushButton_3->mapToGlobal(QPoint(0,0));
		rightdown = ui.pushButton_3->mapToGlobal(QPoint(ui.pushButton_3->width(),ui.pushButton_3->height()));

		if (( OldMouseLocation.x() < leftup.x()
			||	OldMouseLocation.x() > rightdown.x()
			|| OldMouseLocation.y() < leftup.y() 
			||	OldMouseLocation.y() > rightdown.y())
			&&
			( curMouseLocation.x() > leftup.x()
			&&	curMouseLocation.x() < rightdown.x()
			&& curMouseLocation.y() > leftup.y() 
			&&	curMouseLocation.y() < rightdown.y()))
		{
			ui.pushButton_3->setIcon(QIcon(":/images/xiawangbt_on.PNG"));
		}

		if (( curMouseLocation.x() < leftup.x()
			||	curMouseLocation.x() > rightdown.x()
			|| curMouseLocation.y() < leftup.y() 
			||	curMouseLocation.y() > rightdown.y())
			&&
			( OldMouseLocation.x() > leftup.x()
			&&	OldMouseLocation.x() < rightdown.x()
			&& OldMouseLocation.y() > leftup.y() 
			&&	OldMouseLocation.y() < rightdown.y()))
		{
			ui.pushButton_3->setIcon(QIcon(":/images/xiawangbt.PNG"));
		}

		leftup = ui.pushButton_4->mapToGlobal(QPoint(0,0));
		rightdown = ui.pushButton_4->mapToGlobal(QPoint(ui.pushButton_4->width(),ui.pushButton_4->height()));

		if (( OldMouseLocation.x() < leftup.x()
			||	OldMouseLocation.x() > rightdown.x()
			|| OldMouseLocation.y() < leftup.y() 
			||	OldMouseLocation.y() > rightdown.y())
			&&
			( curMouseLocation.x() > leftup.x()
			&&	curMouseLocation.x() < rightdown.x()
			&& curMouseLocation.y() > leftup.y() 
			&&	curMouseLocation.y() < rightdown.y()))
		{
			ui.pushButton_4->setIcon(QIcon(":/images/citaikaiqibt_on.PNG"));
		}

		if (( curMouseLocation.x() < leftup.x()
			||	curMouseLocation.x() > rightdown.x()
			|| curMouseLocation.y() < leftup.y() 
			||	curMouseLocation.y() > rightdown.y())
			&&
			( OldMouseLocation.x() > leftup.x()
			&&	OldMouseLocation.x() < rightdown.x()
			&& OldMouseLocation.y() > leftup.y() 
			&&	OldMouseLocation.y() < rightdown.y()))
		{
			ui.pushButton_4->setIcon(QIcon(":/images/citaikaiqibt.PNG"));
		}

		leftup = ui.pushButton_7->mapToGlobal(QPoint(0,0));
		rightdown = ui.pushButton_7->mapToGlobal(QPoint(ui.pushButton_7->width(),ui.pushButton_7->height()));

		if (( OldMouseLocation.x() < leftup.x()
			||	OldMouseLocation.x() > rightdown.x()
			|| OldMouseLocation.y() < leftup.y() 
			||	OldMouseLocation.y() > rightdown.y())
			&&
			( curMouseLocation.x() > leftup.x()
			&&	curMouseLocation.x() < rightdown.x()
			&& curMouseLocation.y() > leftup.y() 
			&&	curMouseLocation.y() < rightdown.y()))
		{
			QString name = QString(":/images/duiling_on.PNG");
			ui.pushButton_7->setIcon(QIcon(name));
		}

		if (( curMouseLocation.x() < leftup.x()
			||	curMouseLocation.x() > rightdown.x()
			|| curMouseLocation.y() < leftup.y() 
			||	curMouseLocation.y() > rightdown.y())
			&&
			( OldMouseLocation.x() > leftup.x()
			&&	OldMouseLocation.x() < rightdown.x()
			&& OldMouseLocation.y() > leftup.y() 
			&&	OldMouseLocation.y() < rightdown.y()))
		{
			QString name = QString(":/images/duiling.PNG");
			ui.pushButton_7->setIcon(QIcon(name));
		}

		leftup = ui.pushButton_16->mapToGlobal(QPoint(0,0));
		rightdown = ui.pushButton_16->mapToGlobal(QPoint(ui.pushButton_16->width(),ui.pushButton_16->height()));

		if (( OldMouseLocation.x() < leftup.x()
			||	OldMouseLocation.x() > rightdown.x()
			|| OldMouseLocation.y() < leftup.y() 
			||	OldMouseLocation.y() > rightdown.y())
			&&
			( curMouseLocation.x() > leftup.x()
			&&	curMouseLocation.x() < rightdown.x()
			&& curMouseLocation.y() > leftup.y() 
			&&	curMouseLocation.y() < rightdown.y()))
		{
			ui.pushButton_16->setIcon(QIcon(":/images/taiwangbt_on.PNG"));
		}

		if (( curMouseLocation.x() < leftup.x()
			||	curMouseLocation.x() > rightdown.x()
			|| curMouseLocation.y() < leftup.y() 
			||	curMouseLocation.y() > rightdown.y())
			&&
			( OldMouseLocation.x() > leftup.x()
			&&	OldMouseLocation.x() < rightdown.x()
			&& OldMouseLocation.y() > leftup.y() 
			&&	OldMouseLocation.y() < rightdown.y()))
		{
			ui.pushButton_16->setIcon(QIcon(":/images/taiwangbt.PNG"));
		}

		leftup = ui.pushButton_17->mapToGlobal(QPoint(0,0));
		rightdown = ui.pushButton_17->mapToGlobal(QPoint(ui.pushButton_17->width(),ui.pushButton_17->height()));

		if (( OldMouseLocation.x() < leftup.x()
			||	OldMouseLocation.x() > rightdown.x()
			|| OldMouseLocation.y() < leftup.y() 
			||	OldMouseLocation.y() > rightdown.y())
			&&
			( curMouseLocation.x() > leftup.x()
			&&	curMouseLocation.x() < rightdown.x()
			&& curMouseLocation.y() > leftup.y() 
			&&	curMouseLocation.y() < rightdown.y()))
		{
			ui.pushButton_17->setIcon(QIcon(":/images/citaiguanbibt_on.PNG"));
		}

		if (( curMouseLocation.x() < leftup.x()
			||	curMouseLocation.x() > rightdown.x()
			|| curMouseLocation.y() < leftup.y() 
			||	curMouseLocation.y() > rightdown.y())
			&&
			( OldMouseLocation.x() > leftup.x()
			&&	OldMouseLocation.x() < rightdown.x()
			&& OldMouseLocation.y() > leftup.y() 
			&&	OldMouseLocation.y() < rightdown.y()))
		{
			ui.pushButton_17->setIcon(QIcon(":/images/citaiguanbibt.PNG"));
		}

		leftup = ui.pushButton_18->mapToGlobal(QPoint(0,0));
		rightdown = ui.pushButton_18->mapToGlobal(QPoint(ui.pushButton_18->width(),ui.pushButton_18->height()));

		if (( OldMouseLocation.x() < leftup.x()
			||	OldMouseLocation.x() > rightdown.x()
			|| OldMouseLocation.y() < leftup.y() 
			||	OldMouseLocation.y() > rightdown.y())
			&&
			( curMouseLocation.x() > leftup.x()
			&&	curMouseLocation.x() < rightdown.x()
			&& curMouseLocation.y() > leftup.y() 
			&&	curMouseLocation.y() < rightdown.y()))
		{
			if (circleflag == true)
			{
				QString name = QString(":/images/fanzhuan_ing_on.PNG");
				ui.pushButton_18->setIcon(QIcon(name));
			}
			else
			{
				QString name = QString(":/images/fanzhuan_on.PNG");
				ui.pushButton_18->setIcon(QIcon(name));
			}
		}

		if (( curMouseLocation.x() < leftup.x()
			||	curMouseLocation.x() > rightdown.x()
			|| curMouseLocation.y() < leftup.y() 
			||	curMouseLocation.y() > rightdown.y())
			&&
			( OldMouseLocation.x() > leftup.x()
			&&	OldMouseLocation.x() < rightdown.x()
			&& OldMouseLocation.y() > leftup.y() 
			&&	OldMouseLocation.y() < rightdown.y()))
		{
			if (circleflag == true)
			{
				QString name = QString(":/images/fanzhuan_ing.PNG");
				ui.pushButton_18->setIcon(QIcon(name));
			}
			else
			{
				QString name = QString(":/images/fanzhuan.PNG");
				ui.pushButton_18->setIcon(QIcon(name));
			}
		}


		OldMouseLocation = curMouseLocation;
		movelabel->update();


#ifdef TEST
		if (InTestCircle == true)
		{
			//没有做动作，进入动作
			if (InOneCircleAction == false)
			{
				QList<int>::iterator it;

				pubcom.activeprintlistmutex.lockForRead();

				//激活的网头
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//不是在准备状态才自转
					if (pubcom.presetprintstatus[*it] != 4)
					{
						ksmc->test((*it) + 1,3,50,200);
					}
				}
				pubcom.activeprintlistmutex.unlock();
				InOneCircleAction = true;

				//延迟，避免运动卡刷新速率太慢
				Sleep(200);
			} 
		}

		//动作做完，设置标识位
		if (InOneCircleAction == true)
		{
			bool notfinished = false;
			int status[16];
			memset(status,0,16);

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//激活的网头
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				//不是在准备状态才问是否完成
				if (pubcom.presetprintstatus[*it] != 4)
				{
					ksmc->GetMotorInPosStatus(*it + 1,status[*it]);
					if (status[*it] != 1)
					{
						notfinished = true;
						break;
					}
				}
			}
			pubcom.activeprintlistmutex.unlock();

			if ( notfinished == false
				)
			{
				//标识
				InOneCircleAction = false;
			}
		}

#endif

	}
}


//下网
void tech::on_pushButton_3_clicked()
{
	interfaceevent* ptempevent = new interfaceevent();

	//只有在上下标识空闲的时候才处理按钮
	if (printopflag != true)
	{
		publiccaution.addevent("主界面","下网","用户按下网按钮",1);

		ptempevent->cmd = 0x02;//网头操作命令
		ptempevent->status = 0x00;//命令状态
		ptempevent->data[0] = 0x04;//小命令字，下放还是上抬

		printopflag = true;
		//发送一个事件给后台线程
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}

//磁台开启
void tech::on_pushButton_4_clicked()
{
	interfaceevent* ptempevent = new interfaceevent();

	//只有在反转标识空闲的时候才处理按钮
	if (printopflag != true)
	{
		publiccaution.addevent("主界面","磁台开启","用户按磁台开启按钮",1);

		ptempevent->cmd = 0x02;//网头操作命令
		ptempevent->status = 0x00;//命令状态
		ptempevent->data[0] = 0x07;//磁台开启
		printopflag = true;

		//发送一个事件给界面线程
		QCoreApplication::postEvent(backendobject, ptempevent);
	}

}

//对零
void tech::on_pushButton_7_clicked()
{
	//运行状态，不允许对零
	if (pubcom.machprintstatus == 2)
	{
		return;
	}

	//急停或者欠压不允许对零
	if(publiccaution.hascaution(PLCLOCATION, PLCHALTCATION))
	{
		return;
	}

	//if(circleflag == true)
	//	return;

	QMessageBox msgBox;
	msgBox.setText("<font size = 4>对零期间不可操作，真的要对零么？</font>");
	msgBox.setWindowTitle("对零");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("是");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("否");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes)
	{
		publiccaution.addevent("主界面","对零","用户按对零按钮",1);

		interfaceevent* ptempevent = new interfaceevent();

		//只有在对零标识空闲的时候才处理按钮
		if (printopflag != true)
		{
			ptempevent->cmd = 0x02;//网头操作命令
			ptempevent->status = 0x00;//命令状态
			ptempevent->data[0] = 0x53;//对零
			printopflag = true;

			//发送一个事件给后台线程
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}
}

//设置
void tech::on_pushButton_5_clicked()
{
	//运行状态，不运行设置
	if (pubcom.machprintstatus == 2)
	{
		return;
	}

	//是否需要弹出密码键盘
	QDomNode para = GetParaByName("needpass", "设置");
	if (para.isNull())
	{
		QMessageBox msgBox;
		msgBox.setText("配置参数为空！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return;
	}

	if (para.firstChildElement("value").text().toInt() == 1)
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
				//取得模块权限表
				int modright = settings.value("prod/modright").toInt();

				//避免进入set界面
				if ((modright & 0x7C) == 0x00 )
				{
					return;
				}

				//根据权限表绘制按钮，并将按钮和处理代码链接
				set Dlg;
				setobject = &Dlg;
				pDlgset = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.SetUserRole(1);
				Dlg.init(modright);
				Dlg.exec();

				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//刷新故障
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}//机器
			else if (settings.value("mach/pass").toString()
				== keyboardinst.password
				&& !settings.value("mach/pass").toString().isNull())
			{

				//取得模块权限表
				int modright = settings.value("mach/modright").toInt();

				//避免进入set界面
				if ((modright & 0x7C) == 0x00 )
				{
					return;
				}

				//根据权限表绘制按钮，并将按钮和处理代码链接
				set Dlg;
				setobject = &Dlg;
				pDlgset = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.SetUserRole(2);
				Dlg.init(modright);
				Dlg.exec();

				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//刷新故障
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}//调试
			else if (settings.value("tune/pass").toString()
				== keyboardinst.password
				&& !settings.value("tune/pass").toString().isNull())
			{
				//取得模块权限表
				int modright = settings.value("tune/modright").toInt();

				//避免进入set界面
				if ((modright & 0x7C) == 0x00 )
				{
					return;
				}

				//根据权限表绘制按钮，并将按钮和处理代码链接
				set Dlg;
				setobject = &Dlg;
				pDlgset = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.SetUserRole(3);
				Dlg.init(modright);
				Dlg.exec();

				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//刷新故障
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}//一般后门
			else if (keyboardinst.password == BACKDOORPASS )
			{
				//在输入密码后隐藏的按钮显示
				ui.pushButton_3->show();
				ui.pushButton_4->show();
				ui.pushButton_7->show();
				ui.pushButton_16->show();
				ui.pushButton_17->show();
				ui.pushButton_18->show();

				//所有的按钮都可以看到
				set Dlg;
				pDlgset = &Dlg;
				setobject = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.SetUserRole(0);
				Dlg.init(127);
				Dlg.exec();

				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//刷新故障
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}//开发后门
			else if (keyboardinst.password == DEVPASS)
			{
				//在输入密码后隐藏的按钮显示
				ui.pushButton_3->show();
				ui.pushButton_4->show();
				ui.pushButton_7->show();
				ui.pushButton_16->show();
				ui.pushButton_17->show();
				ui.pushButton_18->show();

				//根据权限表绘制按钮，并将按钮和处理代码链接
				set Dlg;
				pDlgset = &Dlg;
				setobject = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.SetUserRole(0);
				Dlg.init(255);
				Dlg.exec();

				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//刷新故障
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}
			//密码不正确，普通用户无法设置
			else
			{
				return;
			}
		}

	}
	else
	{
		//所有的按钮都可以看到
		set Dlg;
		setobject = &Dlg;
		pDlgset = &Dlg;
#ifdef ONIPC
		Dlg.setWindowState(Qt::WindowFullScreen);
#endif
		Dlg.setStyleSheet(dlgbkcolor);
		Dlg.setWindowFlags(Qt::FramelessWindowHint);
		Dlg.init(127);
		Dlg.exec();

		//返回主界面了
		pubcom.CurrentWndMutex.lock();
		pubcom.CurrentWnd = 0;
		pubcom.CurrentWndMutex.unlock();
		cautionbkchgflag = 1;
		cautionstrchgflag = 1;

		setobject = NULL;
	}
}

//帮助
void tech::on_pushButton_6_clicked()
{
	//所有的按钮都可以看到
	help Dlg;
	Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
	Dlg.setWindowState(Qt::WindowFullScreen);
#endif
	Dlg.setWindowFlags(Qt::FramelessWindowHint);
	//Dlg.setModal(false);
	Dlg.init(127);
	Dlg.exec();

	//返回主界面了
	pubcom.CurrentWndMutex.lock();
	pubcom.CurrentWnd = 0;
	pubcom.CurrentWndMutex.unlock();
}

//网头操作
void tech::on_pushButton_8_clicked()
{
	//是否需要弹出密码键盘
	QDomNode para = GetParaByName("needpass", "网头操作");
	if (para.isNull())
	{
		QMessageBox msgBox;
		msgBox.setText("配置参数为空！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();
		return;
	}

	if (para.firstChildElement("value").text().toInt() == 1)
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
				//取得模块权限表
				int modright = settings.value("prod/modright").toInt();

				//避免进入操作界面
				if ((modright & 0x01) == 0x00 )
				{
					return;
				}

				//根据权限表绘制按钮，并将按钮和处理代码链接
				opprint Dlg;
				opobject = &Dlg;
				pDlgop = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();
				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

				opobject = NULL;

			}//机器
			else if (settings.value("mach/pass").toString()
				== keyboardinst.password
				&& !settings.value("mach/pass").toString().isNull())
			{
				//取得模块权限表
				int modright = settings.value("mach/modright").toInt();

				//避免进入操作界面
				if ((modright & 0x01) == 0x00 )
				{
					return;
				}

				//根据权限表绘制按钮，并将按钮和处理代码链接
				opprint Dlg;
				opobject = &Dlg;
				pDlgop = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();
				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				opobject = NULL;

			}//调试
			else if (settings.value("tune/pass").toString()
				== keyboardinst.password
				&& !settings.value("tune/pass").toString().isNull())
			{
				//取得模块权限表
				int modright = settings.value("tune/modright").toInt();

				//避免进入操作界面
				if ((modright & 0x01) == 0x00 )
				{
					return;
				}
				//根据权限表绘制按钮，并将按钮和处理代码链接
				opprint Dlg;
				opobject = &Dlg;
				pDlgop = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();

				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

				opobject = NULL;

			}//一般后门
			else if (keyboardinst.password == BACKDOORPASS )
			{
				//在输入密码后隐藏的按钮显示
				ui.pushButton_3->show();
				ui.pushButton_4->show();
				ui.pushButton_7->show();
				ui.pushButton_16->show();
				ui.pushButton_17->show();
				ui.pushButton_18->show();

				//所有的按钮都可以看到
				opprint Dlg;
				opobject = &Dlg;
				pDlgop = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(127);
				Dlg.exec();

				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

				opobject = NULL;

			}//开发后门
			else if (keyboardinst.password == DEVPASS)
			{

				//在输入密码后隐藏的按钮显示
				ui.pushButton_3->show();
				ui.pushButton_4->show();
				ui.pushButton_7->show();
				ui.pushButton_16->show();
				ui.pushButton_17->show();
				ui.pushButton_18->show();

				//根据权限表绘制按钮，并将按钮和处理代码链接
				opprint Dlg;
				opobject = &Dlg;
				pDlgop = &Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(255);
				Dlg.exec();

				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

				opobject = NULL;

			}
			//密码不正确，普通用户无法设置
			else
			{
				return;
			}
		}

	}
	else
	{
		//所有的按钮都可以看到
		opprint Dlg;
		opobject = &Dlg;
		pDlgop = &Dlg;
#ifdef ONIPC
		Dlg.setWindowState(Qt::WindowFullScreen);
#endif
		Dlg.setWindowFlags(Qt::FramelessWindowHint);
		Dlg.init(127);
		Dlg.exec();

		//返回主界面了
		pubcom.CurrentWndMutex.lock();
		pubcom.CurrentWnd = 0;
		pubcom.CurrentWndMutex.unlock();

		opobject = NULL;
	}

}

//故障界面
void tech::mousePressEvent(QMouseEvent * event)
{
	if (event->x() < 1024 && event->x() > 0 && event->y() < 50 && event->y()
	> 0)
	{
		caution Dlg;
		Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
		Dlg.setWindowState(Qt::WindowFullScreen);
#endif
		Dlg.setWindowFlags(Qt::FramelessWindowHint);
		Dlg.init(127);
		Dlg.exec();

		//返回主界面了
		pubcom.CurrentWndMutex.lock();
		pubcom.CurrentWnd = 0;
		pubcom.CurrentWndMutex.unlock();

	}
	QDialog::mousePressEvent(event);
}

//故障
void tech::on_pushButton_10_clicked()
{
	//是否需要弹出密码键盘
	QDomNode para = GetParaByName("needpass", "故障");
	if (para.isNull())
	{
		QMessageBox msgBox;
		msgBox.setText("配置参数为空！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();
		return;
	}

	if (para.firstChildElement("value").text().toInt() == 1)
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
				//取得模块权限表
				int modright = settings.value("prod/modright").toInt();

				//根据权限表绘制按钮，并将按钮和处理代码链接
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();
				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}//机器
			else if (settings.value("mach/pass").toString()
				== keyboardinst.password
				&& !settings.value("mach/pass").toString().isNull())
			{
				//取得模块权限表
				int modright = settings.value("mach/modright").toInt();

				//根据权限表绘制按钮，并将按钮和处理代码链接
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();
				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}//调试
			else if (settings.value("tune/pass").toString()
				== keyboardinst.password
				&& !settings.value("tune/pass").toString().isNull())
			{
				//取得模块权限表
				int modright = settings.value("tune/modright").toInt();

				//根据权限表绘制按钮，并将按钮和处理代码链接
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();
				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}//一般后门
			else if (keyboardinst.password == BACKDOORPASS )
			{
				//所有的按钮都可以看到
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(127);
				Dlg.exec();
				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}//开发后门
			else if (keyboardinst.password == DEVPASS)
			{
				//根据权限表绘制按钮，并将按钮和处理代码链接
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(255);
				Dlg.exec();
				//返回主界面了
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}
			//密码不正确，普通用户无法设置
			else
			{
				return;
			}
		}

	}
	else
	{
		//所有的按钮都可以看到
		caution Dlg;
		Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
		Dlg.setWindowState(Qt::WindowFullScreen);
#endif
		Dlg.setWindowFlags(Qt::FramelessWindowHint);
		Dlg.init(127);
		Dlg.exec();

		//返回主界面了
		pubcom.CurrentWndMutex.lock();
		pubcom.CurrentWnd = 0;
		pubcom.CurrentWndMutex.unlock();
	}
}




//生产管理
void tech::on_pushButton_12_clicked()
{
#ifdef TEST
	//所有的按钮都可以看到
	motortest Dlg;
	Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
	Dlg.setWindowState(Qt::WindowFullScreen);
#endif
	Dlg.setWindowFlags(Qt::FramelessWindowHint);
	//Dlg.init(127);
	Dlg.exec();

	//返回主界面了
	pubcom.CurrentWndMutex.lock();
	pubcom.CurrentWnd = 0;
	pubcom.CurrentWndMutex.unlock();
#endif

}


//加速按
void tech::on_pushButton_14_pressed()
{
	publiccaution.addevent("主界面","加速","用户按加速按钮",1);

	WritePcMotorAccBit(&regs,true);

}

//加速释放
void tech::on_pushButton_14_released()
{
	publiccaution.addevent("主界面","加速","用户离开加速按钮",1);

	WritePcMotorAccBit(&regs,false);

}


//减速按
void tech::on_pushButton_15_pressed()
{
	publiccaution.addevent("主界面","减速","用户按减速按钮",1);

	WritePcMotorDecBit(&regs,true);


}

//减速释放
void tech::on_pushButton_15_released()
{
	publiccaution.addevent("主界面","减速","用户离开减速按钮",1);

	WritePcMotorDecBit(&regs,false);

}

//产生信号的按钮
pb::pb(QWidget *parent) :
QPushButton(parent)
{
	connect(this, SIGNAL(clicked()), this, SLOT(sendsignal()));
}

//发送信号
void pb::sendsignal()
{
	emit buttonClicked(num);
}

//设置编号
void pb::setnum(int n)
{
	num = n;
}

pb::~pb()
{

}

//产生信号的按钮
pbex::pbex(QWidget *parent) :
QPushButton(parent)
{
	connect(this, SIGNAL(clicked()), this, SLOT(sendsignal()));
}

//发送信号
void pbex::sendsignal()
{
	emit buttonClicked(x, y);
}

//设置编号
void pbex::setnum(int n, int m)
{
	x = n;
	y = m;
}

pbex::~pbex()
{

}

//产生信号的按钮
le::le(QWidget *parent) :
QLineEdit(parent)
{

}

le::~le()
{

}

//设置编号
void le::setnum(int n)
{
	num = n;
}

void le::mousePressEvent(QMouseEvent * event)
{
	QLineEdit::mousePressEvent(event);

	emit leClicked(num);
}

QColor bgColorForName(const QString &name)
{
	if (name == "Black")
		return QColor("#D8D8D8");
	else if (name == "White")
		return QColor("#F1F1F1");
	else if (name == "Red")
		return QColor("#F1D8D8");
	else if (name == "Green")
		return QColor("#D8E4D8");
	else if (name == "Blue")
		return QColor("#D8D8F1");
	else if (name == "Yellow")
		return QColor("#F1F0D8");
	return QColor(name).light(110);
}

QColor fgColorForName(const QString &name)
{
	if (name == "Black")
		return QColor("#6C6C6C");
	else if (name == "White")
		return QColor("#F8F8F8");
	else if (name == "Red")
		return QColor("#F86C6C");
	else if (name == "Green")
		return QColor("#6CB26C");
	else if (name == "Blue")
		return QColor("#6C6CF8");
	else if (name == "Yellow")
		return QColor("#F8F76C");
	return QColor(name);
}


//遍历XML树，取得某类参数的个数
int GetParaRow(const QString& groupname)
{
	QDomNode n;

	int tempj = 0;

	tempj = 0;

	QDomNodeList templist;
	templist = doc->elementsByTagName(groupname);
	if (templist.count() >= 1)
	{
		n = templist.item(0);
		//找到第一个param节点
		n = n.firstChild();
		//遍历所有的param节点
		while (!n.isNull())
		{
			n = n.nextSibling();
			tempj++;
		}
	}
	return tempj;
}

//遍历XML树，取得某类中的一个参数
const QDomNode GetPara(const QString& groupname, const QString tag,
					   const QString& paraname)
{
	QDomNode n;
	QDomNode p;

	QDomNodeList templist;
	templist = doc->elementsByTagName(groupname);
	if (templist.count() >= 1)
	{

		n = templist.item(0);
		//找到第一个param节点
		n = n.firstChild();

		//遍历所有的param节点,找到指定名字的参数的父节点
		while (!n.isNull())
		{
			p = n.firstChild();

			//遍历内部属性
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName() == tag)
					&& (p.toElement().text() == paraname))
				{
					return n;
				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
		}
		publiccaution.addevent("配置文件","未找到参数","在配置文件中没有找到参数",1);
		return n;
	}
	return n;
}

//遍历XML树，取得某类中的一个参数
const QDomNode GetParaByID(const QString& groupname, const QString& paraname)
{
	return GetPara(groupname, "id", paraname);
}

//遍历XML树，取得某类中的一个参数
const QDomNode GetParaByName(const QString& groupname, const QString& paraname)
{
	return GetPara(groupname, "name", paraname);
}

//放大倍数
void bump::setamp(float b)
{
	amp = b;
}

//设置起始显示点
void bump::setdisplaylocation(float x, float y)
{
	locx = x;
	locy = y;
}

//设置浆泵编号
void bump::setbumpid(unsigned int id)
{
	bumpid = id;
}
//设置和圆网连接点坐标
void bump::setprintpoint(float x, float y)
{
	printpointx = x;
	printpointy = y;
}

//显示组件
void bump::display(QPainter* painter)
{
	painter->translate(locx, locy);
	painter->setBrush(Qt::NoBrush);
	painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap,
		Qt::MiterJoin));
	painter->drawPath(bumpbase);

	painter->setBrush(Qt::black);
	painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap,
		Qt::MiterJoin));
	painter->drawPath(bumptri);

	painter->setBrush(Qt::NoBrush);
	painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap,
		Qt::MiterJoin));
	painter->drawPath(bumpcube);

	//bumplinkcube.
	QPainterPath temppath;
	bumplinkcube = temppath;
	bumplinkcube.moveTo(7, 0);
	bumplinkcube.cubicTo(QPointF((7 + (printpointx - locx)) / 2, (printpointy
		- locy)), QPointF((7 + (printpointx - locx)) / 2, (0 + (printpointy
		- locy)) / 2), QPointF((printpointx - locx), (printpointy - locy)));

	painter->drawPath(bumplinkcube);

	//浆泵浆料
	liquid = temppath;

	//00: 浆泵停     01：浆泵正转  10浆泵反转
	if(pubcom.bumpstatus[bumpid] == 00)
	{
		liquidy = 55;
	}
	else if(pubcom.bumpstatus[bumpid] == 01)
	{
		if (liquidy > 0)
		{

			liquidy -= 2;
		}
		else
		{
			liquidy = 55;
		}

	}
	else if(pubcom.bumpstatus[bumpid] == 10)
	{
		if (liquidy < 55)
		{

			liquidy += 2;
		}
		else
		{
			liquidy = 0;
		}
	}

	if (liquidy > 5)
	{
		liquid.addRect(5, liquidy, 5, 5);
		painter->setBrush(Qt::yellow);
		painter->setPen(QPen(Qt::darkYellow, 1, Qt::SolidLine, Qt::FlatCap,
			Qt::MiterJoin));
		painter->drawPath(liquid);
	}

	painter->translate(-locx, -locy);
}

//初始化
void bump::init()
{
	bumpbase.moveTo(2, 80);
	bumpbase.lineTo(2, 85);
	bumpbase.lineTo(12, 85);
	bumpbase.lineTo(12, 80);
	bumpbase.moveTo(7, 85);
	bumpbase.lineTo(7, 76);

	bumpbase.addEllipse(0, 62, 14, 14);

	bumptri.moveTo(7, 62);
	bumptri.lineTo(5, 67);
	bumptri.lineTo(9, 67);
	bumptri.lineTo(7, 62);

	bumpcube.moveTo(7, 62);
	bumpcube.lineTo(7, 0);

	liquidy = 40;
	locx = 0;
	locy = 0;
	printpointx = 0;
	printpointy = 0;

}

//新增计时器
int timermng::addtimer()
{
	int randnum;
	randnum = qrand();

	mutex.lockForWrite();

	//防止有相同数字在里面
	while (timermap.contains(randnum) == true)
	{
		randnum = qrand();
	}

	//插入计时器，初始值为0
	timermap.insert(randnum, 0);
	mutex.unlock();

	return randnum;

}

//删除计时器
int timermng::deltimer(int timerid)
{

	bool num = false;

	mutex.lockForWrite();
	if (timermap.contains(timerid) == true)
	{
		num = timermap.remove(timerid);
	}
	else
	{
		publiccaution.addevent("定时器ID不存在","定时器ID不存在","定时器ID不存在",0);
	}
	mutex.unlock();

	return num;
}

//取计时器的值
unsigned long timermng::gettimerval(int timerid)
{
	unsigned long num;
	mutex.lockForRead();
	num = timermap.value(timerid);
	mutex.unlock();
	return num;
}

//刷新每个计时器
int timermng::flush()
{
	QMap<unsigned int, unsigned long>::iterator i;

	mutex.lockForWrite();
	for (i = timermap.begin(); i != timermap.end(); ++i)
	{
		timermap.insert(i.key(), i.value() + 1);
	}
	mutex.unlock();

	return true;
}

void tech::init()
{
	//当前窗口
	pubcom.CurrentWnd = 0;
}
//设置产量
void tech::on_pushButton_2_clicked()
{

	keyboard2 keyboardinst;
	int number;
	mydb.GetValue("setworknumber",number);
	keyboardinst.setText(QString::number(number));
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//取消canel
	if (keyboardinst.flag == 1)
	{
		;
	}//确认
	else if (keyboardinst.flag == 2)
	{
		setworknumber = keyboardinst.enter.toInt();
		ui.lcdNumber_3->display(setworknumber);
		mydb.SaveValue("setworknumber",setworknumber);
	}
}

//产量归零
void tech::on_pushButton_11_clicked()
{

	QMessageBox msgBox;
	msgBox.setText("<font size = 4>产量归零？</font>");
	msgBox.setWindowTitle("产量归零");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("是");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("否");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes)
	{
		curworknumber = 0;
		sumcount = 0;
		ui.lcdNumber_2->display(0);
		mydb.SaveValue("curworknumber",curworknumber);
	}
}

//选择网头
void tech::on_pushButton_9_clicked()
{
	//运行状态，不允许设置网头
	if (pubcom.machprintstatus == 2)
	{
		return;
	}

	if (pubcom.machprintstatus == 2)
	{
		return;
	}
	selectprint Dlg;
	selectprintobject = &Dlg;
	Dlg.setWindowFlags(Qt::FramelessWindowHint);
	Dlg.init();
	QRect t = geometry();
	Dlg.setGeometry(t.left(),t.top()+50,Dlg.width(),Dlg.height());
	//Dlg.init(127);
	Dlg.exec();

}


//网头调速开关
void tech::on_pushButton_13_clicked()
{
	//网头调速
	QDomNode para = GetParaByName("machsub1", "网头调速允许按钮是否工作");
	bool ok;
	bool btwork = para.firstChildElement("value").text().toInt(&ok);

	if (btwork == true)
	{
		//只有在网头调速按钮空闲的时候才处理按钮
		if (printchgspeedflag != true)
		{
			QDomNode paranode = GetParaByName("prod", QString("网头调速允许"));
			bool ok;

			if (paranode.firstChildElement("value").text().toInt(&ok)
				== 1)
			{
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(0));
			}
			else
			{
				paranode.firstChildElement("value").firstChild().setNodeValue(
					QString::number(1));
			}

			//保存XML文件
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("打开文件aaa.xml失败！");
				msgBox.setWindowTitle("错误");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();

				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);

			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x06;//网头操作命令
			ptempevent->status = 0x00;//命令状态
			ptempevent->data[0] = 0x00;

			printchgspeedflag = true;

			//发送一个事件给界面线程
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}
}


void tech::customEvent(QEvent *e)
{
	if (e->type() == BACKEND_EVENT) //得到是后台的事件
	{
		backendevent* event = (backendevent*) e;

		//速度改变
		if (event->cmd == 0x01 && event->status == 0x01)
		{
			if (event->data[0] == 0x01)
			{
				chgspeedbtflag = false;

			}
			else if (event->data[0] == 0x02)
			{
				chgspeedbtflag = false;
			}
		}
		else if (event->cmd == 0x01 && event->status == 0x02)
		{
			if (event->data[0] == 0x01)
			{
				chgspeedbtflag = false;

			}
			else if (event->data[0] == 0x02)
			{
				chgspeedbtflag = false;
			}
			QMessageBox msgBox;
			msgBox.setText("改变速度失败！");
			msgBox.setWindowTitle("操作失败");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();
		}
		else if (event->cmd == 0x02 && event->status == 0x01)
		{
			if (event->data[0] == 0x54)
			{
				if (circleflag == false)
				{
					circleflag = true;
					ui.pushButton_18->setIcon(QIcon(":/images/fanzhuan_ing.PNG"));
				}
				else
				{
					circleflag = false;
					ui.pushButton_18->setIcon(QIcon(":/images/fanzhuan.PNG"));
				}
			}

			printopflag = false;
		}
		else if (event->cmd == 0x02 && event->status == 0x02)
		{

			printopflag = false;
			QMessageBox msgBox;
			msgBox.setText("网头不能同时做两个操作！");
			msgBox.setWindowTitle("操作失败");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();

		}
		else if (event->cmd == 0x02 && event->status == 0x03)
		{
			printopflag = false;
			QMessageBox msgBox;
			msgBox.setText("动作失败！");
			msgBox.setWindowTitle("命令失败");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();

		}
		//参数设置
		else if (event->cmd == 0x05 && event->status == 0x01)
		{
			pDlgset->close();
			pubcom.applyflag = false;
			//保存XML文件
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("打开文件aaa.xml失败！");
				msgBox.setWindowTitle("错误");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();

				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);

		}
		//参数设置
		else if (event->cmd == 0x05 && event->status == 0x02)
		{
			pDlgset->close();
			pubcom.applyflag = false;
			//保存XML文件
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("打开文件aaa.xml失败！");
				msgBox.setWindowTitle("错误");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();
				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);
			QMessageBox msgBox;
			msgBox.setText("部分参数没有设置成功！");
			msgBox.setWindowTitle("部分成功");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();

		}
		//参数设置
		else if (event->cmd == 0x05 && event->status == 0x03)
		{
			pDlgset->close();
			pubcom.applyflag = false;
			//保存XML文件
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("打开文件aaa.xml失败！");
				msgBox.setWindowTitle("错误");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();
				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);
			QMessageBox msgBox;
			msgBox.setText("参数没有设置成功！");
			msgBox.setWindowTitle("失败");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();

		}
		//参数设置
		else if (event->cmd == 0x05 && event->status == 0x04)
		{
			pDlgset->close();
			pubcom.applyflag = false;
			//保存XML文件
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("打开文件aaa.xml失败！");
				msgBox.setWindowTitle("错误");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();
				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);
			QMessageBox msgBox;
			msgBox.setText("参数暂时不能设置！");
			msgBox.setWindowTitle("失败");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();

		}
		//网头调速
		else if (event->cmd == 0x06 && event->status == 0x01)
		{
			printchgspeedflag = false;
		}
		else if (event->cmd == 0x06 && event->status == 0x02)
		{
			printchgspeedflag = false;

			QMessageBox msgBox;
			msgBox.setText("部分参数没有设置成功！");
			msgBox.setWindowTitle("部分成功");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();
		}
		else if (event->cmd == 0x06 && event->status == 0x03)
		{

			printchgspeedflag = false;

			QMessageBox msgBox;
			msgBox.setText("参数没有设置成功！");
			msgBox.setWindowTitle("失败");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("确定");
			msgBox.exec();
		}
		else if(event->cmd == 0x07)
		{
			//磁台的磁力显示
			for(int i =0; i < 16; i++)
			{
				magicpercent[i]->setVisible(false);
			}

			for(int i =0; i < pubcom.PrintNum; i++)
			{
				magicpercent[i]->setVisible(true);
			}

			//对零显示
			for(int i =0; i < 16; i++)
			{
				zerolb[i]->setVisible(false);
			}

			for(int i =0; i < pubcom.PrintNum; i++)
			{
				zerolb[i]->setVisible(true);
			}

			//网头状态显示
			for(int i =0; i < 16; i++)
			{
				PrintStatusLb[i]->setVisible(false);
			}

			for(int i =0; i < pubcom.PrintNum; i++)
			{
				PrintStatusLb[i]->setVisible(true);
			}
		}
	}
}


//产生信号的按钮
mylabel::mylabel(QWidget *parent) :
QLabel(parent)
{
	;
}
mylabel::~mylabel()
{
}

//动画绘制
void mylabel::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	tech* parent = (tech*)this->parentWidget();

	//出布
	painter.translate(62, 131);
	painter.rotate(+ parent->jinbuangel);
	painter.drawPicture(0, 0, parent->luobupic);
	painter.rotate(- parent->jinbuangel);
	painter.translate(-62, -131);

	//橡毯1
	painter.translate(138, 135);
	painter.rotate(+ parent->angel);
	painter.drawPicture(0, 0, parent->xiangtanpic);
	painter.rotate(- parent->angel);
	painter.translate(-138, -135);

	//橡毯2
	painter.translate(679, 135);
	painter.rotate(+ parent->angel);
	painter.drawPicture(0, 0, parent->xiangtanpic);
	painter.rotate(- parent->angel);
	painter.translate(-679, -135);


	//圆网
	for (int i = 0; i < pubcom.PrintNum; i++)
	{
		painter.translate(175 + 31 * i, 103- parent->updistance[i]);

		//圆网编号
		painter.setFont(parent->f2);
		painter.drawText(0,-15,QString::number(i+1));
		painter.rotate(- parent->motorangel[i]);

		painter.drawPicture(0, 0, parent->yuanwangpic);

		painter.rotate(+ parent->motorangel[i]);
		painter.translate(-(175 + 31 * i), -103+ parent->updistance[i]);
	}

	//磁台
	for (int i = 0; i < pubcom.PrintNum; i++)
	{
		painter.translate(165 + 31 * i, 130);
		painter.setPen(QPen(Qt::blue, 3, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
		painter.setBrush(Qt::NoBrush);

		painter.drawPath(parent->path1);

		painter.setPen(
			QPen(Qt::darkRed, 3, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
		painter.drawPath(parent->path2);

		painter.setPen(
			QPen(Qt::darkRed, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
		painter.drawPath(parent->path3);

		painter.translate(20 * parent->ampify, -30 * parent->ampify);

		for (int j = 0; j < 10; j++)
		{
			char pencent = pubcom.magicpercent[j]/10;
			if (j < (10 - pencent))
			{
				painter.setBrush(Qt::white);
			}
			else
			{
				painter.setBrush(Qt::green);
			}

			painter.drawPath(parent->path4[j]);
		}
		painter.translate(-20 * parent->ampify, 30 * parent->ampify);
		painter.translate(-(165 + 31 * i), -130);

		//浆泵
		parent->mybump[i].setdisplaylocation(185 + 31 * i, 109);
		parent->mybump[i].setprintpoint(175 + 31 * i, 103- parent->updistance[i]);
		parent->mybump[i].display(&painter);
	}

	//烘房
	painter.translate(807, 100);
	painter.rotate(+ parent->hongfangangel1);
	painter.drawPicture(0, 0, parent->hongfangpic);
	painter.rotate(- parent->hongfangangel1);
	painter.translate(-807, -100);

	painter.translate(807, 150);
	painter.rotate(+ parent->hongfangangel2);
	painter.drawPicture(0, 0, parent->hongfangpic);
	painter.rotate(- parent->hongfangangel2);
	painter.translate(-807, -150);


	//落布摆臂
	painter.translate(969, 37);
	painter.rotate(parent->baibiangel);
	painter.drawPicture(0, 0, parent->baibipic);
	painter.setBrush(Qt::NoBrush);
	painter.setPen(QPen(QColor(252, 109, 13, 255), 1.5));
	painter.drawLine(2, -16, 8, 61);
	painter.rotate(- parent->baibiangel);

	painter.translate(-969, -37);

	QMatrix matrix;
	matrix.translate(969, 37);
	matrix.rotate(parent->baibiangel);
	QPoint hem = matrix.map(QPoint(-3, -20));
	QPoint hemb = QPoint(951, 38);
	QPoint hemba = matrix.map(QPoint(8, 61));
	QPoint hembab = QPoint(947, 158);
	painter.setBrush(Qt::NoBrush);
	painter.setPen(QPen(QColor(252, 109, 13, 255), 1.5));
	painter.drawLine(hemb, hem);
	painter.drawLine(hemba, hembab);
	painter.end();

}


#ifdef TEST
//测试：转动
void tech::action_testcircle()
{
	if (InFollow == false)
	{
		if (InTestCircle == false)
		{
			InTestCircle = true;
			pb_testcircle->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(37,218,41) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(0,255,0);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(0,255,0);} ");
		} 
		else
		{
			InTestCircle = false;
			pb_testcircle->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
		}
	}
}


//测试：跟随
void tech::action_testfollow()
{
	if (InTestCircle == false 
		&& InOneCircleAction == false)
	{
		if (InFollow == false)
		{
			InFollow = true;
			pb_testfollow->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(37,218,41) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(0,255,0);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(0,255,0);} ");

			QList<int>::iterator it;
			pubcom.activeprintlistmutex.lockForRead();

			//激活的网头
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				//不是在准备状态才跟随
				if (pubcom.presetprintstatus[*it] != 4)
				{
					ksmc->EnableMasterFollowing((*it) + 1);
				}
			}
			pubcom.activeprintlistmutex.unlock();

		} 
		else
		{
			InFollow = false;
			pb_testfollow->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
			QList<int>::iterator it;
			pubcom.activeprintlistmutex.lockForRead();

			//激活的网头
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				//不是在准备状态才跟随
				if (pubcom.presetprintstatus[*it] != 4)
				{
					ksmc->DisableMasterFollowing((*it) + 1);
				}
			}
			pubcom.activeprintlistmutex.unlock();

		}
	} 
}

#endif


//抬网
void tech::on_pushButton_16_clicked()
{
	interfaceevent* ptempevent = new interfaceevent();

	//只有在上下标识空闲的时候才处理按钮
	if (printopflag != true)
	{
		publiccaution.addevent("主界面","抬网","用户按抬网按钮",1);

		ptempevent->cmd = 0x02;//网头操作命令
		ptempevent->status = 0x00;//命令状态
		ptempevent->data[0] = 0x03;//小命令字，下放还是上抬

		printopflag = true;
		//发送一个事件给后台线程
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}


//磁台关闭
void tech::on_pushButton_17_clicked()
{
	interfaceevent* ptempevent = new interfaceevent();

	//只有在反转标识空闲的时候才处理按钮
	if (printopflag != true)
	{
		publiccaution.addevent("主界面","磁台开启","用户按磁台开启按钮",1);

		ptempevent->cmd = 0x02;//网头操作命令
		ptempevent->status = 0x00;//命令状态
		ptempevent->data[0] = 0x08;//磁台关闭
		printopflag = true;

		//发送一个事件给界面线程
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}


//反转
void tech::on_pushButton_18_clicked()
{
	//运行状态，不允许反转
	if (pubcom.machprintstatus == 2)
	{
		return;
	}

	//急停或者欠压不允许启动反转，但是可以停止反转
	if((circleflag == false) && publiccaution.hascaution(PLCLOCATION, PLCHALTCATION))
	{
		return;
	}
	

	QList<int>::iterator it;

	//多线程保护
	pubcom.activeprintlistmutex.lockForRead();
	for (it = pubcom.activeprintlist.begin(); it
		!= pubcom.activeprintlist.end(); ++it)
	{

		//对零中不可以反转
		if (pubcom.actionzerostep.value(*it) != 0)
		{
			pubcom.activeprintlistmutex.unlock();
			return;
		}
	}
	pubcom.activeprintlistmutex.unlock();


	interfaceevent* ptempevent = new interfaceevent();

	//只有在反转标识空闲的时候才处理按钮
	if (printopflag != true)
	{
		publiccaution.addevent("主界面","反转","用户按反转按钮",1);

		ptempevent->cmd = 0x02;//网头操作命令
		ptempevent->status = 0x00;//命令状态
		ptempevent->data[0] = 0x54;//反转
		printopflag = true;

		//发送一个事件给界面线程
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}

