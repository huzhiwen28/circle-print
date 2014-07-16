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


//���Թ��ܿ��غ�
#define TEST

//�Ƿ��ڹ��ػ������п��غ�
//#define ONIPC

//�Ƿ���CAN��
//#define HAVECANCARD

//�Ƿ���Ĺرյ���
//#define POWEROFF

//�Ƿ���A4N�˶���
//#define HAVEA4NCARD

//�Ƿ��д����豸
//#define HAVESERIAL

//�Ƿ��ӡMODBUS�յ�������
//#define PRINTMODBUSIN

//�Ƿ��ӡMODBUS���͵�����
//#define PRINTMODBUSOUT

//�Ƿ��м��ܹ�
//#define HAVEDOG


//����汾
#define SOFTVERSTR "�汾 Ver1.002"

//һ�����
#define BACKDOORPASS "718191"

//��������
#define DEVPASS "171819"

//�Ƿ��ն����������Ϣ
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

	//�Ŵ���
	void setamp(float b);

	//���ý��ñ��
	void setbumpid(unsigned int id);

	//������ʼ��ʾ��
	void setdisplaylocation(float x,float y);

	//���ú�Բ�����ӵ�����
	void setprintpoint(float x,float y );

	//��ʾ���
	void display(QPainter* painter);

	//��ʼ��
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

//��չ�İ�ť��2ά��ť�¼�
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


	//��̺���ٶ�
	int angel;
	char angelchange;
	float speed;

	//ת���Ƕ�
	int jinbuangel;
	int hongfangangel1;
	int hongfangangel2;

	//������ٶ�
	int motorangel[16];
	char motorangelchange[16];
	float motorspeed[16];

	char screenwork;

	//�����水����ʶ
	bool chgspeedbtflag ; //�ı��ٶȰ�ť�Ƿ��ˣ�
	unsigned char updownstatus;//��ͷ��̧���·�״̬ 0���·�״̬      1����̧״̬

	bool printopflag; //��ͷ������ť

	bool enableprintchgspeed;
	bool printchgspeedflag;

	bool circleflag;

	//��ת����
	char rolldir;

	QImage* image2;

	//��̨1
	QImage* imgmagic1;
	//��̨2
	QImage* imgmagic2;
	//����1
	QImage* imgbump1;
	//����2
	QImage* imgbump2;

	//̧�������
	char upflag[16];
	char updistance[16];
	char upspeed;
	char upmax;

	//������ͷ�������
	QFont f;
	QFont f2;

	//���ƵĻ���
	QPen* graypen;
	QPen* greenpen;
	QPen* blackpen;

	//�����ַ���ʾ��ʱ��
	int cautiondispcount;


	//����le
	le* myle;

	//����ͼƬ
	QImage* backgroundimg;

	//�䲼��תͼ��
	QPicture luobupic;

	//�淿��תͼƬ
	QPicture hongfangpic;

	//��̺��תͼ��
	QPicture xiangtanpic;

	//Բ����תͼ��
	QPicture yuanwangpic;

	//�䲼�ڱ�
	QPicture baibipic;

	//�ڱ۽Ƕ�
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

	//��̨�����ٷֱ���ʾ
	le* magicpercent[16];

	//�����ʶ
	QLabel * zerolb[16];

	QPixmap* zeropix[2];

	//��ͷ״̬
	QLabel* PrintStatusLb[16];
	QPixmap* PrintStatuspix[7];
	QPixmap PrintStatusImg[7];

	//�����ϵ���ͷ״̬
	unsigned char UIprintstatus[16];

	//��������̬��������ui�ļ�����
	mylabel* movelabel;

	//�������bar����ʾ���ϵ���Ϣ
	QLabel* topbar;

	//����ʱ��
	QLabel* datetime;

	//�����ַ���ʾ
	QLabel* Cautiondisplay;

	//�Ƿ���
	QLabel* LicOverDisplay;

	//�����ϵ�ʱ��
	QString OldDataTime;

	//��ʷ����״̬ 0:�޹��� 1���й��ϣ���Ϊ���ϱ�����˸֮��
	bool HisCautionStatus;

	//��ʷ����״̬ 0:�޹��� 1���й���,��Ϊ�����ַ�״̬����֮��
	bool HisCautionStatus2;

	//�¼�������
	int paintEventcnt;

private:
	Ui::techClass ui;
	void pbinit();

public:
	//�ϼ���
	double oldcount;

	//�¼���
	double newcount;

	//�ܼ���
	unsigned long long sumcount;

	//��ʱ������
	int savecurworknumbertimer;

	//�Ƿ񱣴����
	bool savecurworknumber;

	//���ò���
	int setworknumber;

	//��ǰ�Ĳ���
	int curworknumber;

	QPoint OldMouseLocation;

	//����������ʶ
	bool SerialHeartBeatFlag;

    //�ϵ��ٶȣ���Ϊ�ٶȸı�����
	double oldmotorspeed;//m/min


#ifdef HAVEDOG
	//���ܹ���ѯ�趨��ֵ
	int dogquerytimerset;
	//���ܹ���ѯ��ʱ��
	int dogquerytimer;
	//���ܹ���ѯʧ�ܼ���
	int dogqueryfailcount;

	//���ܹ��������Ƿ���Ч
	char dogdateflag;

	//���ܹ��б��������
	QDate licensedata;

	//�鿴�����Ƿ��ڶ�ʱ��
	int checklictimer;


#endif
	//��Ʒ���ڱ�ʶ,0 ���� 1���� 2ͣ��
	int licovertimestatus;
	int hislicovertimestatus; //��ʷ״̬

#ifdef TEST
	//���԰�ť
	QPushButton* pb_testcircle;
	QPushButton* pb_testfollow;

	//�Ƿ������Է�ת��ʶ
	bool InTestCircle;

	//�Ƿ�������
	bool InFollow;

	//�Ƿ���һ����ת�����У�����ڽ��治��ť��ʱ�������ʶλ����������Ч����Ϊ������Ҫһ���ӳ�ʱ��������
	bool InOneCircleAction;
#endif


public slots:
	void timeupdate();
	//���ò���
	void on_pushButton_2_clicked();

	//��ͷ�·���̧
	void on_pushButton_3_clicked();

	//��ͷ��ת
	void on_pushButton_4_clicked();

	void on_pushButton_5_clicked();
	void on_pushButton_6_clicked();

	//����
	void on_pushButton_7_clicked();

	//��ͷ����
	void on_pushButton_8_clicked();

	//ѡ����ͷ
	void on_pushButton_9_clicked();

	//����
	void on_pushButton_10_clicked();

	//��������
	void on_pushButton_11_clicked();

	//��������
	void on_pushButton_12_clicked();

	//��ͷ����
	void on_pushButton_13_clicked();

	//����
	void on_pushButton_14_pressed();
	void on_pushButton_14_released();

	//����
	void on_pushButton_15_pressed();
	void on_pushButton_15_released();

#ifdef TEST
	//���ԣ�ת��
	void action_testcircle();

	//���ԣ�����
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


//��ʱ��������
class timermng
{
public:
	//������ʱ��
	QMap<unsigned int,unsigned long> timermap;

	//���̱߳�����
	QReadWriteLock mutex;

	int addtimer();
	int deltimer(int timerid);
	unsigned long gettimerval(int timerid);
	int flush();
};




#endif // TECH_H
