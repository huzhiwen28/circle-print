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

#ifndef CAUTION_H
#define CAUTION_H

#include <QTimer>
#include <QtGui/QDialog>
#include <QDateTime>
#include "ui_caution.h"
#include <QPixmap>
#include <QLabel>
#include <QBitmap>
#include <QtSql>
#include <QTableView>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QMutex>
#include <QLinkedList>
#include <QReadWriteLock>


class caution : public QDialog
{
    Q_OBJECT

public:
    caution(QWidget *parent = 0);
    ~caution();
	QTimer mytimer;
	QTimer* timer;

	QPixmap greenlight, redlight, yellowlight;

	QBitmap lightmask;

	//最上面的bar，显示故障等信息
	QLabel* topbar;

	//显示故障灯
	QLabel* cautionslight[24];
	QLabel* cautionslighttext[24];

	//显示故障灯的当前状态
	bool cautionslightstatus[24];

	void init(int modright);

	//历史故障
    QSqlQueryModel model;
    QTableView *view;

    //事件
    QSqlQueryModel model2;
    QTableView *view2;

	//调试记录
	QSqlQueryModel model3;
	QTableView *view3;

	//日期时间
	QLabel* datetime;

	int hiscautionsqlcurpage;
	int hiscautionsqlallpage;
	
	int eventsqlcurpage;
	int eventsqlallpage;

	int dbginfosqlcurpage;
	int dbginfosqlallpage;

	int oldhiscautionval;
	int oldeventval;
	int olddbginfoval;

	bool autoflushevent;
	bool autoflushhiscaution;
	bool autoflushdbginfo;

	//刷新历史故障和事件的定时器
	int flushtimer;

private:
    Ui::cautionClass ui;


public slots:
//按钮
void on_pushButton_4_clicked();

void timeupdate();//定时处理的任务

private slots:
	void on_pushButton_15_clicked();
	void on_pushButton_13_clicked();
	void on_pushButton_14_clicked();
	void on_pushButton_16_clicked();
	void on_pushButton_12_clicked();
	void on_pushButton_11_clicked();
	void on_pushButton_10_clicked();
	void on_pushButton_6_clicked();
	void on_pushButton_9_clicked();
	void on_pushButton_7_clicked();
	void on_pushButton_8_clicked();
	void on_pushButton_5_clicked();
	void on_pushButton_3_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_clicked();
};

//故障
typedef struct
{
	unsigned int location;//故障位置
	unsigned int code;//故障具体编码
	unsigned int para;//故障的参数
	QDateTime datetime;//故障产生的日期和时间
}t_caution;


class CautionList
{

public:

	//取得故障的数量
	int count();

	//根据index取得故障
	int getcautionbyindex(int index,t_caution& caution);

	//取得故障字符串
	int cautionstr(QString& str);

	//搜索故障列表中是否有相应的故障
	bool hascaution(const int location);
	bool hascaution(const int location,const int caution);
	bool hascaution(const int location,const int caution,const int para);
	int getcautions(const int location,QList<t_caution>& cautions);
	int addcaution(const int location,const int caution,const int para = 0); //故障的参数
	int delcaution(const int location,const int caution);
	int delcaution(const int location);
	int addhistorycaution(const int location,const int caution,const int para = 0);//历史故障记录

	//增加事件记录
	int addevent(const int location,const int event,const QString detail = QString(""),char eventtype = 0); // type 0:异常记录 1：操作记录
	int addevent(const QString location,const QString event,const QString detail = QString(""),char eventtype = 0); // type 0:异常记录 1：操作记录

	//新增调试信息
	int adddbginfo(const QString detail = QString(""));

	//对外IO映射，故障，这个是收集板子和IPC本身的故障信息
	bool IOHasCaution();

	//对外IO映射，报错，这个是收集板子和IPC本身的错误信息
	bool IOHasErr();

private:

	 QReadWriteLock mutex;

	//故障列表
	QLinkedList<t_caution> cautionlist;

};


extern CautionList publiccaution;

#endif // CAUTION_H
