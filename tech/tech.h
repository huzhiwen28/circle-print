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

#ifndef TECH_H
#define TECH_H

#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QPen>
#include <QTime>
#include <QFile>
#include <QLineEdit>
#include "ui_tech.h"
#include <QPicture>
#include <QMutex>
#include "custevent.h"
#include <QReadWriteLock>
#include <windows.h>


//测试功能开关宏
#define TEST

//是否在工控机上运行开关宏
//#define ONIPC

//是否有CAN卡
//#define HAVECANCARD

//是否真的关闭电脑
//#define POWEROFF

//是否有A4N运动卡
//#define HAVEA4NCARD

//是否有串口设备
//#define HAVESERIAL

//是否打印MODBUS收到的数据
//#define PRINTMODBUSIN

//是否打印MODBUS发送的数据
//#define PRINTMODBUSOUT

//是否有加密狗
//#define HAVEDOG


//软件版本
#define SOFTVERSTR "版本 Ver1.002"

//一般后门
#define BACKDOORPASS "718191"

//开发后门
#define DEVPASS "171819"

//是否终端输出调试信息
//#define CONSOLE_OUTPUT 

#ifdef CONSOLE_OUTPUT
#define Dbgout printf
#else
#define Dbgout emptyfunc
#endif

extern QObject* backendobject;
extern QObject* interfaceobject;
extern QObject* a4nobject;

void emptyfunc(...);

class bump
{
public:

	//放大倍数
	void setamp(float b);

	//设置浆泵编号
	void setbumpid(unsigned int id);

	//设置起始显示点
	void setdisplaylocation(float x,float y);

	//设置和圆网连接点坐标
	void setprintpoint(float x,float y );

	//显示组件
	void display(QPainter* painter);

	//初始化
	void init();

private:
	float amp;
	float locx,locy;
	float printpointx,printpointy;
	QPainterPath bumpbase;
	QPainterPath bumpcube;
	QPainterPath bumplinkcube;
	QPainterPath bumptri;
	QPainterPath liquid;
	float liquidy;
	unsigned int bumpid;
};

class mylabel : public QLabel
{
	Q_OBJECT
public:
	mylabel(QWidget *parent = 0)		;
	~	mylabel();

protected:
	void paintEvent(QPaintEvent *);

private:
};

class KeyPressEater: public QObject
{
	Q_OBJECT

public:
	KeyPressEater(QObject *parent = 0);
	~	KeyPressEater();

protected:
	bool eventFilter(QObject *obj, QEvent *event);
};


class pb: public QPushButton
{
	Q_OBJECT

public:
	pb(QWidget *parent = 0)		;
	~	pb();

	int num;
	void setnum(int n);

signals:
	void buttonClicked(int num);

public slots:
	void sendsignal();

};

//扩展的按钮，2维按钮事件
class pbex: public QPushButton
{
	Q_OBJECT

public:
	pbex(QWidget *parent = 0)		;
	~	pbex();

	int x;
	int y;
	void setnum(int n,int m);
	void setnormalIcon(QString picfilename);
	void setonIcon(QString picfilename);
	void seticonSize(QSize size);

signals:
	void buttonClicked(int x,int y);

public slots:
	void sendsignal();
};




class le: public QLineEdit
{
	Q_OBJECT

public:
	le(QWidget *parent = 0)		;
	~	le();

	int num;
	void setnum(int n);

signals:
	void leClicked(int num);

protected:
	void mousePressEvent ( QMouseEvent * event );
};


class tech: public QDialog
{
	Q_OBJECT

public:
	tech(QWidget *parent = 0)		;
	~	tech();
	void init();

	QTimer mytimer;
	QTimer* timer;

	int timecount;


	//橡毯的速度
	int angel;
	char angelchange;
	float speed;

	//转动角度
	int jinbuangel;
	int hongfangangel1;
	int hongfangangel2;

	//电机的速度
	int motorangel[16];
	char motorangelchange[16];
	float motorspeed[16];

	char screenwork;

	//主界面按键标识
	bool chgspeedbtflag ; //改变速度按钮是否按了？
	unsigned char updownstatus;//网头上抬和下放状态 0：下放状态      1：上抬状态

	bool printopflag; //网头操作按钮

	bool enableprintchgspeed;
	bool printchgspeedflag;

	bool circleflag;

	//旋转方向
	char rolldir;

	QImage* image2;

	//磁台1
	QImage* imgmagic1;
	//磁台2
	QImage* imgmagic2;
	//浆泵1
	QImage* imgbump1;
	//浆泵2
	QImage* imgbump2;

	//抬起的描叙
	char upflag[16];
	char updistance[16];
	char upspeed;
	char upmax;

	//绘制网头编号字体
	QFont f;
	QFont f2;

	//绘制的画笔
	QPen* graypen;
	QPen* greenpen;
	QPen* blackpen;

	//故障字符显示计时器
	int cautiondispcount;


	//定制le
	le* myle;

	//背景图片
	QImage* backgroundimg;

	//落布旋转图像
	QPicture luobupic;

	//烘房旋转图片
	QPicture hongfangpic;

	//橡毯旋转图像
	QPicture xiangtanpic;

	//圆网旋转图像
	QPicture yuanwangpic;

	//落布摆臂
	QPicture baibipic;

	//摆臂角度
	float baibiangel;
	char baibidir;

	float ampify;
	QPainterPath path1;
	QPainterPath path2;
	QPainterPath path3;
	QPainterPath path4[10];

	char backgroundflag;

	int backflushtimer;

	bump mybump[16];

	float myx,myy;

	//磁台磁力百分比显示
	le* magicpercent[16];

	//对零标识
	QLabel * zerolb[16];

	QPixmap* zeropix[2];

	//网头状态
	QLabel* PrintStatusLb[16];
	QPixmap* PrintStatuspix[7];
	QPixmap PrintStatusImg[7];

	//界面上的网头状态
	unsigned char UIprintstatus[16];

	//动画，静态背景放在ui文件中了
	mylabel* movelabel;

	//最上面的bar，显示故障等信息
	QLabel* topbar;

	//日期时间
	QLabel* datetime;

	//故障字符显示
	QLabel* Cautiondisplay;

	//是否到期
	QLabel* LicOverDisplay;

	//保存老的时间
	QString OldDataTime;

	//历史故障状态 0:无故障 1：有故障，作为故障背景闪烁之用
	bool HisCautionStatus;

	//历史故障状态 0:无故障 1：有故障,作为故障字符状态跟踪之用
	bool HisCautionStatus2;

	//事件计数器
	int paintEventcnt;

private:
	Ui::techClass ui;
	void pbinit();

public:
	//老计数
	double oldcount;

	//新计数
	double newcount;

	//总计数
	unsigned long long sumcount;

	//定时计数器
	int savecurworknumbertimer;

	//是否保存产量
	bool savecurworknumber;

	//设置产量
	int setworknumber;

	//当前的产量
	int curworknumber;

	QPoint OldMouseLocation;

	//串口心跳标识
	bool SerialHeartBeatFlag;

    //老的速度，作为速度改变依据
	double oldmotorspeed;//m/min


#ifdef HAVEDOG
	//加密狗查询设定的值
	int dogquerytimerset;
	//加密狗查询计时器
	int dogquerytimer;
	//加密狗查询失败计数
	int dogqueryfailcount;

	//加密狗中日期是否有效
	char dogdateflag;

	//加密狗中保存的日期
	QDate licensedata;

	//查看日期是否到期定时器
	int checklictimer;


#endif
	//产品到期标识,0 正常 1警告 2停机
	int licovertimestatus;
	int hislicovertimestatus; //历史状态

#ifdef TEST
	//测试按钮
	QPushButton* pb_testcircle;
	QPushButton* pb_testfollow;

	//是否进入测试反转标识
	bool InTestCircle;

	//是否进入跟随
	bool InFollow;

	//是否在一个自转动作中，这个在界面不按钮的时候，这个标识位可能依旧有效，因为动作需要一个延迟时间才能完成
	bool InOneCircleAction;
#endif


public slots:
	void timeupdate();
	//设置产量
	void on_pushButton_2_clicked();

	//网头下放上抬
	void on_pushButton_3_clicked();

	//网头反转
	void on_pushButton_4_clicked();

	void on_pushButton_5_clicked();
	void on_pushButton_6_clicked();

	//对零
	void on_pushButton_7_clicked();

	//网头操作
	void on_pushButton_8_clicked();

	//选择网头
	void on_pushButton_9_clicked();

	//故障
	void on_pushButton_10_clicked();

	//产量复零
	void on_pushButton_11_clicked();

	//生产管理
	void on_pushButton_12_clicked();

	//网头调速
	void on_pushButton_13_clicked();

	//加速
	void on_pushButton_14_pressed();
	void on_pushButton_14_released();

	//减速
	void on_pushButton_15_pressed();
	void on_pushButton_15_released();

#ifdef TEST
	//测试：转动
	void action_testcircle();

	//测试：跟随
	void action_testfollow();
#endif


protected:
	void mousePressEvent ( QMouseEvent * event );
	void customEvent(QEvent *e);

private slots:
	void on_pushButton_18_clicked();
	void on_pushButton_17_clicked();
	void on_pushButton_16_clicked();
};


//定时器管理器
class timermng
{
public:
	//公共计时器
	QMap<unsigned int,unsigned long> timermap;

	//多线程保护锁
	QReadWriteLock mutex;

	int addtimer();
	int deltimer(int timerid);
	unsigned long gettimerval(int timerid);
	int flush();
};




#endif // TECH_H
