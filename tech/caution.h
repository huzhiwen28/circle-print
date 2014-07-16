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

	//�������bar����ʾ���ϵ���Ϣ
	QLabel* topbar;

	//��ʾ���ϵ�
	QLabel* cautionslight[24];
	QLabel* cautionslighttext[24];

	//��ʾ���ϵƵĵ�ǰ״̬
	bool cautionslightstatus[24];

	void init(int modright);

	//��ʷ����
    QSqlQueryModel model;
    QTableView *view;

    //�¼�
    QSqlQueryModel model2;
    QTableView *view2;

	//���Լ�¼
	QSqlQueryModel model3;
	QTableView *view3;

	//����ʱ��
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

	//ˢ����ʷ���Ϻ��¼��Ķ�ʱ��
	int flushtimer;

private:
    Ui::cautionClass ui;


public slots:
//��ť
void on_pushButton_4_clicked();

void timeupdate();//��ʱ���������

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

//����
typedef struct
{
	unsigned int location;//����λ��
	unsigned int code;//���Ͼ������
	unsigned int para;//���ϵĲ���
	QDateTime datetime;//���ϲ��������ں�ʱ��
}t_caution;


class CautionList
{

public:

	//ȡ�ù��ϵ�����
	int count();

	//����indexȡ�ù���
	int getcautionbyindex(int index,t_caution& caution);

	//ȡ�ù����ַ���
	int cautionstr(QString& str);

	//���������б����Ƿ�����Ӧ�Ĺ���
	bool hascaution(const int location);
	bool hascaution(const int location,const int caution);
	bool hascaution(const int location,const int caution,const int para);
	int getcautions(const int location,QList<t_caution>& cautions);
	int addcaution(const int location,const int caution,const int para = 0); //���ϵĲ���
	int delcaution(const int location,const int caution);
	int delcaution(const int location);
	int addhistorycaution(const int location,const int caution,const int para = 0);//��ʷ���ϼ�¼

	//�����¼���¼
	int addevent(const int location,const int event,const QString detail = QString(""),char eventtype = 0); // type 0:�쳣��¼ 1��������¼
	int addevent(const QString location,const QString event,const QString detail = QString(""),char eventtype = 0); // type 0:�쳣��¼ 1��������¼

	//����������Ϣ
	int adddbginfo(const QString detail = QString(""));

	//����IOӳ�䣬���ϣ�������ռ����Ӻ�IPC����Ĺ�����Ϣ
	bool IOHasCaution();

	//����IOӳ�䣬����������ռ����Ӻ�IPC����Ĵ�����Ϣ
	bool IOHasErr();

private:

	 QReadWriteLock mutex;

	//�����б�
	QLinkedList<t_caution> cautionlist;

};


extern CautionList publiccaution;

#endif // CAUTION_H
