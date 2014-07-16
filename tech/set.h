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

//���x��yλ��
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

//����ÿ����Ԫ���ڴ���
class tablenode
{
public:

	QTableWidgetItem* tableitem;

	//��Ӧ������ID
	QDomNode id;

	//��Ӧ������ֵ
	QDomNode node;

	//��Ӧ�����ĸı��ʶ��0û���޸� 1���޸�
	QDomNode flag;

};



//����ÿ���û������Ȩ��
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

	//����Ȩ������TABҳ��
			void init(int modright);
			int showitem(QDomNode* p,QString groupname);
			void SetUserRole(int role);

			//������
			QList<QTableWidgetItem *> tableheadlist;

			//���ձ��Ԫ�����ֵ
			QMap<tablekey,tablenode> tablemapprod;
			QMap<tablekey,QTableWidgetItem* > tablemapprodshow;

			//�������Ԫ1
			QMap<tablekey,tablenode> tablemapmachsub1;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub1show;

			//�������Ԫ2
			QMap<tablekey,tablenode> tablemapmachsub2;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub2show;

			//�������Ԫ3
			QMap<tablekey,tablenode> tablemapmachsub3;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub3show;

			//�������Ԫ4
			QMap<tablekey,tablenode> tablemapmachsub4;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub4show;

			//�������Ԫ5
			QMap<tablekey,tablenode> tablemapmachsub5;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub5show;

			//�������Ԫ6
			QMap<tablekey,tablenode> tablemapmachsub6;
			QMap<tablekey,QTableWidgetItem* > tablemapmachsub6show;

			//���Ա��Ԫ
			QMap<tablekey,tablenode> tablemaptune;
			QMap<tablekey,QTableWidgetItem* > tablemaptuneshow;

			//�������Ԫ
			QMap<tablekey,tablenode> tablemapdev;
			QMap<tablekey,QTableWidgetItem* > tablemapdevshow;

			//MODBUS���Ԫ
			QMap<tablekey,tablenode> tablemapmodbus;
			QMap<tablekey,QTableWidgetItem* > tablemapmodbusshow;

			//PLCIO���Ԫ
			QMap<tablekey,tablenode> tablemapplcio;
			QMap<tablekey,QTableWidgetItem* > tablemapplcioshow;

			//PLCPARA���Ԫ
			QMap<tablekey,tablenode> tablemapplcpara;
			QMap<tablekey,QTableWidgetItem* > tablemapplcparashow;

			//���������Ԫ
			QMap<tablekey,tablenode> tablemapdriver;
			QMap<tablekey,QTableWidgetItem* > tablemapdrivershow;

			//Ȩ�޹���
			QLabel* righthead[3];
			QLabel* rightperson[3];
			pb* rightchgpasswd[3];
			pbex* rightchoose[3][7];
			QList<passright> passrightlist;

			QTimer mytimer;
			QTimer* timer;

			//�������bar����ʾ���ϵ���Ϣ
			QLabel* topbar;

			//����ʱ��
			QLabel* datetimebar;

			//�����ַ���ʾ
			QLabel* Cautiondisplay;

			QImage* image2;

			//������ͷ�������
			QFont f;

			//ʹ���б�洢�����ؼ�
			QList<QTabWidget *> tablist;

			//���tabҳ�ӿؼ�
			QList<QWidget *> widgetlist;

			QList<QTableWidget* > tablelist;

			//��ͷѡ���ʶ
			QList<char > chooseprintlist;

			//��ͷѡ��ť
			QList<pb* > mypblist;

			//��ͷ���label
			QList<QLabel* > lbprintlist;

			//���label
			QList<QLabel* > lblist;

			//��ͷ��Ű�ť
			QList<pb* > pbprintnumlist;

			//��ͷ�������
			QList<int > iprintnumlist;

			//�Զ���Ű�ť
			QPushButton* pbautoassign;

			//����
			QList<QLabel* > lbtitlelist;

			//ϵͳ���ÿؼ�
			QList<QFrame*> ssframelist;
			QPushButton* ssbtscreemsaver;
			QList<QLabel*> sslabellist;
			le* ssminute;

			char ssscreemsaverflag;
			QString ssscreemsavertime ;

			QDateTimeEdit* dte;

			QDateTime datetime;

			//������־�������ļ�
			QPushButton* backuplog;

			//�����־
			QPushButton* clearlog;

			//���ݻ���ָ��
			QPushButton* backupfinger;

			//���֤����
			QPushButton* updatelicense;

			//�쳣��¼��ť
			QPushButton* btexceptrec;

			//������¼��ť
			QPushButton* btoprec;

			//��ͷ�Ƿ����ü���
			bool printactiveflag;

			//Ħ��ϵ��������ť
			QPushButton* MeasurePulsePerMeter;

			//���dbginfo��¼��ť
			QPushButton* btcleardbginfo;

			//��ɫid ����1 ����2 ����3
			int role;

			//У�鼸����������������Ψһ��
			bool VeryfyMachParaUniq();


		public slots:

     		//ȷ����ť
			void on_pushButton_clicked();

			//Ӧ�ð�ť
			void on_pushButton_2_clicked();

			//ȡ����ť
			void on_pushButton_3_clicked();

			//�޸����밴ť
			void on_pushButton_9_clicked();

			//ǿ��ˢ�°�ť
			void on_pushButton_10_clicked();

			//���ݰ�ť����
			void backuplog_clicked();

			//�����־����
			void clearlog_clicked();

			//���ݻ���ָ��
			void backupfinger_clicked();

			//�������֤
			void updatelicense_clicked();

 			//���ղ����������
			void cellprod_entered(int row, int column);

			//���������������
			void cellmachsub1_entered(int row, int column);

			//���������������
			void cellmachsub2_entered(int row, int column);

			//���������������
			void cellmachsub3_entered(int row, int column);

			//���������������
			void cellmachsub4_entered(int row, int column);

			//���������������
			void cellmachsub5_entered(int row, int column);

			//���������������
			void cellmachsub6_entered(int row, int column);


			//���Բ����������
			void celltune_entered(int row, int column);

			//MODBUS�����������
			void cellmodbus_entered(int row, int column);

			//���������������
			void celldev_entered(int row, int column);

			//PLCIO�����������
			void cellplcio_entered(int row, int column);

			//��������������
			void celldriver_entered(int row, int column);
			
			//PLCPARA�����������
			void cellplcpara_entered(int row, int column);

			//���������������
			void rightchgpasswd_clicked(int num);

			//���������������
			void rightchgright_clicked(int x,int y);

			void timeupdate();//��ʱ���������

			void swicthscreensaver_clicked(); //��������

			void screensaverle_clicked(int num); //����ʱ��

			void datetime_change(const QDateTime & datetime );

			void updatedatetime();//ϵͳ�����и���ʱ�������

			void MeasurePulsePerMeterDlg();//����Ħ��ϵ������

			void btoprec_clicked(); //�Ƿ��¼����

			void btexceptrec_clicked(); //�Ƿ��¼�쳣

			void btcleardbginfo_clicked();//���dbginfo���

		protected:
			void mousePressEvent ( QMouseEvent * event );
			void customEvent(QEvent *e);

		private:
			Ui::setClass ui;
		};

#endif // SET_H
