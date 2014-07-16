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

#ifndef OPPRINT_H
#define OPPRINT_H

#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QPen>
#include <QTime>
#include <QFile>
#include <QLineEdit>
#include <QTabWidget>
#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QPen>
#include <QTime>
#include <QFile>
#include <QLineEdit>
#include <QtGui/QDialog>
#include <QPushButton>
#include <QLabel>
#include <QtGui/QDialog>
#include <QTabWidget>
#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QDomNode>
#include <QList>
#include <QMap>
#include "ui_opprint.h"
#include "tech.h"

//开启网头操作界面
#define OPENPRINTOP

class opprint : public QDialog
{
    Q_OBJECT

public:
    opprint(QWidget *parent = 0);
    ~opprint();

	void init(int modright);

	//网头命令处理标识 0没有处理 1处理中
	bool inactionflag;

	QTabWidget *tabWidget;
	QWidget* tab[25];

	//标题
	QLabel* labels[24][7];

	//按钮
	pbex* chgpb[24][7];
	
	//按钮位置
	QRect pbloc[24][7];

	//行编辑框
	le* myle[24][12];

	//最上面的bar，显示故障等信息
	QLabel* topbar;

	//日期时间
	QLabel* datetime;

	//故障字符显示
	QLabel* Cautiondisplay;

	QTimer mytimer;
	QTimer* timer;


	QImage* image2;
	QImage* backgroundimg;

	//绘制网头编号字体
	QFont f;

	//背景图片
	QLabel* background[16];

	//表格
	QTableWidget* table[7];

	//表格单元
	QTableWidgetItem * tableitem[16][6];

	//表格标题栏
	QTableWidgetItem * tablehead[10];


	//网头编号
	QLabel* PrintNumLb[16];

	//网头状态和IO状态
	QLabel* PrintStatus[16][6];

	//测试使用的IO
	QLabel* IOStatus[16][16];

	//边框,放置操作面板的IO用的
	QFrame* Frameset[16][2];

	//显示浆泵IO
	QLabel* BumpIOLb[16];
	//显示磁台IO
	QLabel* MagicIOLb[16];
	//显示磁台百分比
	QLabel* MagicPercentLb[16];

private:
    Ui::opprintClass ui;

	//老的鼠标位置
	QPoint OldMouseLocation;

	//网头编号背景图片
	QPixmap* nobkgrd[16];

	//界面上的网头状态
	unsigned char UIprintstatus[16];

	//界面的IO状态
    unsigned char UIlockreleasestatus[16];
	unsigned char UIupdownstatus[16];
	unsigned char UIcolorpipestatus[16];
	unsigned char UImagicstatus[16];
	unsigned char UIbumpstatus[16];

	//界面状态变化定时器,作为文字背景闪烁之用
	unsigned char UIStatusChgTimer[16][6];

	//位置
	int UIxstatus[16];
	int UIystatus[16];
	int UIzstatus[16];
	int UImagicpercent[16];

public slots:
void timeupdate();//定时处理的任务
void action_clicked(int printno,int action);

protected:
void mousePressEvent ( QMouseEvent * event );
void customEvent(QEvent *e);


private slots:
	void on_pushButton_clicked();
};

#endif // OPPRINT_H
