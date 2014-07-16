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

//������ͷ��������
#define OPENPRINTOP

class opprint : public QDialog
{
    Q_OBJECT

public:
    opprint(QWidget *parent = 0);
    ~opprint();

	void init(int modright);

	//��ͷ������ʶ 0û�д��� 1������
	bool inactionflag;

	QTabWidget *tabWidget;
	QWidget* tab[25];

	//����
	QLabel* labels[24][7];

	//��ť
	pbex* chgpb[24][7];
	
	//��ťλ��
	QRect pbloc[24][7];

	//�б༭��
	le* myle[24][12];

	//�������bar����ʾ���ϵ���Ϣ
	QLabel* topbar;

	//����ʱ��
	QLabel* datetime;

	//�����ַ���ʾ
	QLabel* Cautiondisplay;

	QTimer mytimer;
	QTimer* timer;


	QImage* image2;
	QImage* backgroundimg;

	//������ͷ�������
	QFont f;

	//����ͼƬ
	QLabel* background[16];

	//���
	QTableWidget* table[7];

	//���Ԫ
	QTableWidgetItem * tableitem[16][6];

	//��������
	QTableWidgetItem * tablehead[10];


	//��ͷ���
	QLabel* PrintNumLb[16];

	//��ͷ״̬��IO״̬
	QLabel* PrintStatus[16][6];

	//����ʹ�õ�IO
	QLabel* IOStatus[16][16];

	//�߿�,���ò�������IO�õ�
	QFrame* Frameset[16][2];

	//��ʾ����IO
	QLabel* BumpIOLb[16];
	//��ʾ��̨IO
	QLabel* MagicIOLb[16];
	//��ʾ��̨�ٷֱ�
	QLabel* MagicPercentLb[16];

private:
    Ui::opprintClass ui;

	//�ϵ����λ��
	QPoint OldMouseLocation;

	//��ͷ��ű���ͼƬ
	QPixmap* nobkgrd[16];

	//�����ϵ���ͷ״̬
	unsigned char UIprintstatus[16];

	//�����IO״̬
    unsigned char UIlockreleasestatus[16];
	unsigned char UIupdownstatus[16];
	unsigned char UIcolorpipestatus[16];
	unsigned char UImagicstatus[16];
	unsigned char UIbumpstatus[16];

	//����״̬�仯��ʱ��,��Ϊ���ֱ�����˸֮��
	unsigned char UIStatusChgTimer[16][6];

	//λ��
	int UIxstatus[16];
	int UIystatus[16];
	int UIzstatus[16];
	int UImagicpercent[16];

public slots:
void timeupdate();//��ʱ���������
void action_clicked(int printno,int action);

protected:
void mousePressEvent ( QMouseEvent * event );
void customEvent(QEvent *e);


private slots:
	void on_pushButton_clicked();
};

#endif // OPPRINT_H
