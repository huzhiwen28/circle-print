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

#ifndef SET_H
#define SET_H

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
#include <QDateTimeEdit>
#include <QDateTime>
#include "ui_set.h"
#include "tech.h"

//表格x、y位置
class tablekey
{
public:

	int x;
	int y;
};

inline bool operator<(const tablekey&e1, const tablekey &e2)
{
	if (e1.x + e1.y * 10 < e2.x + e2.y * 10)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//表格的每个单元格，内存中
class tablenode
{
public:

	QTableWidgetItem* tableitem;

	//对应参数的ID
	QDomNode id;

	//对应参数的值
	QDomNode node;

	//对应参数的改变标识，0没有修改 1有修改
	QDomNode flag;

};



//保存每个用户密码和权限
class passright
{
public:
	QString pass;
	QString right;
	unsigned char flag;

};

class set: public QDialog
{
Q_OBJECT

public:
    set(QWidget *parent = 0)		;
~	set();

	//根据权限生成TAB页面
			void init(int modright);
			int showitem(QDomNode* p,QString groupname);
			void SetUserRole(int role);

			//标题栏
			QList<QTableWidgetItem *> tableheadlist;

			//工艺表格单元，存放值
			QMap<tablekey,tablenode> tablemapprod;
			QMap<tablekey,QTableWidgetItem* > tablemapprodshow;

			//机器表格单元1
			QMap<tablekey,tablenode> tablemapmachsub1;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub1show;

			//机器表格单元2
			QMap<tablekey,tablenode> tablemapmachsub2;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub2show;

			//机器表格单元3
			QMap<tablekey,tablenode> tablemapmachsub3;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub3show;

			//机器表格单元4
			QMap<tablekey,tablenode> tablemapmachsub4;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub4show;

			//机器表格单元5
			QMap<tablekey,tablenode> tablemapmachsub5;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub5show;

			//机器表格单元6
			QMap<tablekey,tablenode> tablemapmachsub6;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub6show;

			//调试表格单元
			QMap<tablekey,tablenode> tablemaptune;
			QMap<tablekey,QTableWidgetItem* > tablemaptuneshow;

			//开发表格单元
			QMap<tablekey,tablenode> tablemapdev;
			QMap<tablekey,QTableWidgetItem* > tablemapdevshow;

			//MODBUS表格单元
			QMap<tablekey,tablenode> tablemapmodbus;
			QMap<tablekey,QTableWidgetItem* > tablemapmodbusshow;

			//PLCIO表格单元
			QMap<tablekey,tablenode> tablemapplcio;
			QMap<tablekey,QTableWidgetItem* > tablemapplcioshow;

			//PLCPARA表格单元
			QMap<tablekey,tablenode> tablemapplcpara;
			QMap<tablekey,QTableWidgetItem* > tablemapplcparashow;

			//驱动器表格单元
			QMap<tablekey,tablenode> tablemapdriver;
			QMap<tablekey,QTableWidgetItem* > tablemapdrivershow;

			//权限管理
			QLabel* righthead[3];
			QLabel* rightperson[3];
			pb* rightchgpasswd[3];
			pbex* rightchoose[3][7];
			QList<passright> passrightlist;

			QTimer mytimer;
			QTimer* timer;

			//最上面的bar，显示故障等信息
			QLabel* topbar;

			//日期时间
			QLabel* datetimebar;

			//故障字符显示
			QLabel* Cautiondisplay;

			QImage* image2;

			//绘制网头编号字体
			QFont f;

			//使用列表存储各个控件
			QList<QTabWidget *> tablist;

			//存放tab页子控件
			QList<QWidget *> widgetlist;

			QList<QTableWidget* > tablelist;

			//网头选择标识
			QList<char > chooseprintlist;

			//网头选择按钮
			QList<pb* > mypblist;

			//网头序号label
			QList<QLabel* > lbprintlist;

			//序号label
			QList<QLabel* > lblist;

			//网头编号按钮
			QList<pb* > pbprintnumlist;

			//网头编号数据
			QList<int > iprintnumlist;

			//自动编号按钮
			QPushButton* pbautoassign;

			//标题
			QList<QLabel* > lbtitlelist;

			//系统设置控件
			QList<QFrame*> ssframelist;
			QPushButton* ssbtscreemsaver;
			QList<QLabel*> sslabellist;
			le* ssminute;

			char ssscreemsaverflag;
			QString ssscreemsavertime ;

			QDateTimeEdit* dte;

			QDateTime datetime;

			//备份日志和配置文件
			QPushButton* backuplog;

			//清除日志
			QPushButton* clearlog;

			//备份机器指纹
			QPushButton* backupfinger;

			//许可证升级
			QPushButton* updatelicense;

			//异常记录按钮
			QPushButton* btexceptrec;

			//操作记录按钮
			QPushButton* btoprec;

			//网头是否设置激活
			bool printactiveflag;

			//摩擦系数测量按钮
			QPushButton* MeasurePulsePerMeter;

			//清除dbginfo记录按钮
			QPushButton* btcleardbginfo;

			//角色id 工艺1 机器2 调试3
			int role;

			//校验几个机器参数参数的唯一性
			bool VeryfyMachParaUniq();


		public slots:

     		//确定按钮
			void on_pushButton_clicked();

			//应用按钮
			void on_pushButton_2_clicked();

			//取消按钮
			void on_pushButton_3_clicked();

			//修改密码按钮
			void on_pushButton_9_clicked();

			//强制刷新按钮
			void on_pushButton_10_clicked();

			//备份按钮处理
			void backuplog_clicked();

			//清除日志处理
			void clearlog_clicked();

			//备份机器指纹
			void backupfinger_clicked();

			//升级许可证
			void updatelicense_clicked();

 			//工艺参数表格处理函数
			void cellprod_entered(int row, int column);

			//机器参数表格处理函数
			void cellmachsub1_entered(int row, int column);

			//机器参数表格处理函数
			void cellmachsub2_entered(int row, int column);

			//机器参数表格处理函数
			void cellmachsub3_entered(int row, int column);

			//机器参数表格处理函数
			void cellmachsub4_entered(int row, int column);

			//机器参数表格处理函数
			void cellmachsub5_entered(int row, int column);

			//机器参数表格处理函数
			void cellmachsub6_entered(int row, int column);


			//调试参数表格处理函数
			void celltune_entered(int row, int column);

			//MODBUS参数表格处理函数
			void cellmodbus_entered(int row, int column);

			//开发参数表格处理函数
			void celldev_entered(int row, int column);

			//PLCIO参数表格处理函数
			void cellplcio_entered(int row, int column);

			//驱动器参数处理
			void celldriver_entered(int row, int column);
			
			//PLCPARA参数表格处理函数
			void cellplcpara_entered(int row, int column);

			//开发参数表格处理函数
			void rightchgpasswd_clicked(int num);

			//开发参数表格处理函数
			void rightchgright_clicked(int x,int y);

			void timeupdate();//定时处理的任务

			void swicthscreensaver_clicked(); //开关屏保

			void screensaverle_clicked(int num); //屏保时间

			void datetime_change(const QDateTime & datetime );

			void updatedatetime();//系统设置中更新时间和日期

			void MeasurePulsePerMeterDlg();//测试摩擦系数界面

			void btoprec_clicked(); //是否记录操作

			void btexceptrec_clicked(); //是否记录异常

			void btcleardbginfo_clicked();//清除dbginfo表格

		protected:
			void mousePressEvent ( QMouseEvent * event );
			void customEvent(QEvent *e);

		private:
			Ui::setClass ui;
		};

#endif // SET_H
