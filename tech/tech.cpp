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

//ȫ�ֵ�can�豸
extern canif candevice;

//���ܹ����
extern safenet safenetinst;

static const QSize resultSize(200, 200);
QColor bgColorForName(const QString &name);

//����setting���沿������
extern QSettings settings;

extern struct _modbusRTU modbusRTU;
extern struct _regs regs;


//����ַ���
QString choosestyle = QString("background-color: #00AA00; font: bold 14px;");//ѡ��
QString unchoosestyle = QString("font: bold 14px;");//û��ѡ��
QString preparestyle = QString("background-color: blue;font: bold 14px;");//׼��
QString datetimestyle = QString("color: #0000FF; font-size: 24px");//����
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

//ָ��set�ĶԻ���
set* pDlgset;

//ָ����ͷ�����ĶԻ���
opprint* pDlgop;

//�����Խ���
motortest* pDlgmotortest;

//A4N���
extern CKSMCA4 *ksmc;


//���ݿ�
extern DB mydb;

extern QObject* backendobject;
extern QObject* interfaceobject;
extern QObject* a4nobject;
extern QObject* selectprintobject;

char cautionbkflag = 0;
char cautionbkchgflag = 0;
char datetimechgflag = 0;
char cautionstrchgflag = 0;
//��ǰ��ʾ�Ĺ���id
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

	//������ʱ��
	timer = &mytimer;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));
	timer->start(100); //100ms��ʱ

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

	chgspeedbtflag = false; //�ı��ٶȰ�ť�Ƿ��ˣ�

	printopflag = false;

	oldmotorspeed = 0;

	f.setPointSize(30);
	f2.setPointSize(15);
	greenpen = new QPen(Qt::green, 4);
	graypen = new QPen(Qt::gray, 4);
	blackpen = new QPen(Qt::black, 4);

	//��ǰ����ָʾ
	curcautionindex = 0;

	//��ǰ���ϼ�ʱ��
	cautiondispcount = 0;
	//ui.lcdNumber->setStyleSheet(sudustyle);
	//ui.lcdNumber_2->setStyleSheet(chanliangstyle);

	//����
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
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_8->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_5->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_10->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_12->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_13->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_2->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:24px ����_GB2312; color:rgb(0,0,127)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_9->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:24px ����_GB2312; color:rgb(0,0,127)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	ui.pushButton_11->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:24px ����_GB2312; color:rgb(0,0,127)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_14->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:60px ����_GB2312; color:rgb(0,0,128)} QPushButton:hover {background-color: rgb(208,208,208);  color:rgb(0,0,128); border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_15->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 3px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:60px ����_GB2312; color:rgb(0,0,128)} QPushButton:hover {background-color: rgb(208,208,208);  color:rgb(0,0,128); border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.label_7->setStyleSheet("font:28px bold ����_GB2312");
	ui.label_9->setStyleSheet("font:28px bold ����_GB2312");
	ui.label_10->setStyleSheet("font:28px bold ����_GB2312");
	ui.label_8->setStyleSheet("font:28px bold ����_GB2312 ");
	ui.label_11->setStyleSheet("font:28px bold ����_GB2312 ");

	ui.pushButton_3->setStyleSheet(
		" QPushButton:hover {background-image: url(:/images/up_normal.png);background-color: rgb(208,208,208);  color:rgb(0,255,0); border-width: 4px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	printchgspeedflag = false;

	//��ͷ��̧���·�״̬ 0���·�״̬      1����̧״̬
	updownstatus = 0;

	//��̨�Ĵ�����ʾ
	for(int i =0; i < 16; i++)
	{
		magicpercent[i] = new le(this);
		magicpercent[i]->setText(QString::number(pubcom.magicpercent[i]) + "%");
		magicpercent[i]->setGeometry(175+i*31,600,30,20);
		magicpercent[i]->setVisible(false);
	}

	//��̨�Ĵ�����ʾ
	for(int i =0; i < pubcom.PrintNum; i++)
	{
		magicpercent[i]->setVisible(true);
	}

	//״̬ͼ��
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

	//״̬��ʾ
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

	//����ͼ��
	zeropix[0] = new QPixmap(":/images/zero0.PNG", "PNG");
	zeropix[1] = new QPixmap(":/images/zero1.PNG", "PNG");

	//������ʾ
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

	//����ʷ��¼�лָ�
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
	pb_testcircle->setText("��ת");
	pb_testcircle->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	connect(pb_testcircle,SIGNAL(clicked()),this,SLOT(action_testcircle()));

	pb_testfollow = new QPushButton(this);
	pb_testfollow->setGeometry(QRect(330, 300, 111, 51));
	pb_testfollow->setText("����");
	pb_testfollow->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	connect(pb_testfollow,SIGNAL(clicked()),this,SLOT(action_testfollow()));

	//�Ƿ������Է�ת��ʶ
	InTestCircle  = false;

	//�Ƿ�������
	InFollow = false;

	//�Ƿ���һ����ת������
	InOneCircleAction = false;

#endif
	licovertimestatus = 0;
	hislicovertimestatus = 0;
#ifdef HAVEDOG
	//���ܹ���ѯ�趨��ֵ
	int multi = 32767 / 600;
	dogquerytimerset = rand() / multi + 1; //+1�Ƿ�ֹ����ʱ��Ϊ0

	//���ܹ���ѯ��ʱ��
	dogquerytimer = 0;
	//���ܹ���ѯʧ�ܼ���
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

//��ʱ���������
void tech::timeupdate()
{
	//��ʱ��ˢ��
	pubtimermng.flush();

	//can����ʱ���ˢ��
	candevice.refreshtime();

	//ˢ�¼Ĵ�������������
	FlushRegsReadHeatbeat(&regs);

	if (pubcom.PowerOff == true)
	{
		this->accept();
	}

	//����������ʶ
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
	//���ܹ���ѯ��ʱ��
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
		else//ֻҪ�ɹ�һ�Σ�������Ϊ0
		{
			dogqueryfailcount = 0;
		}
		//�����µ������ѯʱ��
		int multi = 32767 / 600;
		dogquerytimerset = rand() / multi + 1; //+1�Ƿ�ֹ����ʱ��Ϊ0
	}


	if (dogqueryfailcount >= 3 )
	{
		timer->disconnect(this);

		//ȷ������ˢ�µ����ݿ��ļ���
		while (mydb.SQLQueuechg == true 
			|| mydb.SQLQueuechg2 == true)
		{
			;
		}

		pubcom.quitthread = true;

		QMessageBox msgBox;
		msgBox.setText("û�м�⵽���ܹ���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		this->accept();

	}



	//�鿴����
	if (dogdateflag == '1')
	{
		//�鿴�����Ƿ���
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



	//���ϱ�����˸������
	if (publiccaution.count() >= 1
		&& pubcom.delcauflag == false)
	{
		//������˸
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

	//�����ַ���ʾ��ʱ��
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

	//ˢ�����ں�ʱ��
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
		LicOverDisplay->setText("��Ʒ���֤����,�벦���ҵ绰021-56035777��");
	}
	//���֤�����˳�
	else if(hislicovertimestatus != 2 && licovertimestatus == 2)
	{
		timer->disconnect(this);

		//ȷ������ˢ�µ����ݿ��ļ���
		while (mydb.SQLQueuechg == true 
			|| mydb.SQLQueuechg2 == true)
		{
			;
		}

		pubcom.quitthread = true;

		QMessageBox msgBox;
		msgBox.setText("��Ʒʹ�����޵�������ϵ���ң�");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		this->accept();
	}

	//Բ��ÿ���׵�������
	double pulseperunit = 0.0;
	ksmc->GetPulsePerUnit(pulseperunit);

		//��ʼ������ܿ�ʼ���˶���
	if (pubcom.initflag == 1)
	{

#ifdef TEST
		//����ת��ʱ�򣬲���ȡ�������ٶ�
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
		//������ٶȸı䣬�����ٶȸ���ͷ
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
			&& pubcom.motorspeed > 1) //��̺�ٶ���С�ٶȱ���Ϊ1���ϲſ��Բɼ�
#endif
		{
			//�ϼ���
			ksmc->GetEncoderCount(newcount);
		}
#ifdef TEST
		else
#else
		else if (pubcom.machprintstatus == 2
			&& pubcom.motorspeed > 1)//��̺�ٶ���С�ٶȱ���Ϊ1���ϲſ��Բɼ�
#endif
		{
			oldcount = newcount;
			ksmc->GetEncoderCount(newcount);


			//��ֵС��С�ھ�ֵ��˵�����ܷ�ת��
			if (newcount < oldcount)
			{
				//������ת�������Ǳ���������,�������岻���ܳ���3000000000
				if (newcount < 1000000000 && oldcount > 4000000000)
				{
					//λ�ƾ���У�飬��100ms������棬�����200m/���ӣ����λ��Ϊ200*1000/60 *0.1 = 333����
					//if(abs((4294967295 - oldcount + newcount)/pulseperunit) < 2000)
					//{
						//4294967295��32λ���ε����ֵ
						sumcount += 4294967295 - oldcount + newcount;
					//}
				}
				//���������˲��������
				/*
				else
				{
				sumcount += newcount - oldcount;
				}
				*/
			}
			else
			{
				//λ�ƾ���У�飬��100ms������棬�����200m/���ӣ����λ��Ϊ200*1000/60 *0.1 = 333����
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

				//�Ƿ�Ҫ�������
				savecurworknumber = true;
			}
		}

		ui.lcdNumber_2->display( (curworknumber) ); 

		//�����Ƿ����
		if (curworknumber >= setworknumber)
		{
			WritePcWorkFinBit(&regs,true);
		}
		else
		{
			WritePcWorkFinBit(&regs,false);
		}
	}

	//������д
	WritePcWorkRecDWord(&regs,curworknumber);

	//10�뱣��һ�ε�ǰ�������������´μ���
	if (savecurworknumbertimer < 100)
	{
		savecurworknumbertimer++;
	}
	else
	{
		savecurworknumbertimer = 0;

		//�Ƿ��иı䣬�иı��򱣴�
		if (savecurworknumber == true)
		{
			mydb.SaveValue("curworknumber",curworknumber);
			savecurworknumber = false;
		}
	}


	//������ٶ�
	if (pubcom.CurrentWnd == 0)
	{

#ifndef TEST
		//������ٶ�Ϊ ��/���� �������ٶ�120��/���ӣ���Ӧ30��
		//���ó����ٶ�Ϊ ������/����
		speed = ((float)pubcom.motorspeed/120)*30;

		//������ٲ�ת������
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
			//������ٶ�Ϊ ��/���� �������ٶ�120��/���ӣ���Ӧ30��
			//���ó����ٶ�Ϊ ������/����
			speed = ((float)pubcom.motorspeed/120)*30;

			//������ٲ�ת������
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

		//��ת�Ƕ�
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


		//Բ��
		for (int i = 0; i < pubcom.PrintNum; i++)
		{
			double tempspeed = 0.0;


			if (ksmc->GetMotorSpeed(i+1,tempspeed) == true)
			{
				//����ٶ�
				//ÿ���������� / ÿ�������� �ó� ÿ���ӵ����� ���ٶ�
				//�����ٶ�Ϊ120��/���� �������ٶȵ�ʱ�򣬶�Ӧ�Ľ��ٶ�Ϊ 30��
				//���յ��ٶ�Ϊ ������/ÿ���� 
				motorspeed[i] = ((float)tempspeed * 1000 * 60 / (pulseperunit * 1000)) /120 * 30;

#ifdef TEST
				//������ת��ʱ�򣬻����ڶ����е�ʱ�򣬽���ͷ1���ٶ���ʾ�ڽ�����
				if (i == 0 
					&& ( InTestCircle == true || InOneCircleAction == true)
					)
				{
					pubcom.motorspeed = -(tempspeed * 1000 * 60 / (pulseperunit * 1000));
				}
#endif
			}
			//������ٲ�ת������
			if(motorspeed[i] > 0.2 && motorspeed[i] < 1.2)
			{
				motorspeed[i] = 1;
			}
			else if (motorspeed[i] < -0.2 && motorspeed[i] > -1.2)
			{
				motorspeed[i] = -1;
			}

			//��ת�Ƕ�
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

		//������ͷ��������
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
			//��̧��
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

			//�½���
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


		//���ϱ����仯��
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

		//�����ַ��仯��
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


		//���ڱ仯��
		if (datetimechgflag == 1)
		{
			datetimechgflag = 0;
			datetime->setText(QDateTime::currentDateTime().toString(
				"yyyy-MM-dd hh:mm:ss"));
		}

		//ֻ����������ʱ��Ŵ���
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

		//��ʾ����
		ui.lcdNumber_3->display(setworknumber);

		ui.lcdNumber->display(pubcom.motorspeed);

		//��̨�ٷֱ�
		for(int i =0; i < pubcom.PrintNum; i++)
		{
			magicpercent[i]->setText(QString::number(pubcom.magicpercent[i]) + "%");
			//magicpercent[i]->setText(QString::number(100) + "%");
		}

		//��ͷ״̬
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

		//����״̬
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

		QDomNode para = GetParaByName("machsub1", "��ͷ��������ť�Ƿ���");
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

		QDomNode paranode = GetParaByName("prod", QString("��ͷ��������"));

		if (paranode.firstChildElement("value").text().toInt(&ok)
			== 1)
		{
			ui.pushButton_13->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(37,218,41) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(0,255,0);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(0,255,0);} ");
			ui.pushButton_13->setText(QString("���ٿ�"));

		}
		else
		{
			ui.pushButton_13->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
			ui.pushButton_13->setText(QString("���ٹ�"));
		}


		QPoint leftup;
		QPoint rightdown;

		//��ǰ���λ��
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
			//û�������������붯��
			if (InOneCircleAction == false)
			{
				QList<int>::iterator it;

				pubcom.activeprintlistmutex.lockForRead();

				//�������ͷ
				for (it = pubcom.activeprintlist.begin(); it
					!= pubcom.activeprintlist.end(); ++it)
				{
					//������׼��״̬����ת
					if (pubcom.presetprintstatus[*it] != 4)
					{
						ksmc->test((*it) + 1,3,50,200);
					}
				}
				pubcom.activeprintlistmutex.unlock();
				InOneCircleAction = true;

				//�ӳ٣������˶���ˢ������̫��
				Sleep(200);
			} 
		}

		//�������꣬���ñ�ʶλ
		if (InOneCircleAction == true)
		{
			bool notfinished = false;
			int status[16];
			memset(status,0,16);

			QList<int>::iterator it;

			pubcom.activeprintlistmutex.lockForRead();

			//�������ͷ
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				//������׼��״̬�����Ƿ����
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
				//��ʶ
				InOneCircleAction = false;
			}
		}

#endif

	}
}


//����
void tech::on_pushButton_3_clicked()
{
	interfaceevent* ptempevent = new interfaceevent();

	//ֻ�������±�ʶ���е�ʱ��Ŵ���ť
	if (printopflag != true)
	{
		publiccaution.addevent("������","����","�û���������ť",1);

		ptempevent->cmd = 0x02;//��ͷ��������
		ptempevent->status = 0x00;//����״̬
		ptempevent->data[0] = 0x04;//С�����֣��·Ż�����̧

		printopflag = true;
		//����һ���¼�����̨�߳�
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}

//��̨����
void tech::on_pushButton_4_clicked()
{
	interfaceevent* ptempevent = new interfaceevent();

	//ֻ���ڷ�ת��ʶ���е�ʱ��Ŵ���ť
	if (printopflag != true)
	{
		publiccaution.addevent("������","��̨����","�û�����̨������ť",1);

		ptempevent->cmd = 0x02;//��ͷ��������
		ptempevent->status = 0x00;//����״̬
		ptempevent->data[0] = 0x07;//��̨����
		printopflag = true;

		//����һ���¼��������߳�
		QCoreApplication::postEvent(backendobject, ptempevent);
	}

}

//����
void tech::on_pushButton_7_clicked()
{
	//����״̬�����������
	if (pubcom.machprintstatus == 2)
	{
		return;
	}

	//��ͣ����Ƿѹ���������
	if(publiccaution.hascaution(PLCLOCATION, PLCHALTCATION))
	{
		return;
	}

	//if(circleflag == true)
	//	return;

	QMessageBox msgBox;
	msgBox.setText("<font size = 4>�����ڼ䲻�ɲ��������Ҫ����ô��</font>");
	msgBox.setWindowTitle("����");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("��");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes)
	{
		publiccaution.addevent("������","����","�û������㰴ť",1);

		interfaceevent* ptempevent = new interfaceevent();

		//ֻ���ڶ����ʶ���е�ʱ��Ŵ���ť
		if (printopflag != true)
		{
			ptempevent->cmd = 0x02;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x53;//����
			printopflag = true;

			//����һ���¼�����̨�߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}
}

//����
void tech::on_pushButton_5_clicked()
{
	//����״̬������������
	if (pubcom.machprintstatus == 2)
	{
		return;
	}

	//�Ƿ���Ҫ�����������
	QDomNode para = GetParaByName("needpass", "����");
	if (para.isNull())
	{
		QMessageBox msgBox;
		msgBox.setText("���ò���Ϊ�գ�");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		return;
	}

	if (para.firstChildElement("value").text().toInt() == 1)
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
				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("prod/modright").toInt();

				//�������set����
				if ((modright & 0x7C) == 0x00 )
				{
					return;
				}

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
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

				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//ˢ�¹���
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}//����
			else if (settings.value("mach/pass").toString()
				== keyboardinst.password
				&& !settings.value("mach/pass").toString().isNull())
			{

				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("mach/modright").toInt();

				//�������set����
				if ((modright & 0x7C) == 0x00 )
				{
					return;
				}

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
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

				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//ˢ�¹���
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}//����
			else if (settings.value("tune/pass").toString()
				== keyboardinst.password
				&& !settings.value("tune/pass").toString().isNull())
			{
				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("tune/modright").toInt();

				//�������set����
				if ((modright & 0x7C) == 0x00 )
				{
					return;
				}

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
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

				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//ˢ�¹���
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}//һ�����
			else if (keyboardinst.password == BACKDOORPASS )
			{
				//��������������صİ�ť��ʾ
				ui.pushButton_3->show();
				ui.pushButton_4->show();
				ui.pushButton_7->show();
				ui.pushButton_16->show();
				ui.pushButton_17->show();
				ui.pushButton_18->show();

				//���еİ�ť�����Կ���
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

				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//ˢ�¹���
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}//��������
			else if (keyboardinst.password == DEVPASS)
			{
				//��������������صİ�ť��ʾ
				ui.pushButton_3->show();
				ui.pushButton_4->show();
				ui.pushButton_7->show();
				ui.pushButton_16->show();
				ui.pushButton_17->show();
				ui.pushButton_18->show();

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
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

				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				//ˢ�¹���
				cautionbkchgflag = 1;
				cautionstrchgflag = 1;

				setobject = NULL;

			}
			//���벻��ȷ����ͨ�û��޷�����
			else
			{
				return;
			}
		}

	}
	else
	{
		//���еİ�ť�����Կ���
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

		//������������
		pubcom.CurrentWndMutex.lock();
		pubcom.CurrentWnd = 0;
		pubcom.CurrentWndMutex.unlock();
		cautionbkchgflag = 1;
		cautionstrchgflag = 1;

		setobject = NULL;
	}
}

//����
void tech::on_pushButton_6_clicked()
{
	//���еİ�ť�����Կ���
	help Dlg;
	Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
	Dlg.setWindowState(Qt::WindowFullScreen);
#endif
	Dlg.setWindowFlags(Qt::FramelessWindowHint);
	//Dlg.setModal(false);
	Dlg.init(127);
	Dlg.exec();

	//������������
	pubcom.CurrentWndMutex.lock();
	pubcom.CurrentWnd = 0;
	pubcom.CurrentWndMutex.unlock();
}

//��ͷ����
void tech::on_pushButton_8_clicked()
{
	//�Ƿ���Ҫ�����������
	QDomNode para = GetParaByName("needpass", "��ͷ����");
	if (para.isNull())
	{
		QMessageBox msgBox;
		msgBox.setText("���ò���Ϊ�գ�");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();
		return;
	}

	if (para.firstChildElement("value").text().toInt() == 1)
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
				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("prod/modright").toInt();

				//��������������
				if ((modright & 0x01) == 0x00 )
				{
					return;
				}

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
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
				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

				opobject = NULL;

			}//����
			else if (settings.value("mach/pass").toString()
				== keyboardinst.password
				&& !settings.value("mach/pass").toString().isNull())
			{
				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("mach/modright").toInt();

				//��������������
				if ((modright & 0x01) == 0x00 )
				{
					return;
				}

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
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
				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();
				opobject = NULL;

			}//����
			else if (settings.value("tune/pass").toString()
				== keyboardinst.password
				&& !settings.value("tune/pass").toString().isNull())
			{
				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("tune/modright").toInt();

				//��������������
				if ((modright & 0x01) == 0x00 )
				{
					return;
				}
				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
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

				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

				opobject = NULL;

			}//һ�����
			else if (keyboardinst.password == BACKDOORPASS )
			{
				//��������������صİ�ť��ʾ
				ui.pushButton_3->show();
				ui.pushButton_4->show();
				ui.pushButton_7->show();
				ui.pushButton_16->show();
				ui.pushButton_17->show();
				ui.pushButton_18->show();

				//���еİ�ť�����Կ���
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

				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

				opobject = NULL;

			}//��������
			else if (keyboardinst.password == DEVPASS)
			{

				//��������������صİ�ť��ʾ
				ui.pushButton_3->show();
				ui.pushButton_4->show();
				ui.pushButton_7->show();
				ui.pushButton_16->show();
				ui.pushButton_17->show();
				ui.pushButton_18->show();

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
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

				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

				opobject = NULL;

			}
			//���벻��ȷ����ͨ�û��޷�����
			else
			{
				return;
			}
		}

	}
	else
	{
		//���еİ�ť�����Կ���
		opprint Dlg;
		opobject = &Dlg;
		pDlgop = &Dlg;
#ifdef ONIPC
		Dlg.setWindowState(Qt::WindowFullScreen);
#endif
		Dlg.setWindowFlags(Qt::FramelessWindowHint);
		Dlg.init(127);
		Dlg.exec();

		//������������
		pubcom.CurrentWndMutex.lock();
		pubcom.CurrentWnd = 0;
		pubcom.CurrentWndMutex.unlock();

		opobject = NULL;
	}

}

//���Ͻ���
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

		//������������
		pubcom.CurrentWndMutex.lock();
		pubcom.CurrentWnd = 0;
		pubcom.CurrentWndMutex.unlock();

	}
	QDialog::mousePressEvent(event);
}

//����
void tech::on_pushButton_10_clicked()
{
	//�Ƿ���Ҫ�����������
	QDomNode para = GetParaByName("needpass", "����");
	if (para.isNull())
	{
		QMessageBox msgBox;
		msgBox.setText("���ò���Ϊ�գ�");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();
		return;
	}

	if (para.firstChildElement("value").text().toInt() == 1)
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
				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("prod/modright").toInt();

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();
				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}//����
			else if (settings.value("mach/pass").toString()
				== keyboardinst.password
				&& !settings.value("mach/pass").toString().isNull())
			{
				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("mach/modright").toInt();

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();
				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}//����
			else if (settings.value("tune/pass").toString()
				== keyboardinst.password
				&& !settings.value("tune/pass").toString().isNull())
			{
				//ȡ��ģ��Ȩ�ޱ�
				int modright = settings.value("tune/modright").toInt();

				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(modright);
				Dlg.exec();
				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}//һ�����
			else if (keyboardinst.password == BACKDOORPASS )
			{
				//���еİ�ť�����Կ���
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(127);
				Dlg.exec();
				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}//��������
			else if (keyboardinst.password == DEVPASS)
			{
				//����Ȩ�ޱ���ư�ť��������ť�ʹ����������
				caution Dlg;
				Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
				Dlg.setWindowState(Qt::WindowFullScreen);
#endif
				Dlg.setWindowFlags(Qt::FramelessWindowHint);
				Dlg.init(255);
				Dlg.exec();
				//������������
				pubcom.CurrentWndMutex.lock();
				pubcom.CurrentWnd = 0;
				pubcom.CurrentWndMutex.unlock();

			}
			//���벻��ȷ����ͨ�û��޷�����
			else
			{
				return;
			}
		}

	}
	else
	{
		//���еİ�ť�����Կ���
		caution Dlg;
		Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
		Dlg.setWindowState(Qt::WindowFullScreen);
#endif
		Dlg.setWindowFlags(Qt::FramelessWindowHint);
		Dlg.init(127);
		Dlg.exec();

		//������������
		pubcom.CurrentWndMutex.lock();
		pubcom.CurrentWnd = 0;
		pubcom.CurrentWndMutex.unlock();
	}
}




//��������
void tech::on_pushButton_12_clicked()
{
#ifdef TEST
	//���еİ�ť�����Կ���
	motortest Dlg;
	Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
	Dlg.setWindowState(Qt::WindowFullScreen);
#endif
	Dlg.setWindowFlags(Qt::FramelessWindowHint);
	//Dlg.init(127);
	Dlg.exec();

	//������������
	pubcom.CurrentWndMutex.lock();
	pubcom.CurrentWnd = 0;
	pubcom.CurrentWndMutex.unlock();
#endif

}


//���ٰ�
void tech::on_pushButton_14_pressed()
{
	publiccaution.addevent("������","����","�û������ٰ�ť",1);

	WritePcMotorAccBit(&regs,true);

}

//�����ͷ�
void tech::on_pushButton_14_released()
{
	publiccaution.addevent("������","����","�û��뿪���ٰ�ť",1);

	WritePcMotorAccBit(&regs,false);

}


//���ٰ�
void tech::on_pushButton_15_pressed()
{
	publiccaution.addevent("������","����","�û������ٰ�ť",1);

	WritePcMotorDecBit(&regs,true);


}

//�����ͷ�
void tech::on_pushButton_15_released()
{
	publiccaution.addevent("������","����","�û��뿪���ٰ�ť",1);

	WritePcMotorDecBit(&regs,false);

}

//�����źŵİ�ť
pb::pb(QWidget *parent) :
QPushButton(parent)
{
	connect(this, SIGNAL(clicked()), this, SLOT(sendsignal()));
}

//�����ź�
void pb::sendsignal()
{
	emit buttonClicked(num);
}

//���ñ��
void pb::setnum(int n)
{
	num = n;
}

pb::~pb()
{

}

//�����źŵİ�ť
pbex::pbex(QWidget *parent) :
QPushButton(parent)
{
	connect(this, SIGNAL(clicked()), this, SLOT(sendsignal()));
}

//�����ź�
void pbex::sendsignal()
{
	emit buttonClicked(x, y);
}

//���ñ��
void pbex::setnum(int n, int m)
{
	x = n;
	y = m;
}

pbex::~pbex()
{

}

//�����źŵİ�ť
le::le(QWidget *parent) :
QLineEdit(parent)
{

}

le::~le()
{

}

//���ñ��
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


//����XML����ȡ��ĳ������ĸ���
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
		//�ҵ���һ��param�ڵ�
		n = n.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			n = n.nextSibling();
			tempj++;
		}
	}
	return tempj;
}

//����XML����ȡ��ĳ���е�һ������
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
		//�ҵ���һ��param�ڵ�
		n = n.firstChild();

		//�������е�param�ڵ�,�ҵ�ָ�����ֵĲ����ĸ��ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();

			//�����ڲ�����
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
		publiccaution.addevent("�����ļ�","δ�ҵ�����","�������ļ���û���ҵ�����",1);
		return n;
	}
	return n;
}

//����XML����ȡ��ĳ���е�һ������
const QDomNode GetParaByID(const QString& groupname, const QString& paraname)
{
	return GetPara(groupname, "id", paraname);
}

//����XML����ȡ��ĳ���е�һ������
const QDomNode GetParaByName(const QString& groupname, const QString& paraname)
{
	return GetPara(groupname, "name", paraname);
}

//�Ŵ���
void bump::setamp(float b)
{
	amp = b;
}

//������ʼ��ʾ��
void bump::setdisplaylocation(float x, float y)
{
	locx = x;
	locy = y;
}

//���ý��ñ��
void bump::setbumpid(unsigned int id)
{
	bumpid = id;
}
//���ú�Բ�����ӵ�����
void bump::setprintpoint(float x, float y)
{
	printpointx = x;
	printpointy = y;
}

//��ʾ���
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

	//���ý���
	liquid = temppath;

	//00: ����ͣ     01��������ת  10���÷�ת
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

//��ʼ��
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

//������ʱ��
int timermng::addtimer()
{
	int randnum;
	randnum = qrand();

	mutex.lockForWrite();

	//��ֹ����ͬ����������
	while (timermap.contains(randnum) == true)
	{
		randnum = qrand();
	}

	//�����ʱ������ʼֵΪ0
	timermap.insert(randnum, 0);
	mutex.unlock();

	return randnum;

}

//ɾ����ʱ��
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
		publiccaution.addevent("��ʱ��ID������","��ʱ��ID������","��ʱ��ID������",0);
	}
	mutex.unlock();

	return num;
}

//ȡ��ʱ����ֵ
unsigned long timermng::gettimerval(int timerid)
{
	unsigned long num;
	mutex.lockForRead();
	num = timermap.value(timerid);
	mutex.unlock();
	return num;
}

//ˢ��ÿ����ʱ��
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
	//��ǰ����
	pubcom.CurrentWnd = 0;
}
//���ò���
void tech::on_pushButton_2_clicked()
{

	keyboard2 keyboardinst;
	int number;
	mydb.GetValue("setworknumber",number);
	keyboardinst.setText(QString::number(number));
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//ȡ��canel
	if (keyboardinst.flag == 1)
	{
		;
	}//ȷ��
	else if (keyboardinst.flag == 2)
	{
		setworknumber = keyboardinst.enter.toInt();
		ui.lcdNumber_3->display(setworknumber);
		mydb.SaveValue("setworknumber",setworknumber);
	}
}

//��������
void tech::on_pushButton_11_clicked()
{

	QMessageBox msgBox;
	msgBox.setText("<font size = 4>�������㣿</font>");
	msgBox.setWindowTitle("��������");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("��");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes)
	{
		curworknumber = 0;
		sumcount = 0;
		ui.lcdNumber_2->display(0);
		mydb.SaveValue("curworknumber",curworknumber);
	}
}

//ѡ����ͷ
void tech::on_pushButton_9_clicked()
{
	//����״̬��������������ͷ
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


//��ͷ���ٿ���
void tech::on_pushButton_13_clicked()
{
	//��ͷ����
	QDomNode para = GetParaByName("machsub1", "��ͷ��������ť�Ƿ���");
	bool ok;
	bool btwork = para.firstChildElement("value").text().toInt(&ok);

	if (btwork == true)
	{
		//ֻ������ͷ���ٰ�ť���е�ʱ��Ŵ���ť
		if (printchgspeedflag != true)
		{
			QDomNode paranode = GetParaByName("prod", QString("��ͷ��������"));
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

			//����XML�ļ�
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("���ļ�aaa.xmlʧ�ܣ�");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);

			interfaceevent* ptempevent = new interfaceevent();

			ptempevent->cmd = 0x06;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x00;

			printchgspeedflag = true;

			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}
}


void tech::customEvent(QEvent *e)
{
	if (e->type() == BACKEND_EVENT) //�õ��Ǻ�̨���¼�
	{
		backendevent* event = (backendevent*) e;

		//�ٶȸı�
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
			msgBox.setText("�ı��ٶ�ʧ�ܣ�");
			msgBox.setWindowTitle("����ʧ��");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
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
			msgBox.setText("��ͷ����ͬʱ������������");
			msgBox.setWindowTitle("����ʧ��");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

		}
		else if (event->cmd == 0x02 && event->status == 0x03)
		{
			printopflag = false;
			QMessageBox msgBox;
			msgBox.setText("����ʧ�ܣ�");
			msgBox.setWindowTitle("����ʧ��");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

		}
		//��������
		else if (event->cmd == 0x05 && event->status == 0x01)
		{
			pDlgset->close();
			pubcom.applyflag = false;
			//����XML�ļ�
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("���ļ�aaa.xmlʧ�ܣ�");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);

		}
		//��������
		else if (event->cmd == 0x05 && event->status == 0x02)
		{
			pDlgset->close();
			pubcom.applyflag = false;
			//����XML�ļ�
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("���ļ�aaa.xmlʧ�ܣ�");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);
			QMessageBox msgBox;
			msgBox.setText("���ֲ���û�����óɹ���");
			msgBox.setWindowTitle("���ֳɹ�");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

		}
		//��������
		else if (event->cmd == 0x05 && event->status == 0x03)
		{
			pDlgset->close();
			pubcom.applyflag = false;
			//����XML�ļ�
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("���ļ�aaa.xmlʧ�ܣ�");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);
			QMessageBox msgBox;
			msgBox.setText("����û�����óɹ���");
			msgBox.setWindowTitle("ʧ��");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

		}
		//��������
		else if (event->cmd == 0x05 && event->status == 0x04)
		{
			pDlgset->close();
			pubcom.applyflag = false;
			//����XML�ļ�
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("���ļ�aaa.xmlʧ�ܣ�");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}

			QTextStream out(&file);
			doc->save(out, 4);
			QMessageBox msgBox;
			msgBox.setText("������ʱ�������ã�");
			msgBox.setWindowTitle("ʧ��");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

		}
		//��ͷ����
		else if (event->cmd == 0x06 && event->status == 0x01)
		{
			printchgspeedflag = false;
		}
		else if (event->cmd == 0x06 && event->status == 0x02)
		{
			printchgspeedflag = false;

			QMessageBox msgBox;
			msgBox.setText("���ֲ���û�����óɹ���");
			msgBox.setWindowTitle("���ֳɹ�");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
		}
		else if (event->cmd == 0x06 && event->status == 0x03)
		{

			printchgspeedflag = false;

			QMessageBox msgBox;
			msgBox.setText("����û�����óɹ���");
			msgBox.setWindowTitle("ʧ��");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
		}
		else if(event->cmd == 0x07)
		{
			//��̨�Ĵ�����ʾ
			for(int i =0; i < 16; i++)
			{
				magicpercent[i]->setVisible(false);
			}

			for(int i =0; i < pubcom.PrintNum; i++)
			{
				magicpercent[i]->setVisible(true);
			}

			//������ʾ
			for(int i =0; i < 16; i++)
			{
				zerolb[i]->setVisible(false);
			}

			for(int i =0; i < pubcom.PrintNum; i++)
			{
				zerolb[i]->setVisible(true);
			}

			//��ͷ״̬��ʾ
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


//�����źŵİ�ť
mylabel::mylabel(QWidget *parent) :
QLabel(parent)
{
	;
}
mylabel::~mylabel()
{
}

//��������
void mylabel::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	tech* parent = (tech*)this->parentWidget();

	//����
	painter.translate(62, 131);
	painter.rotate(+ parent->jinbuangel);
	painter.drawPicture(0, 0, parent->luobupic);
	painter.rotate(- parent->jinbuangel);
	painter.translate(-62, -131);

	//��̺1
	painter.translate(138, 135);
	painter.rotate(+ parent->angel);
	painter.drawPicture(0, 0, parent->xiangtanpic);
	painter.rotate(- parent->angel);
	painter.translate(-138, -135);

	//��̺2
	painter.translate(679, 135);
	painter.rotate(+ parent->angel);
	painter.drawPicture(0, 0, parent->xiangtanpic);
	painter.rotate(- parent->angel);
	painter.translate(-679, -135);


	//Բ��
	for (int i = 0; i < pubcom.PrintNum; i++)
	{
		painter.translate(175 + 31 * i, 103- parent->updistance[i]);

		//Բ�����
		painter.setFont(parent->f2);
		painter.drawText(0,-15,QString::number(i+1));
		painter.rotate(- parent->motorangel[i]);

		painter.drawPicture(0, 0, parent->yuanwangpic);

		painter.rotate(+ parent->motorangel[i]);
		painter.translate(-(175 + 31 * i), -103+ parent->updistance[i]);
	}

	//��̨
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

		//����
		parent->mybump[i].setdisplaylocation(185 + 31 * i, 109);
		parent->mybump[i].setprintpoint(175 + 31 * i, 103- parent->updistance[i]);
		parent->mybump[i].display(&painter);
	}

	//�淿
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


	//�䲼�ڱ�
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
//���ԣ�ת��
void tech::action_testcircle()
{
	if (InFollow == false)
	{
		if (InTestCircle == false)
		{
			InTestCircle = true;
			pb_testcircle->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(37,218,41) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(0,255,0);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(0,255,0);} ");
		} 
		else
		{
			InTestCircle = false;
			pb_testcircle->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
		}
	}
}


//���ԣ�����
void tech::action_testfollow()
{
	if (InTestCircle == false 
		&& InOneCircleAction == false)
	{
		if (InFollow == false)
		{
			InFollow = true;
			pb_testfollow->setStyleSheet(
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(37,218,41) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(0,255,0);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(0,255,0);} ");

			QList<int>::iterator it;
			pubcom.activeprintlistmutex.lockForRead();

			//�������ͷ
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				//������׼��״̬�Ÿ���
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
				"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
			QList<int>::iterator it;
			pubcom.activeprintlistmutex.lockForRead();

			//�������ͷ
			for (it = pubcom.activeprintlist.begin(); it
				!= pubcom.activeprintlist.end(); ++it)
			{
				//������׼��״̬�Ÿ���
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


//̧��
void tech::on_pushButton_16_clicked()
{
	interfaceevent* ptempevent = new interfaceevent();

	//ֻ�������±�ʶ���е�ʱ��Ŵ���ť
	if (printopflag != true)
	{
		publiccaution.addevent("������","̧��","�û���̧����ť",1);

		ptempevent->cmd = 0x02;//��ͷ��������
		ptempevent->status = 0x00;//����״̬
		ptempevent->data[0] = 0x03;//С�����֣��·Ż�����̧

		printopflag = true;
		//����һ���¼�����̨�߳�
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}


//��̨�ر�
void tech::on_pushButton_17_clicked()
{
	interfaceevent* ptempevent = new interfaceevent();

	//ֻ���ڷ�ת��ʶ���е�ʱ��Ŵ���ť
	if (printopflag != true)
	{
		publiccaution.addevent("������","��̨����","�û�����̨������ť",1);

		ptempevent->cmd = 0x02;//��ͷ��������
		ptempevent->status = 0x00;//����״̬
		ptempevent->data[0] = 0x08;//��̨�ر�
		printopflag = true;

		//����һ���¼��������߳�
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}


//��ת
void tech::on_pushButton_18_clicked()
{
	//����״̬��������ת
	if (pubcom.machprintstatus == 2)
	{
		return;
	}

	//��ͣ����Ƿѹ������������ת�����ǿ���ֹͣ��ת
	if((circleflag == false) && publiccaution.hascaution(PLCLOCATION, PLCHALTCATION))
	{
		return;
	}
	

	QList<int>::iterator it;

	//���̱߳���
	pubcom.activeprintlistmutex.lockForRead();
	for (it = pubcom.activeprintlist.begin(); it
		!= pubcom.activeprintlist.end(); ++it)
	{

		//�����в����Է�ת
		if (pubcom.actionzerostep.value(*it) != 0)
		{
			pubcom.activeprintlistmutex.unlock();
			return;
		}
	}
	pubcom.activeprintlistmutex.unlock();


	interfaceevent* ptempevent = new interfaceevent();

	//ֻ���ڷ�ת��ʶ���е�ʱ��Ŵ���ť
	if (printopflag != true)
	{
		publiccaution.addevent("������","��ת","�û�����ת��ť",1);

		ptempevent->cmd = 0x02;//��ͷ��������
		ptempevent->status = 0x00;//����״̬
		ptempevent->data[0] = 0x54;//��ת
		printopflag = true;

		//����һ���¼��������߳�
		QCoreApplication::postEvent(backendobject, ptempevent);
	}
}

