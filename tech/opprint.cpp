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
#include <QLabel>
#include <QPushButton>
#include <QBitmap>
#include <QPixmap>
#include <QBrush>
#include "opprint.h"
#include "tech.h"
#include <QPainter>
#include <iostream>
#include <QMessageBox>
#include <QSize>
#include "tech.h"
#include "help.h"
#include "caution.h"
#include "canif.h"

extern QString choosestyle2;
extern QString unchoosestyle;
extern QString dlgbkcolor;

QColor bgColorForName(const QString &name);
QColor fgColorForName(const QString &name);

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
extern QPicture topbarpic;
extern QString dlgbkcolor;

extern char cautionbkflag ;
extern char cautionbkchgflag ;
extern char datetimechgflag ;
extern char cautionstrchgflag ;
//��ǰ��ʾ�Ĺ���id
extern char curcautionindex; 

extern QPicture topbarpic;
extern QPicture topbarredpic;
extern QPicture topbargraypic;

extern QString datetimestyle;
extern QString cautionstyle;

opprint::opprint(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);

	ui.pushButton->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	image2 = new QImage();
	image2->load(":/DSC_1422.PNG");

	f.setPointSize(30);

	inactionflag = false;

	topbar = new QLabel(this);
	topbar->setGeometry(QRect(0,0,1024,50));
	topbar->setPicture(topbarpic);

	datetime = new QLabel(topbar);
	datetime->setGeometry(QRect(790,10,231,31));
	datetime->setStyleSheet(datetimestyle);

	Cautiondisplay = new QLabel(topbar);
	Cautiondisplay->setGeometry(QRect(400,0,400,50));
	Cautiondisplay->setStyleSheet(cautionstyle);

	//������ʱ��
	timer = &mytimer;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));

	for(int i=0;i < 16;i++)
	{
		UIprintstatus[i] = 1;
		UIlockreleasestatus[i] = 0;
		UIupdownstatus[i] = 0;
		UIcolorpipestatus[i] = 0;
		UImagicstatus[i] = 0;
		UIbumpstatus[i] = 0;
		UIxstatus[i] = pubcom.xstatus[i];
		UIystatus[i] = pubcom.ystatus[i];
		UIzstatus[i] = pubcom.zstatus[i];
		UImagicpercent[i] = 0;
	}

	for (int i = 0; i< 16; ++i)
	{
		for (int j = 0; j < 6; ++j)
		{
			UIStatusChgTimer[i][j] = 0;
		}
	}

}

opprint::~opprint()
{

}

//��̬TABҳ
void opprint::init(int modright)
{
	publiccaution.addevent("��ͷ����ҳ��","������ͷ����ҳ��","�û���ͷ����ҳ��",1);

	pubcom.CurrentWnd = 1;

	tabWidget = new QTabWidget(this);
	tabWidget->setGeometry(QRect(0, 60, 1024, 670));
	tabWidget->setStyleSheet("font-size: 18px");

	QDomNode para;

	//ȡģ��ID
	para = GetParaByName("modlist", "��ͷ����");
	if (para.isNull())
	{
		publiccaution.addevent("�����ļ�����","Ϊ��","");
		return;
	}

	if ((modright >> (para.firstChildElement("id").text().toInt() - 1) & 0x01)
			== 0x01)
	{

		for (int i = 0; i < pubcom.PrintNum; i++)
		{
			tab[i] = new QWidget();
			if(pubcom.nodeonlinemap.value(i) == false)
			{
#ifdef OPENPRINTOP
#else
				tab[i]->setDisabled(true);
#endif
			}

			int xp = 60;
			int yp = 30;

			Frameset[i][0] = new QFrame(tab[i]);
			Frameset[i][0]->setGeometry(QRect(50, 20, 435, 500));
			Frameset[i][0]->setFrameShadow(QFrame::Plain);
			Frameset[i][0]->setFrameShape(QFrame::Box);
			Frameset[i][0]->setLineWidth(4);

			background[i] = new QLabel(Frameset[i][0]);
			background[i]->setPixmap(QPixmap(":/images/printpad4.PNG"));
			background[i]->setGeometry(QRect(10, 10, 415, 483));

			chgpb[i][0] = new pbex(background[i]);
            chgpb[i][0]->setIcon(QIcon(":/images/action1.PNG"));
			chgpb[i][0]->setIconSize(QSize(46, 46));
			chgpb[i][0]->setnum(i, 0);
			//chgpb[i][0]->setMask(QBitmap(":/images/up_mask.PNG"));
			chgpb[i][0]->setGeometry(QRect(32, 156, 46, 46));
			connect(chgpb[i][0], SIGNAL(buttonClicked(int,int)), this, SLOT(action_clicked(int,int)));
			//pbloc[i][0] = chgpb[i][0]->geometry();

			chgpb[i][1] = new pbex(background[i]);
			chgpb[i][1]->setIcon(QIcon(":/images/action2.PNG"));
			chgpb[i][1]->setIconSize(QSize(46, 46));
			chgpb[i][1]->setnum(i, 1);
			//chgpb[i][1]->setMask(QBitmap(":/images/down_mask.PNG"));
			chgpb[i][1]->setGeometry(QRect(38, 212, 46, 46));
			connect(chgpb[i][1], SIGNAL(buttonClicked(int,int)), this, SLOT(action_clicked(int,int)));

			chgpb[i][2] = new pbex(background[i]);
			chgpb[i][2]->setIcon(QIcon(":/images/action3.PNG"));
			chgpb[i][2]->setIconSize(QSize(46, 46));
			chgpb[i][2]->setnum(i, 2);
			//chgpb[i][2]->setMask(QBitmap(":/images/auto_mask.PNG"));
			chgpb[i][2]->setGeometry(QRect(48, 269, 46, 46));
			connect(chgpb[i][2], SIGNAL(buttonClicked(int,int)), this, SLOT(action_clicked(int,int)));

			chgpb[i][3] = new pbex(background[i]);
			chgpb[i][3]->setIcon(QIcon(":/images/action4.PNG"));
			chgpb[i][3]->setIconSize(QSize(46, 46));
			chgpb[i][3]->setnum(i, 3);
			//chgpb[i][2]->setMask(QBitmap(":/images/auto_mask.PNG"));
			chgpb[i][3]->setGeometry(QRect(65, 322, 46, 46));
			connect(chgpb[i][3], SIGNAL(buttonClicked(int,int)), this, SLOT(action_clicked(int,int)));

			chgpb[i][4] = new pbex(background[i]);
			chgpb[i][4]->setIcon(QIcon(":/images/action5.PNG"));
			chgpb[i][4]->setIconSize(QSize(46, 46));
			chgpb[i][4]->setnum(i, 4);
			//chgpb[i][2]->setMask(QBitmap(":/images/auto_mask.PNG"));
			chgpb[i][4]->setGeometry(QRect(87, 378, 46, 46));
			connect(chgpb[i][4], SIGNAL(buttonClicked(int,int)), this, SLOT(action_clicked(int,int)));

			//��ͷ���
			nobkgrd[i] = new QPixmap(":/images/nobkgrd.PNG");
			QPainter painter;
			painter.begin(nobkgrd[i]); // paint in picture
			painter.setBrush(Qt::NoBrush);
			QFont f;
			f.setPointSize(20);
			painter.setFont(f);
			//�����2λ���ı��
			if ((i+1) >= 10 )
			{
				painter.drawText(10,32,QString::number(i+1));
			}
			else
			{
				painter.drawText(18,32,QString::number(i+1));
			}
			painter.end();


			PrintNumLb[i] = new QLabel(background[i]);
			PrintNumLb[i]->setPixmap(*(nobkgrd[i]));
			PrintNumLb[i]->setStyleSheet("font: bold 50px;");
			PrintNumLb[i]->setGeometry(QRect(190, 8, 50, 47));

			Frameset[i][1] = new QFrame(tab[i]);
			Frameset[i][1]->setGeometry(QRect(540, 20, 435, 500));
			Frameset[i][1]->setFrameShadow(QFrame::Plain);
			Frameset[i][1]->setFrameShape(QFrame::Box);
			Frameset[i][1]->setLineWidth(4);

			//��ʾ������λ��
			xp = 10;
			yp = 20;

			labels[i][0] = new QLabel(Frameset[i][1]);
			labels[i][0]->setText("��ͷ״̬��");
			labels[i][0]->setGeometry(QRect(xp, yp, 80, 30));
			labels[i][0]->setStyleSheet(unchoosestyle);

			myle[i][0] = new le(Frameset[i][1]);
			switch (UIprintstatus[i])
			{
			case 1:
				myle[i][0]->setText("����");
				PrintStatus[i][0] = new QLabel(Frameset[i][1]);
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/xiumian.PNG"));
				PrintStatus[i][0]->setGeometry(QRect(xp+150, yp, 86, 80));

				break;

			case 2:
				myle[i][0]->setText("����");
				PrintStatus[i][0] = new QLabel(Frameset[i][1]);
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/shezhi.PNG"));
				PrintStatus[i][0]->setGeometry(QRect(xp+150, yp, 86, 80));

				break;
			case 3:
				myle[i][0]->setText("�ֶ�");
				PrintStatus[i][0] = new QLabel(Frameset[i][1]);
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/shoudong.PNG"));
				PrintStatus[i][0]->setGeometry(QRect(xp+150, yp, 86, 80));
			break;
			case 4:
				myle[i][0]->setText("׼��");
				PrintStatus[i][0] = new QLabel(Frameset[i][1]);
			    PrintStatus[i][0]->setPixmap(QPixmap(":/images/zhunbei.PNG"));
			    PrintStatus[i][0]->setGeometry(QRect(xp+150, yp, 86, 80));
				break;
			case 5:
				myle[i][0]->setText("ӡ��");
				PrintStatus[i][0] = new QLabel(Frameset[i][1]);
		        PrintStatus[i][0]->setPixmap(QPixmap(":/images/yunxing.PNG"));
		        PrintStatus[i][0]->setGeometry(QRect(xp+150, yp, 86, 80));
				break;
			case 7:
				myle[i][0]->setText("��ͣ");
				PrintStatus[i][0] = new QLabel(Frameset[i][1]);
		        PrintStatus[i][0]->setPixmap(QPixmap(":/images/jiting.PNG"));
		        PrintStatus[i][0]->setGeometry(QRect(xp+150, yp, 86, 80));
				break;
			case 8:
				myle[i][0]->setText("����");
				PrintStatus[i][0] = new QLabel(Frameset[i][1]);
		        PrintStatus[i][0]->setPixmap(QPixmap(":/images/guzhang.PNG"));
		        PrintStatus[i][0]->setGeometry(QRect(xp+150, yp, 86, 80));
				break;
			}

			myle[i][0]->setGeometry(QRect(xp + 230, yp+10, 100, 30));
			myle[i][0]->setReadOnly(true);
			myle[i][0]->setStyleSheet("background: lightgray");

			yp += 100;
			labels[i][1] = new QLabel(Frameset[i][1]);
			labels[i][1]->setText("��ǰ��ͷIO״̬��");
			labels[i][1]->setGeometry(QRect(xp, yp, 180, 30));
			labels[i][1]->setStyleSheet(unchoosestyle);

			yp += 35;
			myle[i][1] = new le(Frameset[i][1]);
			if (UIlockreleasestatus[i] == 0)
			{
				PrintStatus[i][1] = new QLabel(Frameset[i][1]);
				PrintStatus[i][1]->setPixmap(QPixmap(":/images/songwang.PNG"));
				PrintStatus[i][1]->setGeometry(QRect(xp+150, yp, 46, 29));

				myle[i][1]->setText("����");
			}
			else if (UIlockreleasestatus[i] == 1)
			{
				PrintStatus[i][1] = new QLabel(Frameset[i][1]);
				PrintStatus[i][1]->setPixmap(QPixmap(":/images/zhangjin.PNG"));
				PrintStatus[i][1]->setGeometry(QRect(xp+150, yp, 46, 29));

				myle[i][1]->setText("����");
			}
			myle[i][1]->setGeometry(QRect(xp + 230, yp, 100, 30));
			myle[i][1]->setReadOnly(true);
			myle[i][1]->setStyleSheet("background: lightgray");

			yp += 65;
			myle[i][2] = new le(Frameset[i][1]);
			if (UIupdownstatus[i] == 1)
			{
				PrintStatus[i][2] = new QLabel(Frameset[i][1]);
				PrintStatus[i][2]->setPixmap(QPixmap(":/images/taiqi.PNG"));
				PrintStatus[i][2]->setGeometry(QRect(xp+150, yp, 46, 29));

				myle[i][2]->setText("̧��");
			}
			else if (UIupdownstatus[i] == 0)
			{
				PrintStatus[i][2] = new QLabel(Frameset[i][1]);
				PrintStatus[i][2]->setPixmap(QPixmap(":/images/fangxia.PNG"));
				PrintStatus[i][2]->setGeometry(QRect(xp+150, yp, 46, 29));

				myle[i][2]->setText("����");
			}
			myle[i][2]->setGeometry(QRect(xp + 230, yp, 100, 30));
			myle[i][2]->setReadOnly(true);
			myle[i][2]->setStyleSheet("background: lightgray");
			yp += 55;
			myle[i][3] = new le(Frameset[i][1]);
			if (UIcolorpipestatus[i] == 1)
			{
				PrintStatus[i][3] = new QLabel(Frameset[i][1]);
				PrintStatus[i][3]->setPixmap(QPixmap(":/images/liaogansuojing.PNG"));
				PrintStatus[i][3]->setGeometry(QRect(xp+150, yp, 50, 45));

				myle[i][3]->setText("�ϸ�����");
			}
			else if (UIcolorpipestatus[i] == 0)
			{
				PrintStatus[i][3] = new QLabel(Frameset[i][1]);
				PrintStatus[i][3]->setPixmap(QPixmap(":/images/liaogansongxia.PNG"));
				PrintStatus[i][3]->setGeometry(QRect(xp+150, yp, 50, 45));

				myle[i][3]->setText("�ϸ�����");
			}
			myle[i][3]->setGeometry(QRect(xp + 230, yp+7, 100, 30));
			myle[i][3]->setReadOnly(true);
			myle[i][3]->setStyleSheet("background: lightgray");
			yp += 55;
			myle[i][4] = new le(Frameset[i][1]);
			if (UImagicstatus[i] == 0)
			{
				PrintStatus[i][4] = new QLabel(Frameset[i][1]);
				PrintStatus[i][4]->setPixmap(QPixmap(":/images/citaiguanbi.PNG"));
				PrintStatus[i][4]->setGeometry(QRect(xp+150, yp, 45, 64));

				myle[i][4]->setText("��̨�ر�");
			}
			else if (UImagicstatus[i] == 1)
			{
				PrintStatus[i][4] = new QLabel(Frameset[i][1]);
				PrintStatus[i][4]->setPixmap(QPixmap(":/images/citaijiaci.PNG"));
				PrintStatus[i][4]->setGeometry(QRect(xp+150, yp, 45, 64));

				myle[i][4]->setText("��̨�Ӵ�");
			}
			myle[i][4]->setGeometry(QRect(xp + 230, yp+17, 100, 30));
			myle[i][4]->setReadOnly(true);
			myle[i][4]->setStyleSheet("background: lightgray");
			yp += 70;
			myle[i][5] = new le(Frameset[i][1]);
			if (UIbumpstatus[i] == 0)
			{
				PrintStatus[i][5] = new QLabel(Frameset[i][1]);
				PrintStatus[i][5]->setPixmap(QPixmap(":/images/jiangbenting.PNG"));
				PrintStatus[i][5]->setGeometry(QRect(xp+150, yp, 53, 54));

				myle[i][5]->setText("����ͣ");
			}
			else if (UIbumpstatus[i] == 1)
			{
				PrintStatus[i][5] = new QLabel(Frameset[i][1]);
				PrintStatus[i][5]->setPixmap(QPixmap(":/images/jiangbenfanzhuan.PNG"));
				PrintStatus[i][5]->setGeometry(QRect(xp+150, yp, 53, 54));

				myle[i][5]->setText("������ת");
			}
			else if (UIbumpstatus[i] == 2)
			{
				PrintStatus[i][5] = new QLabel(Frameset[i][1]);
				PrintStatus[i][5]->setPixmap(QPixmap(":/images/jiangbenzhengzhuan.PNG"));
				PrintStatus[i][5]->setGeometry(QRect(xp+150, yp, 53, 54));

				myle[i][5]->setText("���÷�ת");
			}
			myle[i][5]->setGeometry(QRect(xp + 230, yp+12, 100, 30));
			myle[i][5]->setReadOnly(true);
			myle[i][5]->setStyleSheet("background: lightgray");

			xp = 480;
			yp = 550;

			labels[i][4] = new QLabel(tab[i]);
			labels[i][4]->setText("�߶ȣ�");
			labels[i][4]->setGeometry(QRect(xp, yp, 80, 30));
			labels[i][4]->setStyleSheet(unchoosestyle);

			myle[i][8] = new le(tab[i]);
			myle[i][8]->setText(QString::number(pubcom.zstatus[i]));
			myle[i][8]->setGeometry(QRect(xp + 100, yp, 60, 30));
			myle[i][8]->setReadOnly(true);
			myle[i][8]->setStyleSheet("background: lightgray");

			labels[i][6] = new QLabel(tab[i]);
			labels[i][6]->setText("������");
			labels[i][6]->setGeometry(QRect(xp, yp+50, 80, 30));
			labels[i][6]->setStyleSheet(unchoosestyle);

			myle[i][10] = new le(tab[i]);
			myle[i][10]->setText(QString::number(pubcom.magicpercent[i]));
			myle[i][10]->setGeometry(QRect(xp + 100, yp+50, 60, 30));
			myle[i][10]->setReadOnly(true);
			myle[i][10]->setStyleSheet("background: lightgray");

			
			labels[i][2] = new QLabel(tab[i]);
			labels[i][2]->setText("����λ�ã�");
			labels[i][2]->setGeometry(QRect(xp+200, yp, 80, 30));
			labels[i][2]->setStyleSheet(unchoosestyle);

			myle[i][6] = new le(tab[i]);
			myle[i][6]->setText(QString::number(pubcom.xstatus[i]));
			myle[i][6]->setGeometry(QRect(xp + 300, yp, 60, 30));
			myle[i][6]->setReadOnly(true);
			myle[i][6]->setStyleSheet("background: lightgray");

			/*
			chgpb[i][5] = new pbex(tab[i]);
			chgpb[i][5]->setText("����λ�û���");
			chgpb[i][5]->setnum(i, 5);
			chgpb[i][5]->setGeometry(QRect(xp + 370, yp, 120, 30));
			connect(chgpb[i][5], SIGNAL(buttonClicked(int,int)), this, SLOT(action_clicked(int,int)));
			*/

			labels[i][3] = new QLabel(tab[i]);
			labels[i][3]->setText("б��λ�ã�");
			labels[i][3]->setGeometry(QRect(xp+200, yp+50, 80, 30));
			labels[i][3]->setStyleSheet(unchoosestyle);

			myle[i][7] = new le(tab[i]);
			myle[i][7]->setText(QString::number(pubcom.ystatus[i]));
			myle[i][7]->setGeometry(QRect(xp + 300, yp+50, 60, 30));
			myle[i][7]->setReadOnly(true);
			myle[i][7]->setStyleSheet("background: lightgray");

			/*
			chgpb[i][6] = new pbex(tab[i]);
			chgpb[i][6]->setText("б��λ�û���");
			chgpb[i][6]->setnum(i, 6);
			chgpb[i][6]->setGeometry(QRect(xp + 370, yp+50, 120, 30));
			connect(chgpb[i][6], SIGNAL(buttonClicked(int,int)), this, SLOT(action_clicked(int,int)));
			
		
			backpb[i] = new QPushButton(tab[i]);
			backpb[i]->setText("����");
			backpb[i]->setGeometry(QRect(80,580,121,41));
			backpb[i]->setStyleSheet(
						"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
			connect(backpb[i], SIGNAL(clicked()), this, SLOT(backpb_clicked()));
			*/

			tabWidget->addTab(tab[i], QString("��ͷ") + QString::number(i + 1));
		}

		/**************************************IO��ʾ*****************************************/

		tab[pubcom.PrintNum] = new QWidget();
		tabWidget->addTab(tab[pubcom.PrintNum], "IO��ʾ");

		table[0] = new QTableWidget(tab[pubcom.PrintNum]);
		table[0]->setRowCount(16);
		table[0]->setColumnCount(6);
		table[0]->setGeometry(QRect(20, 20, 980, 580));
		table[0]->setRowHeight(2, 20);

		tablehead[0] = new QTableWidgetItem("״̬");
		table[0]->setHorizontalHeaderItem(0, tablehead[0]);

		tablehead[1] = new QTableWidgetItem("˵��");
		table[0]->setHorizontalHeaderItem(1, tablehead[1]);

		tablehead[2] = new QTableWidgetItem("״̬");
		table[0]->setHorizontalHeaderItem(2, tablehead[2]);

		tablehead[3] = new QTableWidgetItem("˵��");
		table[0]->setHorizontalHeaderItem(3, tablehead[3]);

		tablehead[4] = new QTableWidgetItem("״̬");
		table[0]->setHorizontalHeaderItem(4, tablehead[4]);

		tablehead[5] = new QTableWidgetItem("˵��");
		table[0]->setHorizontalHeaderItem(5, tablehead[5]);

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				if (i == 0)
				{
					switch(UIbumpstatus[j])
					{
					case 0x00:
						BumpIOLb[j] = new QLabel(this);
						BumpIOLb[j]->setPixmap(QPixmap(":/images/io00.PNG"));
						BumpIOLb[j]->setAlignment(Qt::AlignCenter);

                        table[0]->setCellWidget(j,i,BumpIOLb[j]);
						break;
					case 0x01:
						BumpIOLb[j] = new QLabel(this);
						BumpIOLb[j]->setPixmap(QPixmap(":/images/io01.PNG"));
						BumpIOLb[j]->setAlignment(Qt::AlignCenter);
						table[0]->setCellWidget(j,i,BumpIOLb[j]);
						break;
					case 0x02:
						BumpIOLb[j] = new QLabel(this);
						BumpIOLb[j]->setPixmap(QPixmap(":/images/io10.PNG"));
						BumpIOLb[j]->setAlignment(Qt::AlignCenter);
						table[0]->setCellWidget(j,i,BumpIOLb[j]);
						break;
					case 0x03:
						BumpIOLb[j] = new QLabel(this);
						BumpIOLb[j]->setPixmap(QPixmap(":/images/io11.PNG"));
						BumpIOLb[j]->setAlignment(Qt::AlignCenter);
						table[0]->setCellWidget(j,i,BumpIOLb[j]);
						break;
					}
				}
				else if( i == 2)
				{
					if(UImagicstatus[j] == 0)
					{
						MagicIOLb[j] = new QLabel(this);
						MagicIOLb[j]->setPixmap(QPixmap(":/images/io0.PNG"));
						MagicIOLb[j]->setAlignment(Qt::AlignCenter);
						table[0]->setCellWidget(j,i,MagicIOLb[j]);
					}
					else
					{
						MagicIOLb[j] = new QLabel(this);
						MagicIOLb[j]->setPixmap(QPixmap(":/images/io1.PNG"));
						MagicIOLb[j]->setAlignment(Qt::AlignCenter);
						table[0]->setCellWidget(j,i,MagicIOLb[j]);
					}
				}
				else
				{
					if (i == 1)
					{
						tableitem[j][i] = new QTableWidgetItem();

						tableitem[j][i]->setText(QString("�������IO"
								+ QString::number(j)));
						tableitem[j][i]->setTextAlignment(Qt::AlignCenter);
						tableitem[j][i]->setBackgroundColor(Qt::lightGray);
						table[0]->setItem(j, i, tableitem[j][i]);

					}
					else if (i == 3)
					{
						tableitem[j][i] = new QTableWidgetItem();

						tableitem[j][i]->setText(QString("��̨���IO"
								+ QString::number(j)));
						tableitem[j][i]->setTextAlignment(Qt::AlignCenter);
						tableitem[j][i]->setBackgroundColor(Qt::lightGray);
						table[0]->setItem(j, i, tableitem[j][i]);


					}
					else if (i == 5)
					{
						tableitem[j][i] = new QTableWidgetItem();

						tableitem[j][i]->setText(QString("��̨ģ����"
								+ QString::number(j)));
						tableitem[j][i]->setTextAlignment(Qt::AlignCenter);
						tableitem[j][i]->setBackgroundColor(Qt::lightGray);
						table[0]->setItem(j, i, tableitem[j][i]);

					}
					else if (i == 4)
					{
						MagicPercentLb[j] = new QLabel(this);
						MagicPercentLb[j]->setNum(pubcom.magicpercent[j]);
						MagicPercentLb[j]->setAlignment(Qt::AlignCenter);
					    table[0]->setCellWidget(j,i,MagicPercentLb[j]);
					}
				}
			}
		}

		table[0]->setColumnWidth(0, 40);
		table[0]->setColumnWidth(1, 275);
		table[0]->setColumnWidth(2, 40);
		table[0]->setColumnWidth(3, 275);
		table[0]->setColumnWidth(4, 40);
		table[0]->setColumnWidth(5, 270);

		/*
		backpb[pubcom.PrintNum] = new QPushButton(tab[pubcom.PrintNum]);
		backpb[pubcom.PrintNum]->setText("����");
		backpb[pubcom.PrintNum]->setGeometry(QRect(30,600,121,41));
		backpb[pubcom.PrintNum]->setStyleSheet(
			"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
		connect(backpb[pubcom.PrintNum], SIGNAL(clicked()), this, SLOT(backpb_clicked()));
		*/

	}


	//��ʱ�������Ź���
	timer->start(100); //100ms��ʱ


}

//��ʱ���������
void opprint::timeupdate()
{
	if (pubcom.CurrentWnd == 1)
	{
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
	}


	//��ʱ������ˢ��
	for (int i = 0; i< 16; ++i)
	{
		for (int j = 0; j < 6; ++j)
		{

			if (UIStatusChgTimer[i][j] > 0 )
			{
				UIStatusChgTimer[i][j] --;

				//�ı䱳��ɫ
				if (UIStatusChgTimer[i][j] == 1)
				{
					myle[i][j]->setStyleSheet("background: lightgray");
				}
			}
		}
	}

	for (int i = 0; i < pubcom.PrintNum; i++)
	{
		if(pubcom.nodeonlinemap.value(i) == false)
		{
#ifdef TEST
#else
				tab[i]->setDisabled(true);
#endif
		}
		else
		{
#ifdef TEST
#else
				tab[i]->setDisabled(false);
#endif
		}

		if (pubcom.printstatus[i] != UIprintstatus[i])
		{
			//4*100ms
			UIStatusChgTimer[i][0] = 4;
			UIprintstatus[i] = pubcom.printstatus[i];

			switch (UIprintstatus[i])
			{
			case 1:
				myle[i][0]->setText("����");
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/xiumian.PNG"));
				break;

			case 2:
				myle[i][0]->setText("����");
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/shezhi.PNG"));

				break;
			case 3:
				myle[i][0]->setText("�ֶ�");
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/shoudong.PNG"));
				break;
			case 4:
				myle[i][0]->setText("׼��");
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/zhunbei.PNG"));
				break;
			case 5:
				myle[i][0]->setText("ӡ��");
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/yunxing.PNG"));
				break;
			case 7:
				myle[i][0]->setText("��ͣ");
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/jiting.PNG"));
				break;
			case 8:
				myle[i][0]->setText("����");
				PrintStatus[i][0]->setPixmap(QPixmap(":/images/guzhang.PNG"));
				break;
			}
			myle[i][0]->setStyleSheet("background: yellow");

		}


		if (UIlockreleasestatus[i] != pubcom.lockreleasestatus[i])
		{
			//4*100ms
			UIStatusChgTimer[i][1] = 4;
			UIlockreleasestatus[i] = pubcom.lockreleasestatus[i];

			switch(UIlockreleasestatus[i]) {
			case 0:
				myle[i][1]->setText("����");
				PrintStatus[i][1]->setPixmap(QPixmap(":/images/songwang.PNG"));

				break;
			case 1:
				myle[i][1]->setText("����");
				PrintStatus[i][1]->setPixmap(QPixmap(":/images/zhangjin.PNG"));
				break;
			}
			myle[i][1]->setStyleSheet("background: yellow");

		}

		if (pubcom.updownstatus[i] != UIupdownstatus[i])
		{
			//4*100ms
			UIStatusChgTimer[i][2] = 4;
			UIupdownstatus[i] = pubcom.updownstatus[i];

			switch(UIupdownstatus[i]) {
			case 0:
				myle[i][2]->setText("����");
				PrintStatus[i][2]->setPixmap(QPixmap(":/images/fangxia.PNG"));

				break;
			case 1:
				myle[i][2]->setText("̧��");
				PrintStatus[i][2]->setPixmap(QPixmap(":/images/taiqi.PNG"));

				break;
			}
			myle[i][2]->setStyleSheet("background: yellow");

		}

		if (pubcom.colorpipestatus[i] != UIcolorpipestatus[i])
		{
			//4*100ms
			UIStatusChgTimer[i][3] = 4;
			UIcolorpipestatus[i] = pubcom.colorpipestatus[i];

			switch(UIcolorpipestatus[i]) {
			case 1:
				myle[i][3]->setText("�ϸ�����");
				PrintStatus[i][3]->setPixmap(QPixmap(":/images/liaogansuojing.PNG"));

				break;
			case 0:
				myle[i][3]->setText("�ϸ�����");
				PrintStatus[i][3]->setPixmap(QPixmap(":/images/liaogansongxia.PNG"));

				break;
			}
			myle[i][3]->setStyleSheet("background: yellow");


		}

		if (pubcom.magicstatus[i] != UImagicstatus[i])
		{
			//4*100ms
			UIStatusChgTimer[i][4] = 4;
			UImagicstatus[i] = pubcom.magicstatus[i];

			switch(UImagicstatus[i]) {
			case 0:
				myle[i][4]->setText("��̨�ر�");
				PrintStatus[i][4]->setPixmap(QPixmap(":/images/citaiguanbi.PNG"));
				MagicIOLb[i]->setPixmap(QPixmap(":/images/io0.PNG"));

				break;
			case 1:
				myle[i][4]->setText("��̨�Ӵ�");
				PrintStatus[i][4]->setPixmap(QPixmap(":/images/citaijiaci.PNG"));
				MagicIOLb[i]->setPixmap(QPixmap(":/images/io1.PNG"));


				break;
			}

			myle[i][4]->setStyleSheet("background: yellow");


		}

		if (pubcom.bumpstatus[i] != UIbumpstatus[i])
		{
			//4*100ms
			UIStatusChgTimer[i][5] = 4;
			UIbumpstatus[i] = pubcom.bumpstatus[i];

			switch(UIbumpstatus[i]) {
			case 0:
				myle[i][5]->setText("����ͣ");
				PrintStatus[i][5]->setPixmap(QPixmap(":/images/jiangbenting.PNG"));
				BumpIOLb[i]->setPixmap(QPixmap(":/images/io00.PNG"));

				break;
			case 1:
				myle[i][5]->setText("������ת");
				PrintStatus[i][5]->setPixmap(QPixmap(":/images/jiangbenfanzhuan.PNG"));
				BumpIOLb[i]->setPixmap(QPixmap(":/images/io01.PNG"));

				break;
			case 2:
				myle[i][5]->setText("���÷�ת");
				PrintStatus[i][5]->setPixmap(QPixmap(":/images/jiangbenzhengzhuan.PNG"));
				BumpIOLb[i]->setPixmap(QPixmap(":/images/io10.PNG"));
				break;
			case 3:
				BumpIOLb[i]->setPixmap(QPixmap(":/images/io11.PNG"));
				break;
			}

			myle[i][5]->setStyleSheet("background: yellow");

		}

		if (pubcom.xstatus[i] != UIxstatus[i])
		{
			UIxstatus[i] = pubcom.xstatus[i];
			myle[i][6]->setText(QString::number(UIxstatus[i]));
		}

		if (pubcom.ystatus[i] != UIystatus[i])
		{
			UIystatus[i] = pubcom.ystatus[i];
			myle[i][7]->setText(QString::number(UIystatus[i]));
		}

		if (pubcom.zstatus[i] != UIzstatus[i])
		{
			UIzstatus[i] = pubcom.zstatus[i];
			myle[i][8]->setText(QString::number(UIzstatus[i]));
		}

		if (pubcom.magicpercent[i] != UImagicpercent[i])
		{
			UImagicpercent[i] = pubcom.magicpercent[i];
			myle[i][10]->setText(QString::number(UImagicpercent[i]));

			MagicPercentLb[i]->setNum(UImagicpercent[i]);
		}
	}

	int index = tabWidget->currentIndex();

	QPoint leftup;
	QPoint rightdown;

	//��ǰ���λ��
	QPoint curMouseLocation;
	curMouseLocation = QCursor::pos();

	if(index < pubcom.PrintNum && pubcom.nodeonlinemap.value(index) == true)
	{
		for (int i = 0; i < 5; i++)
		{
			leftup = chgpb[index][i]->mapToGlobal(QPoint(0,0));
			rightdown = chgpb[index][i]->mapToGlobal(QPoint(chgpb[index][i]->width(),chgpb[index][i]->height()));

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
				QString name = QString(":/images/action") + QString::number(i+1) + QString("_on.PNG");
				chgpb[index][i]->setIcon(QIcon(name));
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
				QString name = QString(":/images/action") + QString::number(i+1) + QString(".PNG");
				chgpb[index][i]->setIcon(QIcon(name));
			}
		}
	}

	OldMouseLocation = curMouseLocation;

}



void opprint::mousePressEvent(QMouseEvent * event)
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
		//����������÷�����ת�����´���
		setModal(false);
		Dlg.exec();

		pubcom.CurrentWnd = 1;

	}
	QDialog::mousePressEvent(event);
}


void opprint::action_clicked(int printno, int action)
{
	//action:
	//0 �ɽ��� 1���� 2̧������ 3�ϴ�ȥ�� 4��תֹͣ��ת 5������� 6 б�����

	if (inactionflag == false)
	{
		interfaceevent* ptempevent = new interfaceevent();
		switch (action)
		{

		//�š�����
		case 0:

			publiccaution.addevent("��ͷ����ҳ��","������","�û�����������ť",1);
     		ptempevent->cmd = 0x03;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = (pubcom.lockreleasestatus[printno] == 1) ? 0x02 : 0x01;//С�����֣�02�ɻ���01��
			ptempevent->data[1] = printno;//��ͷ���
			inactionflag = true;
			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);

			break;

		//����
		case 1:

			publiccaution.addevent("��ͷ����ҳ��","����","�û������㰴ť",1);
     		ptempevent->cmd = 0x03;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = REQZEROCMD;//��������
			ptempevent->data[1] = printno;//��ͷ���
			inactionflag = true;
			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);

			break;

			//̧������
		case 2:

			publiccaution.addevent("��ͷ����ҳ��","̧������","�û���̧��������ť",1);
			ptempevent->cmd = 0x03;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = (pubcom.updownstatus[printno] == 1) ? 0x04 : 0x03;//С�����֣��·Ż�����̧
			ptempevent->data[1] = printno;//��ͷ���
			inactionflag = true;
			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);

			break;

			//�ϡ�ȥ��
		case 3:

			//ptempevent->cmd = 0x03;//��ͷ��������
			//ptempevent->status = 0x00;//����״̬
			//ptempevent->data[0] = (pubcom.magicstatus[printno] == 1) ? 0x08 : 0x07;//С�����֣�ȥ�Ż����ϴ�
			//ptempevent->data[1] = printno;//��ͷ���
			//inactionflag = true;
			//����һ���¼��������߳�
			//QCoreApplication::postEvent(backendobject, ptempevent);

			break;

			//��ת
		case 4:
			publiccaution.addevent("��ͷ����ҳ��","��ת","�û�����ת��ť",1);
			ptempevent->cmd = 0x03;//��ͷ��������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = REQCIRCLECMD;//��ת
			ptempevent->data[1] = printno;//��ͷ���
			inactionflag = true;
			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);

			break;

			//�������
		case 5:
			{
				QMessageBox msgBox;
				msgBox.setText("������㣬��ȷ�ϲ���?");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
				QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
				tb1->setText("ȷ��");
				QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
				tb2->setText("ȡ��");

				int ret = msgBox.exec();

				if (ret == QMessageBox::Yes)
				{
					publiccaution.addevent("��ͷ����ҳ��","�������","�û���������㰴ť",1);
					ptempevent->cmd = 0x03;//��ͷ��������
					ptempevent->status = 0x00;//����״̬
					ptempevent->data[0] = 0x05;//����λ�û���
					ptempevent->data[1] = printno;//��ͷ���
					inactionflag = true;
					//����һ���¼��������߳�
					QCoreApplication::postEvent(backendobject, ptempevent);
				}
				break;

			}

			//б�����
		case 6:
			{
				publiccaution.addevent("��ͷ����ҳ��","б�����","�û���б����㰴ť",1);

				QMessageBox msgBox;
				msgBox.setText("б����㣬��ȷ�ϲ���?");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
				QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
				tb1->setText("ȷ��");
				QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
				tb2->setText("ȡ��");
				int ret = msgBox.exec();



				if (ret == QMessageBox::Yes)
				{
					ptempevent->cmd = 0x03;//��ͷ��������
					ptempevent->status = 0x00;//����״̬
					ptempevent->data[0] = 0x06;//б��λ�û���
					ptempevent->data[1] = printno;//��ͷ���
					inactionflag = true;
					//����һ���¼��������߳�
					QCoreApplication::postEvent(backendobject, ptempevent);
				}
				break;

			}
		}
	}
}


void opprint::customEvent(QEvent *e)
{
	if (e->type() == BACKEND_EVENT) //�õ��Ǻ�̨���¼�
	{
		backendevent* event = (backendevent*) e;

		//��ͷ�������ɹ�
		if (event->cmd == 0x03
				&& event->status == 0x01)
		{
				inactionflag = false;

		}
		//��ͷ������ʧ��
		else if (event->cmd == 0x03
				&& event->status == 0x02)
		{
			inactionflag = false;
			QMessageBox msgBox;
			msgBox.setText("����ִ��ʧ��");
			msgBox.setWindowTitle("ʧ��");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

		}
		//��ͷ��������ǰ����ִ��
		else if (event->cmd == 0x03
				&& event->status == 0x03)
		{
			inactionflag = false;
			QMessageBox msgBox;
			msgBox.setText("��ǰ������ִ�д˶���");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ʧ��");
			msgBox.exec();
		}
		//��ͷ�������¼�����
		else if(event->cmd == 0x08
				&& event->status == 0x00)
		{
			if(event->data[0] == 1)
			{
				tab[event->data[1]]->setEnabled(true);
			}
			else if(event->data[0] == 2)
			{
				tab[event->data[1]]->setEnabled(false);
			}
		}
	}
}




void opprint::on_pushButton_clicked()
{
	publiccaution.addevent("��ͷ����ҳ��","�˳���ͷ����ҳ��","�û��˳���ͷ����ҳ��",1);

	close();

}