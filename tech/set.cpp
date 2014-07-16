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
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <iostream>
#include <QMessageBox>
#include <QIcon>
#include <QDomNodeList>
#include <QTextStream>
#include <QDomNamedNodeMap>
#include <QFile>
#include <QTime>
#include <QList>
#include <QByteArray>
#include "keyboard2.h"
#include "set.h"
#include "tech.h"
#include "keyboard2.h"
#include "keyboard.h"
#include <QPainter>
#include <Windows.h>
#include <QDir>
#include <QFile>
#include <QApplication>
#include "tech.h"
#include "help.h"
#include "caution.h"
#include "chgpasswd.h"
#include "canif.h"
#include "CMotor.h"
#include "measurepulse.h"
#include "DB.h"
#include "pcfingerprint.h"
#include "safenet.h"
#include <QLabel>

QColor bgColorForName(const QString &name);
QColor fgColorForName(const QString &name);

extern QSettings settings;
extern QString choosestyle2;
extern QString unchoosestyle;
extern QDomDocument* doc;
extern char screemsaverflag;
extern long screemsavertime;
extern QPicture topbarpic;
extern com pubcom;

int GetParaRow(const QString& groupname);

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);

//����XML����ȡ��ĳ���е�һ������
const QDomNode GetPara(const QString& groupname, const QString tag,
		const QString& paraname);

//�¼����ݶ���
extern QObject* backendobject;
extern QObject* interfaceobject;
extern QObject* a4nobject;
extern QObject* measurepulseobject;



//A4N���
extern CKSMCA4 *ksmc;

QTableWidgetItem* FricationNodeItem;
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

//���ݿ�
extern DB mydb;

//���ܹ����
extern safenet safenetinst;

set::set(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);


	//��ͷѡ��
	for (int i = 1; i <= pubcom.PrintNum; i++)
	{
		chooseprintlist.append(1);
	}

	//��ʼ��
	for (int i = 1; i <= pubcom.PrintNum; i++)
	{
		iprintnumlist.append(i);
	}


	image2 = new QImage();
	image2->load(":/DSC_1422.PNG");

	f.setPointSize(30);
	f.setBold(true);

	ui.pushButton->setStyleSheet("QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_2->setStyleSheet("QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_3->setStyleSheet("QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_9->setStyleSheet("QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_10->setStyleSheet("QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	pubcom.applyflag = false;

	printactiveflag = false;

	topbar = new QLabel(this);
	topbar->setGeometry(QRect(0,0,1024,50));
	topbar->setPicture(topbarpic);

	datetimebar = new QLabel(topbar);
	datetimebar->setGeometry(QRect(790,10,231,31));
	datetimebar->setStyleSheet(datetimestyle);

	Cautiondisplay = new QLabel(topbar);
	Cautiondisplay->setGeometry(QRect(400,0,400,50));
	Cautiondisplay->setStyleSheet(cautionstyle);

	//������ʱ��
	timer = &mytimer;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));

}

set::~set()
{

}

//��̬TABҳ
void set::init(int modright)
{
	publiccaution.addevent("����ҳ��","����","�û���������ҳ��",1);

	pubcom.CurrentWnd = 2;

	tablist.append(new QTabWidget(this));
	tablist.last()->setGeometry(QRect(0, 60, 1024, 650));
	tablist.last()->setStyleSheet("font-size: 18px");

	QDomNode para;
	QDomNode n;
	QDomNode p;

	int tempi = 0;
	int tempj = 0;

	/**************************************���ղ�������************************************/

	//ȡģ��ID
	para = GetParaByName("modlist", "���ղ���");
	if (para.isNull())
	{
		publiccaution.addevent("�����ļ�����","Ϊ��","");
		return;
	}

	if ((modright >> (para.firstChildElement("id").text().toInt() - 1) & 0x01)
			== 0x01)
	{
		widgetlist.append(new QWidget());

		tablist.last()->addTab(widgetlist.last(), "���ղ���");

		tablist.append(new QTabWidget(widgetlist.last()));
		tablist.last()->setGeometry(QRect(10, 10, 1004, 600));
		tablist.last()->setStyleSheet("font-size: 18px");

		widgetlist.append(new QWidget());

		tablist.last()->addTab(widgetlist.last(), "���ղ���");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("prod"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 964, 540));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		tempi = 0;
		tempj = 0;
		n = doc->firstChild();

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellprod_entered(int, int)));

		//�ҵ�XML�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("xml"))
				{
					break;
				}
			}
			n = n.nextSibling();
		}

		n = n.firstChild();

		//�ҵ�prod�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("prod"))
				{
					break;
				}
			}
			//showitem(&n);
			n = n.nextSibling();
		}

		//�ҵ���һ��param�ڵ�
		n = n.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;

					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc") )
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapprodshow.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{

						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapprod.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}
				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 660);


	}

	/**************************************������������************************************/

	//ȡģ��ID
	para = GetParaByName("modlist", "��������");
	if (para.isNull())
	{
		publiccaution.addevent("�����ļ�����","Ϊ��","",0);
		return;
	}

	if ((modright >> (para.firstChildElement("id").text().toInt() - 1) & 0x01)
			== 0x01)
	{
		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "��������");

		tablist.append(new QTabWidget(widgetlist.last()));
		tablist.last()->setGeometry(QRect(10, 10, 1004, 600));
		tablist.last()->setStyleSheet("font-size: 18px");

		widgetlist.append(new QWidget());
		tablist.last()->addTab(widgetlist.last(), "��������");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("machsub1"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 964, 540));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellmachsub1_entered(int, int)));

		tempi = 0;
		tempj = 0;
		n = doc->firstChild();

		//�ҵ�XML�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("xml"))
				{
					break;
				}
			}
			n = n.nextSibling();
		}

		n = n.firstChild();

		//�ҵ�mach�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("mach"))
				{
					break;
				}
			}
			//showitem(&n);
			n = n.nextSibling();
		}

		QDomElement q;

		//���丸�ڵ�
		QDomNode qq = n;

		q = qq.firstChildElement("machsub1");

		//�ҵ���һ��param�ڵ�
		n = q.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;
					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapmachsub1show.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapmachsub1.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 680);

		widgetlist.append(new QWidget());
		tablist.last()->addTab(widgetlist.last(), "��ͷ����ƫ��");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("machsub2"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 964, 540));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellmachsub2_entered(int, int)));

		tempi = 0;
		tempj = 0;

		q = qq.firstChildElement("machsub2");

		//�ҵ���һ��param�ڵ�
		n = q.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;
					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapmachsub2show.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapmachsub2.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}
				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[2]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 660);

		widgetlist.append(new QWidget());
		tablist.last()->addTab(widgetlist.last(), "���ù������");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("machsub3"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 964, 540));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellmachsub3_entered(int, int)));

		tempi = 0;
		tempj = 0;

		q = qq.firstChildElement("machsub3");

		//�ҵ���һ��param�ڵ�
		n = q.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;
					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapmachsub3show.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapmachsub3.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[2]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 660);

		widgetlist.append(new QWidget());
		tablist.last()->addTab(widgetlist.last(), "���������");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("machsub4"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 964, 540));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellmachsub4_entered(int, int)));

		tempi = 0;
		tempj = 0;

		q = qq.firstChildElement("machsub4");

		//�ҵ���һ��param�ڵ�
		n = q.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;
					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapmachsub4show.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapmachsub4.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[2]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 660);

		widgetlist.append(new QWidget());
		tablist.last()->addTab(widgetlist.last(), "��̨�����");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("machsub5"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 964, 540));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellmachsub5_entered(int, int)));

		tempi = 0;
		tempj = 0;

		q = qq.firstChildElement("machsub5");

		//�ҵ���һ��param�ڵ�
		n = q.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;
					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapmachsub5show.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapmachsub5.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[2]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 660);

		widgetlist.append(new QWidget());
		tablist.last()->addTab(widgetlist.last(), "��̨��ѹ�����");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("machsub6"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 964, 540));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellmachsub6_entered(int, int)));

		tempi = 0;
		tempj = 0;

		q = qq.firstChildElement("machsub6");

		//�ҵ���һ��param�ڵ�
		n = q.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;

					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapmachsub6show.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapmachsub6.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[2]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 660);
	}

	/**************************************���Բ�������************************************/
	//ȡģ��ID
	para = GetParaByName("modlist", "���Բ���");
	if (para.isNull())
	{
		publiccaution.addevent("�����ļ�����","Ϊ��","",0);
		return;
	}

	if ((modright >> (para.firstChildElement("id").text().toInt() - 1) & 0x01)
			== 0x01)
	{
		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "���Բ���");

		tablelist.append(new QTableWidget(widgetlist.last()));
		tablelist.last()->setRowCount(GetParaRow("tune"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 980, 480));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(celltune_entered(int, int)));

		//Ħ��ϵ�����԰�ť
		MeasurePulsePerMeter = new QPushButton(widgetlist.last());
		MeasurePulsePerMeter->setGeometry(QRect(40, 520, 140, 40));
		MeasurePulsePerMeter->setText("Ħ��ϵ������");
		connect(MeasurePulsePerMeter, SIGNAL(clicked()), this, SLOT(MeasurePulsePerMeterDlg()));

		tempi = 0;
		tempj = 0;
		n = doc->firstChild();

		//�ҵ�XML�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("xml"))
				{
					break;
				}
			}
			n = n.nextSibling();
		}

		n = n.firstChild();

		//�ҵ�tune�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("tune"))
				{
					break;
				}
			}
			//showitem(&n);
			n = n.nextSibling();
		}

		//�ҵ���һ��param�ڵ�
		n = n.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;

					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemaptuneshow.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemaptune.insert(t, pp);
						
						if (NID.toElement().text() == QString("Ħ��ϵ��"))
						{
							pp.tableitem->setBackgroundColor(Qt::lightGray);
							FricationNodeItem = pp.tableitem;
						}
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[3]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 680);


	}
	/**************************************������������************************************/

	if (((modright >> 7) & 0x01) == 0x01)
	{

		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "��������");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("dev"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 980, 550));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(celldev_entered(int, int)));

		tempi = 0;
		tempj = 0;
		n = doc->firstChild();

		//�ҵ�XML�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("xml"))
				{
					break;
				}
			}
			n = n.nextSibling();
		}

		n = n.firstChild();

		//�ҵ�dev�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("dev"))
				{
					break;
				}
			}
			//showitem(&n);
			n = n.nextSibling();
		}

		//�ҵ���һ��param�ڵ�
		n = n.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
						== QString("name") || p.toElement().tagName()
						== QString("value") || p.toElement().tagName()
						== QString("desc")|| p.toElement().tagName()
						== QString("id")|| p.toElement().tagName()
						== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;

					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
							|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapdevshow.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapdev.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[4]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 680);


		btcleardbginfo = new QPushButton(widgetlist.last());
		btcleardbginfo->setGeometry(QRect(40, 575, 140, 40));
		btcleardbginfo->setText("������Լ�¼");
		connect(btcleardbginfo, SIGNAL(clicked()), this, SLOT(btcleardbginfo_clicked()));

		clearlog = new QPushButton(widgetlist.last());
		clearlog->setGeometry(QRect(200, 575, 140, 40));
		clearlog->setText("�����־");
		connect(clearlog, SIGNAL(clicked()), this, SLOT(clearlog_clicked()));

		//����������
		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "������");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("driver"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 980, 550));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(celldriver_entered(int, int)));

		tempi = 0;
		tempj = 0;
		n = doc->firstChild();

		//�ҵ�XML�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("xml"))
				{
					break;
				}
			}
			n = n.nextSibling();
		}

		n = n.firstChild();

		//�ҵ�driver
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("driver"))
				{
					break;
				}
			}
			//showitem(&n);
			n = n.nextSibling();
		}

		//�ҵ���һ��param�ڵ�
		n = n.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
					== QString("name") || p.toElement().tagName()
					== QString("value") || p.toElement().tagName()
					== QString("desc")|| p.toElement().tagName()
					== QString("id")|| p.toElement().tagName()
					== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;

					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
						|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapdrivershow.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapdriver.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[4]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 680);

		//mdobus����
		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "MODBUS");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("modbus"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 980, 550));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellmodbus_entered(int, int)));

		tempi = 0;
		tempj = 0;
		n = doc->firstChild();

		//�ҵ�XML�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("xml"))
				{
					break;
				}
			}
			n = n.nextSibling();
		}

		n = n.firstChild();

		//�ҵ�modbus�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("modbus"))
				{
					break;
				}
			}
			//showitem(&n);
			n = n.nextSibling();
		}

		//�ҵ���һ��param�ڵ�
		n = n.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
					== QString("name") || p.toElement().tagName()
					== QString("value") || p.toElement().tagName()
					== QString("desc")|| p.toElement().tagName()
					== QString("id")|| p.toElement().tagName()
					== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;

					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
						|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapmodbusshow.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapmodbus.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[4]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 680);


		//PLC�������IOλ
		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "PLCIO");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("plcio"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 980, 550));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellplcio_entered(int, int)));

		tempi = 0;
		tempj = 0;
		n = doc->firstChild();

		//�ҵ�XML�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("xml"))
				{
					break;
				}
			}
			n = n.nextSibling();
		}

		n = n.firstChild();

		//�ҵ�plcio�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("plcio"))
				{
					break;
				}
			}
			//showitem(&n);
			n = n.nextSibling();
		}

		//�ҵ���һ��param�ڵ�
		n = n.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
					== QString("name") || p.toElement().tagName()
					== QString("value") || p.toElement().tagName()
					== QString("desc")|| p.toElement().tagName()
					== QString("id")|| p.toElement().tagName()
					== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;

					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
						|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapplcioshow.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapplcio.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[4]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 680);



		//PLCPARA����
		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "PLCPARA");

		tablelist.append(new QTableWidget(widgetlist.last()));

		tablelist.last()->setRowCount(GetParaRow("plcpara"));
		tablelist.last()->setColumnCount(3);
		tablelist.last()->setGeometry(QRect(20, 20, 980, 550));

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(0, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("����"));
		tablelist.last()->setHorizontalHeaderItem(1, tableheadlist.last());

		tableheadlist.append(new QTableWidgetItem("˵��"));
		tablelist.last()->setHorizontalHeaderItem(2, tableheadlist.last());

		connect(tablelist.last(), SIGNAL(cellDoubleClicked ( int, int)), this, SLOT(cellplcpara_entered(int, int)));

		tempi = 0;
		tempj = 0;
		n = doc->firstChild();

		//�ҵ�XML�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("xml"))
				{
					break;
				}
			}
			n = n.nextSibling();
		}

		n = n.firstChild();

		//�ҵ�plcpara�ڵ�
		while (!n.isNull())
		{
			if (n.isElement())
			{
				if (n.toElement().tagName() == QString("plcpara"))
				{
					break;
				}
			}
			//showitem(&n);
			n = n.nextSibling();
		}

		//�ҵ���һ��param�ڵ�
		n = n.firstChild();
		//�������е�param�ڵ�
		while (!n.isNull())
		{
			p = n.firstChild();
			tempi = 0;
			QDomNode NID;
			QDomNode NVAL;
			//�����ڲ�����
			while (!p.isNull())
			{
				if (p.isElement() && (p.toElement().tagName()
					== QString("name") || p.toElement().tagName()
					== QString("value") || p.toElement().tagName()
					== QString("desc")|| p.toElement().tagName()
					== QString("id")|| p.toElement().tagName()
					== QString("flag")))
				{
					tablekey t;
					t.x = tempi;
					t.y = tempj;

					//��ͨ�ĵ�Ԫ��
					if(p.toElement().tagName() == QString("name")
						|| p.toElement().tagName() == QString("desc"))
					{
						QTableWidgetItem* tableitem = new QTableWidgetItem(p.toElement().text());
						tablemapplcparashow.insert(t,tableitem);
						tableitem->setBackgroundColor(Qt::lightGray);
						tableitem->setTextAlignment(Qt::AlignCenter);
						tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, tempi, tableitem);
						tempi++;
					}
					else if(p.toElement().tagName() == QString("value"))//�����ĵ�Ԫ��
					{
						NVAL = p;
						tempi++;
					}
					else if(p.toElement().tagName() == QString("id"))
					{
						NID = p;
					}
					else if(p.toElement().tagName() == QString("flag"))
					{
						t.x = 1;
						tablenode pp;
						pp.id = NID;
						pp.node = NVAL;
						pp.flag = p;
						pp.tableitem = new QTableWidgetItem(NVAL.toElement().text());
						tablemapplcpara.insert(t, pp);
						pp.tableitem->setTextAlignment(Qt::AlignCenter);
						pp.tableitem->setFlags((bool)!Qt::ItemIsSelectable);
						tablelist.last()->setItem(tempj, 1, pp.tableitem);
					}

				}
				p = p.nextSibling();
			}
			n = n.nextSibling();
			tempj++;
		}

		//table[4]->resizeColumnsToContents();
		tablelist.last()->setColumnWidth(0, 200);
		tablelist.last()->setColumnWidth(1, 50);
		tablelist.last()->setColumnWidth(2, 680);
	}

	/**************************************Ȩ�޹���************************************/

	//�����Ȩ���ڴ�����
	passright one;
	one.flag = 0;
	one.pass = settings.value("prod/pass").toString();
	one.right = settings.value("prod/modright").toString();
	passrightlist.append(one);
	one.pass = settings.value("mach/pass").toString();
	one.right = settings.value("mach/modright").toString();
	passrightlist.append(one);
	one.pass = settings.value("tune/pass").toString();
	one.right = settings.value("tune/modright").toString();
	passrightlist.append(one);

	//ȡģ��ID
	para = GetParaByName("modlist", "Ȩ�޹���");
	if (para.isNull())
	{
		publiccaution.addevent("�����ļ�����","Ϊ��","",0);
		return;
	}

	if ((modright >> (para.firstChildElement("id").text().toInt() - 1) & 0x01)
			== 0x01)
	{
		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "Ȩ�޹���");

		{
			int beginx = 25;
			int xp = beginx;
			int yp = 50;

			righthead[0] = new QLabel(widgetlist.last());
			righthead[0]->setGeometry(QRect(xp, yp, 181, 51));
			righthead[0]->setText("�û�");
			righthead[0]->setStyleSheet(unchoosestyle);

			xp += 120;
			righthead[1] = new QLabel(widgetlist.last());
			righthead[1]->setGeometry(QRect(xp, yp, 181, 51));
			righthead[1]->setText("��������");
			righthead[1]->setStyleSheet(unchoosestyle);

			xp += 200;
			righthead[2] = new QLabel(widgetlist.last());
			righthead[2]->setGeometry(QRect(xp, yp, 181, 51));
			righthead[2]->setText("ģ��Ȩ��ѡ��");
			righthead[2]->setStyleSheet(unchoosestyle);

			xp = beginx;
			yp = 50;
			yp += 50;
			rightperson[0] = new QLabel(widgetlist.last());
			rightperson[0]->setGeometry(QRect(xp, yp, 181, 51));
			rightperson[0]->setText("����");
			rightperson[0]->setStyleSheet(unchoosestyle);

			yp += 50;
			rightperson[1] = new QLabel(widgetlist.last());
			rightperson[1]->setGeometry(QRect(xp, yp, 181, 51));
			rightperson[1]->setText("����");
			rightperson[1]->setStyleSheet(unchoosestyle);

			yp += 50;
			rightperson[2] = new QLabel(widgetlist.last());
			rightperson[2]->setGeometry(QRect(xp, yp, 181, 51));
			rightperson[2]->setText("����");
			rightperson[2]->setStyleSheet(unchoosestyle);

			xp = beginx;
			yp = 50;
			xp += 120;

			yp += 50;
			rightchgpasswd[0] = new pb(widgetlist.last());
			rightchgpasswd[0]->setnum(0);
			rightchgpasswd[0]->setGeometry(QRect(xp, yp + 10, 140, 40));
			rightchgpasswd[0]->setText("������(1XXXXX)");
			connect(rightchgpasswd[0], SIGNAL(buttonClicked(int)), this, SLOT(rightchgpasswd_clicked(int)));

			yp += 50;
			rightchgpasswd[1] = new pb(widgetlist.last());
			rightchgpasswd[1]->setnum(1);
			rightchgpasswd[1]->setGeometry(QRect(xp, yp + 10, 140, 40));
			rightchgpasswd[1]->setText("������(2XXXXX)");
			connect(rightchgpasswd[1], SIGNAL(buttonClicked(int)), this, SLOT(rightchgpasswd_clicked(int)));

			yp += 50;
			rightchgpasswd[2] = new pb(widgetlist.last());
			rightchgpasswd[2]->setnum(2);
			rightchgpasswd[2]->setGeometry(QRect(xp, yp + 10, 140, 40));
			rightchgpasswd[2]->setText("������(3XXXXX)");
			connect(rightchgpasswd[2], SIGNAL(buttonClicked(int)), this, SLOT(rightchgpasswd_clicked(int)));

			xp = beginx;
			yp = 50;
			xp += 320;
			yp += 50;
			int usernum = GetParaRow("defaultright");

			for (int i = 0; i < usernum; i++)
			{
				int modnum = GetParaRow("modbutton");
				for (int j = 0; j < modnum; j++)
				{
					QDomNode node = GetPara("modbutton", "id", QString::number(
							j + 1));
					rightchoose[i][j] = new pbex(widgetlist.last());
					rightchoose[i][j]->setnum(j, i);
					rightchoose[i][j]->setGeometry(QRect(xp + j * 95, yp + 10
							+ i * 50, 90, 40));
					rightchoose[i][j]->setText(
							node.firstChildElement("name").text());
					if (((passrightlist[i].right.toInt() >> j) & 0x01) == 0x01)
					{
						rightchoose[i][j]->setStyleSheet(choosestyle2);
					}
					connect(rightchoose[i][j], SIGNAL(buttonClicked(int,int)), this, SLOT(rightchgright_clicked(int,int)));
				}
			}
		}
	}

	/**************************************ϵͳ����************************************/
	//ȡģ��ID


	para = GetParaByName("modlist", "ϵͳ����");
	if (para.isNull())
	{
		publiccaution.addevent("�����ļ�����","Ϊ��","",0);
		return;
	}

	if ((modright >> (para.firstChildElement("id").text().toInt() - 1) & 0x01)
			== 0x01)
	{
		bool ok;
		int xp = 20;
		int yp = 20;

		ssscreemsaverflag = settings.value("screensaver/flag").toInt(&ok);
		ssscreemsavertime = settings.value("screensaver/time").toString();

		widgetlist.append(new QWidget());
		tablist[0]->addTab(widgetlist.last(), "ϵͳ����");

		sslabellist.append(new QLabel(widgetlist.last()));
		sslabellist.last()->setGeometry(QRect(xp, yp, 181, 51));
		sslabellist.last()->setText("��������");

		ssframelist.append(new QFrame(widgetlist.last()));
		ssframelist.last()->setGeometry(QRect(xp, yp + 50, 500, 80));
		ssframelist.last()->setFrameShape(QFrame::StyledPanel);
		ssframelist.last()->setFrameShadow(QFrame::Sunken);

		ssbtscreemsaver = new QPushButton(ssframelist.last());
		ssbtscreemsaver->setGeometry(QRect(20, 20, 75, 41));

		connect(ssbtscreemsaver, SIGNAL(clicked()), this, SLOT(swicthscreensaver_clicked()));

		sslabellist.append(new QLabel(ssframelist.last()));
		sslabellist.last()->setGeometry(QRect(180, 20, 90, 41));
		sslabellist.last()->setText("�ȴ�ʱ��:");
		//sslabellist.last()->setStyleSheet(unchoosestyle);

		ssminute = new le(ssframelist.last());
		ssminute->setGeometry(QRect(270, 20, 100, 41));
		ssminute->setText(ssscreemsavertime);
		connect(ssminute, SIGNAL(leClicked(int)), this, SLOT(screensaverle_clicked(int)));

		sslabellist.append(new QLabel(ssframelist.last()));
		sslabellist.last()->setGeometry(QRect(380, 20, 75, 41));
		sslabellist.last()->setText("����");

		if (ssscreemsaverflag == 1)
		{
			ssbtscreemsaver->setText("��");
		    ssbtscreemsaver->setStyleSheet(choosestyle2);
			ssminute->setEnabled(true);
		}
		else
		{
			ssbtscreemsaver->setText("��");
			ssminute->setEnabled(false);
		}

		sslabellist.append(new QLabel(widgetlist.last()));
		sslabellist.last()->setGeometry(QRect(20, 200, 120, 41));
		sslabellist.last()->setText("��������ʱ��");

		dte = new QDateTimeEdit(widgetlist.last());
		dte->setGeometry(QRect(20, 240, 600, 100));
		dte->setStyleSheet("font: 50px;");

		datetime = QDateTime::currentDateTime();
		dte->setTime(datetime.time());
		dte->setDate(datetime.date());

		backuplog = new QPushButton(widgetlist.last());
		backuplog->setGeometry(QRect(20, 400, 90, 30));
		backuplog->setText("������־");


		connect(dte, SIGNAL(dateTimeChanged ( const QDateTime)), this, SLOT(datetime_change(const QDateTime)));
		connect(timer, SIGNAL(timeout( )), this, SLOT(updatedatetime()));
		connect(backuplog, SIGNAL(clicked()), this, SLOT(backuplog_clicked()));

		sslabellist.append(new QLabel(widgetlist.last()));
		sslabellist.last()->setGeometry(QRect(xp+550, yp, 181, 51));
		sslabellist.last()->setText("��¼����");

		ssframelist.append(new QFrame(widgetlist.last()));
		ssframelist.last()->setGeometry(QRect(xp+550, yp + 50, 300, 80));
		ssframelist.last()->setFrameShape(QFrame::StyledPanel);
		ssframelist.last()->setFrameShadow(QFrame::Sunken);

		btoprec = new QPushButton(ssframelist.last());
		btoprec->setGeometry(QRect(20, 20, 75, 41));
		btoprec->setText("��¼����");
		if (pubcom.bloprecflag == true)
		{
			btoprec->setStyleSheet(choosestyle2);
		}
		else
		{
			btoprec->setStyleSheet(unchoosestyle);
		}

		btexceptrec = new QPushButton(ssframelist.last());
		btexceptrec->setGeometry(QRect(120, 20, 75, 41));
		btexceptrec->setText("��¼�쳣");
		if (pubcom.blexceptrecflag == true)
		{
			btexceptrec->setStyleSheet(choosestyle2);
	    }
		else
		{
			btexceptrec->setStyleSheet(unchoosestyle);
        }

		connect(btoprec, SIGNAL(clicked()), this, SLOT(btoprec_clicked()));
		connect(btexceptrec, SIGNAL(clicked()), this, SLOT(btexceptrec_clicked()));

	}

	//��ʱ�������ſ�ʼ����
	timer->start(100); //100ms��ʱ

	//�����ɫ�����޸����룺һ���̨�Ϳ�����̨
	if(role == 0)
	{
		ui.pushButton_9->hide();
	}

}

int set::showitem(QDomNode* p, QString groupname)
{
	int tempj = 0;

	tempj = 0;
	QDomNode n;

	if (p->hasChildNodes())
	{
		if (p->isElement())
		{
			if (p->toElement().tagName() == groupname)
			{
				//�ҵ���һ��param�ڵ�
				n = n.firstChild();
				//�������е�param�ڵ�
				while (!n.isNull())
				{
					n = n.nextSibling();
					tempj++;
				}
				return tempj;
			}
		}
		QDomNode t = p->firstChild();
		while (!t.isNull())
		{
			if ((tempj = showitem(&t, groupname)) > 0)
			{
				return tempj;
			}
			t = t.nextSibling();
		}
	}
}

//У�鼸����������������Ψһ��
bool set::VeryfyMachParaUniq()
{
	QList<int> temparray;
	QList<int>::iterator itt;

	QMap<tablekey,tablenode>::iterator it;

	bool ret = true;
	bool ok;

	//����ID
	//��ȡֵ
	for (it = tablemapmachsub3.begin(); it != tablemapmachsub3.end(); ++it)
	{
		temparray.append(it.value().node.toElement().text().toInt(&ok));
	}

	//�鿴Ψһ��
	for (itt = temparray.begin(); itt != temparray.end(); ++ itt)
	{
		if (temparray.count(*itt) > 1)
		{
			ret = false;

			QMessageBox msgBox;
			msgBox.setText("����ID��Ψһ���������޸�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			break;
		}
	}

	temparray.clear();

	//���������
	//��ȡֵ
	for (it = tablemapmachsub4.begin(); it != tablemapmachsub4.end(); ++it)
	{
		temparray.append(it.value().node.toElement().text().toInt(&ok));
	}

	//�鿴Ψһ��
	for (itt = temparray.begin(); itt != temparray.end(); ++ itt)
	{
		if (temparray.count(*itt) > 1)
		{
			ret = false;

			QMessageBox msgBox;
			msgBox.setText("��������㲻Ψһ���������޸�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			break;
		}
	}
	temparray.clear();

	//��̨�����
	//��ȡֵ
	for (it = tablemapmachsub5.begin(); it != tablemapmachsub5.end(); ++it)
	{
		temparray.append(it.value().node.toElement().text().toInt(&ok));
	}

	//�鿴Ψһ��
	for (itt = temparray.begin(); itt != temparray.end(); ++ itt)
	{
		if (temparray.count(*itt) > 1)
		{
			ret = false;
			QMessageBox msgBox;
			msgBox.setText("��̨����㲻Ψһ���������޸�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			break;
		}
	}
	temparray.clear();

	//��̨��ѹ�����
	//��ȡֵ
	for (it = tablemapmachsub6.begin(); it != tablemapmachsub6.end(); ++it)
	{
		temparray.append(it.value().node.toElement().text().toInt(&ok));
	}

	//�鿴Ψһ��
	for (itt = temparray.begin(); itt != temparray.end(); ++ itt)
	{
		if (temparray.count(*itt) > 1)
		{
			ret = false;
			QMessageBox msgBox;
			msgBox.setText("��̨��ѹ����㲻Ψһ���������޸�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			break;
		}
	}

	return ret;
}

//ȷ����ť��Ӧ�ò�����
void set::on_pushButton_clicked()
{
	//У�����������Ψһ
	if (VeryfyMachParaUniq() == false)
	{
		return;
	}

	QMessageBox msgBox;
	msgBox.setText("<font size = 4>��ȷ��ҪӦ�ò��������أ�</font>");
	msgBox.setWindowTitle("��ȷ��");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("��");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes)
	{
		publiccaution.addevent("����ҳ��","ȷ����ť��Ӧ�ò�����","�û��˳�����ҳ��",1);

		//������Ϣ����̨���к�̨Ӧ��
		interfaceevent* ptempevent = new interfaceevent();

		//ֻ�������±�ʶ���е�ʱ��Ŵ���ť
		if (pubcom.applyflag != true)
		{
			ptempevent->cmd = 0x05;//����ȷ������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x00;
			pubcom.applyflag = true;
			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}

}

//Ӧ�ð�ť
void set::on_pushButton_2_clicked()
{
	//У�����������Ψһ
	if (VeryfyMachParaUniq() == false)
	{
		return;
	}

	QMessageBox msgBox;
	msgBox.setText("<font size = 4>��ȷ��ҪӦ�ò�����</font>");
	msgBox.setWindowTitle("��ȷ��");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("��");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes)
	{
		publiccaution.addevent("����ҳ��","Ӧ�ð�ť","�û���Ӧ�ð�ť",1);

		//������Ϣ���к�̨Ӧ��
		interfaceevent* ptempevent = new interfaceevent();

		//ֻ�������±�ʶ���е�ʱ��Ŵ���ť
		if (pubcom.applyflag != true)
		{
			ptempevent->cmd = 0x04;//����Ӧ������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x00;
			pubcom.applyflag = true;
			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}
}

//ȡ����ť����Ӧ�ò�����
void set::on_pushButton_3_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("<font size = 4>��ȷ��Ҫȡ���ղ����õĲ��������أ�</font>");
	msgBox.setWindowTitle("��ȷ��");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("��");

	int ret = msgBox.exec();


	if (ret == QMessageBox::Yes)
	{
		publiccaution.addevent("����ҳ��","ȡ����ť����Ӧ�ò�����","�û��˳�����ҳ��",1);

		doc->clear();

		//����ˢ��DOC�ڴ�ӳ��
		QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
		if (!file.open(QIODevice::ReadWrite))
		{
			QMessageBox msgBox;
			msgBox.setText("�������ļ�ʧ�ܣ�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}

		if (!doc->setContent(&file))
		{
			file.close();
			return;
		}
		file.close();
		//ֱ�ӷ���
		close();
	}
}

//���ձ������
void set::cellprod_entered(int row, int column)
{

	if (column != 1)
	{
		return;
	}

	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapprod.value(t).node.toElement().text().toDouble(&ok);
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
		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;


		//����У��
		if (tablemapprod.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("prod",
					tablemapprod.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}


			//�����иı�
			tablemapprod.value(t).flag.firstChild().setNodeValue(QString::number(1));
			(tablemapprod.value(t).tableitem)->setText(keyboardinst.enter);
			tablemapprod.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);

		}

	}
}

//�������1����
void set::cellmachsub1_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}

	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapmachsub1.value(t).node.toElement().text().toDouble(&ok);
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
		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapmachsub1.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("machsub1",
					tablemapmachsub1.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}


			//�����иı�
			tablemapmachsub1.value(t).flag.firstChild().setNodeValue("1");

			(tablemapmachsub1.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapmachsub1.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);

		}
	}
}

//�������2����
void set::cellmachsub2_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}

	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapmachsub2.value(t).node.toElement().text().toDouble(&ok);
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
		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapmachsub2.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("machsub2",
					tablemapmachsub2.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}


			//�����иı�
			tablemapmachsub2.value(t).flag.firstChild().setNodeValue("1");

			(tablemapmachsub2.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapmachsub2.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);

		}
	}
}

//�������3����
void set::cellmachsub3_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}

	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapmachsub3.value(t).node.toElement().text().toDouble(&ok);
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
		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapmachsub3.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("machsub3",
					tablemapmachsub3.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}


			//�����иı�
			tablemapmachsub3.value(t).flag.firstChild().setNodeValue("1");

			(tablemapmachsub3.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapmachsub3.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);

		}
	}
}

//�������4����
void set::cellmachsub4_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}

	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapmachsub4.value(t).node.toElement().text().toDouble(&ok);
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
		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapmachsub4.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("machsub4",
					tablemapmachsub4.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}


			//�����иı�
			tablemapmachsub4.value(t).flag.firstChild().setNodeValue("1");

			(tablemapmachsub4.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapmachsub4.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);

		}
	}
}

//�������5����
void set::cellmachsub5_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}

	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapmachsub5.value(t).node.toElement().text().toDouble(&ok);
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

		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapmachsub5.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("machsub5",
					tablemapmachsub5.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}


			//�����иı�
			tablemapmachsub5.value(t).flag.firstChild().setNodeValue("1");

			(tablemapmachsub5.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapmachsub5.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);

		}
	}
}

//�������6����
void set::cellmachsub6_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}

	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapmachsub6.value(t).node.toElement().text().toDouble(&ok);
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
		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapmachsub6.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("machsub6",
					tablemapmachsub6.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

				return;
			}

			//�����иı�
			tablemapmachsub6.value(t).flag.firstChild().setNodeValue("1");

			(tablemapmachsub6.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapmachsub6.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);
		}
	}
}

//���Ա������
void set::celltune_entered(int row, int column)
{
	tablekey t;
	t.x = column;
	t.y = row;

	//����ѡ���ֵ��
	if ((column != 1)
		|| (tablemaptune.value(t).id.toElement().text() == QString("Ħ��ϵ��")) )
	{
		return;
	}

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemaptune.value(t).node.toElement().text().toDouble(&ok);
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
		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemaptune.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("tune",
					tablemaptune.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toDouble(&ok);
			max = para.firstChildElement("rangemax").text().toDouble(&ok);
			step = para.firstChildElement("valuestep").text().toDouble(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toDouble(&ok) > max
					|| keyboardinst.enter.toDouble(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}


			//�����иı�
			tablemaptune.value(t).flag.firstChild().setNodeValue("1");

			(tablemaptune.value(t).tableitem)->setText(keyboardinst.enter);

			tablemaptune.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);

		}
	}
}

//�����������
void set::celldev_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}
	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapdev.value(t).node.toElement().text().toDouble(&ok);
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

		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapdev.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("dev",
					tablemapdev.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}

			//�����иı�
			tablemapdev.value(t).flag.firstChild().setNodeValue("1");

			(tablemapdev.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapdev.value(t).node.firstChild().setNodeValue(
					keyboardinst.enter);
		}
	}
}


//��������������
void set::celldriver_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}
	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapdriver.value(t).node.toElement().text().toDouble(&ok);
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

		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapdriver.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("driver",
				tablemapdriver.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
				|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}


			//�����иı�
			tablemapdriver.value(t).flag.firstChild().setNodeValue("1");

			(tablemapdriver.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapdriver.value(t).node.firstChild().setNodeValue(
				keyboardinst.enter);
		}
	}
}

//PLCIO����
void set::cellplcio_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}
	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapplcio.value(t).node.toElement().text().toDouble(&ok);
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

		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapplcio.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("plcio",
				tablemapplcio.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
				|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}


			//�����иı�
			tablemapplcio.value(t).flag.firstChild().setNodeValue("1");

			(tablemapplcio.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapplcio.value(t).node.firstChild().setNodeValue(
				keyboardinst.enter);
		}
	}
}


//plcpara����
void set::cellplcpara_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}
	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapplcpara.value(t).node.toElement().text().toDouble(&ok);
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

		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapplcpara.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("plcpara",
				tablemapplcpara.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
				|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}


			//�����иı�
			tablemapplcpara.value(t).flag.firstChild().setNodeValue("1");

			(tablemapplcpara.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapplcpara.value(t).node.firstChild().setNodeValue(
				keyboardinst.enter);
		}
	}
}
//modbus����
void set::cellmodbus_entered(int row, int column)
{
	if (column != 1)
	{
		return;
	}
	tablekey t;
	t.x = column;
	t.y = row;

	keyboard2 keyboardinst;

	bool ok;
	double number;
	number = tablemapmodbus.value(t).node.toElement().text().toDouble(&ok);
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

		//����
		tablekey t2;
		t2.x = 0;
		t2.y = row;

		bool ok;
		float min, max, step;

		//����У��
		if (tablemapmodbus.value(t).tableitem->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByID("modbus",
				tablemapmodbus.value(t).id.toElement().text());

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
				|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}


			//�����иı�
			tablemapmodbus.value(t).flag.firstChild().setNodeValue("1");

			(tablemapmodbus.value(t).tableitem)->setText(keyboardinst.enter);

			tablemapmodbus.value(t).node.firstChild().setNodeValue(
				keyboardinst.enter);
		}
	}
}


//Ȩ�޹���ť�޸�����
void set::rightchgpasswd_clicked(int num)
{
	keyboard2 keyboardinst;
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//ȡ��canel
	if (keyboardinst.flag == 1)
	{
		;
	}//ȷ��
	else if (keyboardinst.flag == 2)
	{
		//У������������
		if( !keyboardinst.enter.startsWith(QString::number(num + 1)))
		{
			QMessageBox msgBox;
			msgBox.setText(QString("����Ӧ����") + QString::number(num + 1) + QString("��ͷ��"));
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}

		//У�����볤��
		if (keyboardinst.enter.length() != 6 )
		{
			QMessageBox msgBox;
			msgBox.setText("���볤��Ӧ��Ϊ6��");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}
		passrightlist[num].flag = 1;
		passrightlist[num].pass = keyboardinst.enter;
	}

	if (passrightlist[0].flag == 1)
	{
		passrightlist[0].flag = 0;
		settings.setValue("prod/pass", passrightlist[0].pass);
	}
	else if (passrightlist[1].flag == 1)
	{
		passrightlist[1].flag = 0;
		settings.setValue("mach/pass", passrightlist[1].pass);
	}
	else if (passrightlist[2].flag == 1)
	{
		passrightlist[2].flag = 0;
		settings.setValue("tune/pass", passrightlist[2].pass);
	}

	publiccaution.addevent("����ҳ��","�޸�����","�û��޸�����",1);
}

//Ȩ�޹���ѡ��Ȩ��
void set::rightchgright_clicked(int x, int y)
{
	if (((passrightlist[y].right.toInt() >> x) & 0x01) == 0x01)
	{
		passrightlist[y].right = QString::number(
				(passrightlist[y].right.toInt() & ~(0x01 << x)));
		passrightlist[y].flag = 1;
		rightchoose[y][x]->setStyleSheet(unchoosestyle);
	}
	else
	{
		passrightlist[y].right = QString::number(
				(passrightlist[y].right.toInt() | (0x01 << x)));
		passrightlist[y].flag = 1;
		rightchoose[y][x]->setStyleSheet(choosestyle2);
	}

	if (passrightlist[0].flag == 1)
	{
		passrightlist[0].flag = 0;
		settings.setValue("prod/modright", passrightlist[0].right);
	}
	else if (passrightlist[1].flag == 1)
	{
		passrightlist[1].flag = 0;
		settings.setValue("mach/modright", passrightlist[1].right);
	}
	else if (passrightlist[2].flag == 1)
	{
		passrightlist[1].flag = 0;
		settings.setValue("tune/modright", passrightlist[2].right);
	}

	publiccaution.addevent("����ҳ��","�޸��û�Ȩ��","�û��޸�Ȩ��",1);
}


//��ʱ���������
void set::timeupdate()
{
	if (	pubcom.CurrentWnd == 2)
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
			datetimebar->setText(QDateTime::currentDateTime().toString(
				"yyyy-MM-dd hh:mm:ss"));
		}
	}
}


void set::mousePressEvent(QMouseEvent * event)
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
		pubcom.CurrentWnd = 2;
	}
	QDialog::mousePressEvent(event);
}

//�޸�����
void set::on_pushButton_9_clicked()
{
	chgpasswd keyboardinst;
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.setRole(role);
	keyboardinst.exec();

	//ȡ��canel
	if (keyboardinst.flag == 1)
	{
		;
	}//ȷ��
	else if (keyboardinst.flag == 2)
	{
		QString CompareOldPass;

		switch (role)
		{
			case 1:
				CompareOldPass = settings.value("prod/pass").toString();
				break;

			case 2:
				CompareOldPass = settings.value("mach/pass").toString();
				break;

			case 3:
				CompareOldPass = settings.value("tune/pass").toString();
				break;
		}
		std::cout << keyboardinst.oldpassword.toStdString() << "," << CompareOldPass.toStdString() << std::endl;
		//������˶�
		if( keyboardinst.oldpassword != CompareOldPass)
		{
			
			QMessageBox msgBox;
			msgBox.setText(QString("���������"));
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}
		//����������˶�
		if(keyboardinst.newpassword != keyboardinst.newpassword2)
		{
			QMessageBox msgBox;
			msgBox.setText(QString("������1��������2����ͬ��"));
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}

		//У������������
		if( !keyboardinst.newpassword.startsWith(QString::number(role)))
		{
			QMessageBox msgBox;
			msgBox.setText(QString("����Ӧ����") + QString::number(role) + QString("��ͷ��"));
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}

		//У�����볤��
		if (keyboardinst.newpassword.length() != 6 )
		{
			QMessageBox msgBox;
			msgBox.setText("���볤��Ӧ��Ϊ6��");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}

		//���뱣��
		switch (role)
		{
			case 1:
				settings.setValue("prod/pass",keyboardinst.newpassword);
				break;

			case 2:
				settings.setValue("mach/pass",keyboardinst.newpassword);
				break;

			case 3:
				settings.setValue("tune/pass",keyboardinst.newpassword);
				break;
		}
	}
}

void set::SetUserRole(int role)
{
	set::role = role;
}


//ǿ��ˢ��
void set::on_pushButton_10_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("<font size = 4>��ȷ��Ҫǿ��ˢ�²�����</font>");
	msgBox.setWindowTitle("��ȷ��");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("��");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes)
	{
		publiccaution.addevent("����ҳ��","ǿ��ˢ��","�û�ǿ��ˢ�²���",1);

		//������Ϣ���к�̨Ӧ��
		interfaceevent* ptempevent = new interfaceevent();

		//ֻ�������±�ʶ���е�ʱ��Ŵ���ť
		if (pubcom.applyflag != true)
		{
			ptempevent->cmd = 0x08;//����Ӧ������
			ptempevent->status = 0x00;//����״̬
			ptempevent->data[0] = 0x00;
			pubcom.applyflag = true;
			//����һ���¼��������߳�
			QCoreApplication::postEvent(backendobject, ptempevent);
		}
	}
}

//��������
void set::swicthscreensaver_clicked()
{
	publiccaution.addevent("����ҳ��","��������","�û���������",1);

	if (ssscreemsaverflag == 1)
	{
		ssscreemsaverflag = 0;
		ssbtscreemsaver->setText("��");
		ssbtscreemsaver->setStyleSheet(unchoosestyle);
		ssminute->setEnabled(false);
	}
	else if (ssscreemsaverflag == 0)
	{
		ssscreemsaverflag = 1;
		ssbtscreemsaver->setText("��");
		ssbtscreemsaver->setStyleSheet(choosestyle2);
		ssminute->setEnabled(true);
	}

	screemsaverflag = ssscreemsaverflag;
	settings.setValue("screensaver/flag", ssscreemsaverflag);
}

void set::screensaverle_clicked(int num)
{
	keyboard2 keyboardinst;
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//ȡ��canel
	if (keyboardinst.flag == 1)
	{
		;
	}//ȷ��
	else if (keyboardinst.flag == 2)
	{

		bool ok;
		float min, max, step;

		//����У��
		if (ssminute->text() != keyboardinst.enter)
		{
			QDomNode para = GetParaByName("screensaver","�ȴ�ʱ��");

			min = para.firstChildElement("rangemin").text().toFloat(&ok);
			max = para.firstChildElement("rangemax").text().toFloat(&ok);
			step = para.firstChildElement("valuestep").text().toFloat(&ok);

			//У�������Ƿ񳬱�
			if (keyboardinst.enter.toFloat(&ok) > max
					|| keyboardinst.enter.toFloat(&ok) < min)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();				return;
			}

			int intstep = step * 10;
			int intenter = keyboardinst.enter.toFloat(&ok) * 10;

			//У�������Ƿ��ǿ��Ա�stepvalue����
			if (intenter % intstep != 0)
			{
				QMessageBox msgBox;
				msgBox.setText("�������ݾ��ȳ�����Χ��");
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();
				return;
			}

			//�����иı�
			ssminute->setText(keyboardinst.enter);
			ssscreemsavertime = keyboardinst.enter;
			screemsavertime = ssscreemsavertime.toInt()*60*10; //100msΪ��λ�ļ���
			settings.setValue("screensaver/time", ssscreemsavertime);

			publiccaution.addevent("����ҳ��","��������ʱ��","�û���������ʱ��",1);

		}
	}
}

//����ʱ��ı�
void set::datetime_change(const QDateTime & datetime )
{
	SYSTEMTIME currenttime;
	currenttime.wYear = datetime.date().year();
	currenttime.wMonth = datetime.date().month();
	currenttime.wDayOfWeek = datetime.date().dayOfWeek();
	currenttime.wDay = datetime.date().day();
	currenttime.wHour = datetime.time().hour()-8;
	currenttime.wMinute = datetime.time().minute();
	currenttime.wSecond = datetime.time().second();
	currenttime.wMilliseconds = datetime.time().msec();

	SetSystemTime(&currenttime);
}

//����ʱ�������
void set::updatedatetime()
{
	if(QDateTime::currentDateTime().time() != dte->time())
	{
	dte->setTime(QDateTime::currentDateTime().time());
	}

	if(QDateTime::currentDateTime().date() != dte->date())
	{
	dte->setDate(QDateTime::currentDateTime().date());
	}
}

void set::customEvent(QEvent *e)
{
	if (e->type() == BACKEND_EVENT) //�õ��Ǻ�̨���¼�
	{
		backendevent* event = (backendevent*) e;

		//��������
		if (event->cmd == 0x04
				&& event->status == 0x01)
		{
				pubcom.applyflag = false;
				//����XML�ļ�
				QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
				if (!file.open(QFile::WriteOnly))
				{
					QMessageBox msgBox;
					msgBox.setText("�������ļ�ʧ�ܣ�");
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
		else if (event->cmd == 0x04
				&& event->status == 0x02)
		{
			pubcom.applyflag = false;
				//����XML�ļ�
				QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
				if (!file.open(QFile::WriteOnly))
				{
					QMessageBox msgBox;
					msgBox.setText("�������ļ�ʧ�ܣ�");
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
				msgBox.setWindowTitle("���棡");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

		}
		//��������
		else if (event->cmd == 0x04
				&& event->status == 0x03)
		{
			pubcom.applyflag = false;
				//����XML�ļ�
				QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
				if (!file.open(QFile::WriteOnly))
				{
					QMessageBox msgBox;
					msgBox.setText("�������ļ�ʧ�ܣ�");
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
				msgBox.setWindowTitle("����");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("ȷ��");
				msgBox.exec();

		}
		//��������
		else if (event->cmd == 0x04
			&& event->status == 0x04)
		{
			pubcom.applyflag = false;
			//����XML�ļ�
			QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
			if (!file.open(QFile::WriteOnly))
			{
				QMessageBox msgBox;
				msgBox.setText("�������ļ�ʧ�ܣ�");
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
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

		}
		else
		{
			publiccaution.adddbginfo(QString("���ý�����ܵ�����ʶ�ĺ�̨�¼�"));
		}
	}
	else
	{
		publiccaution.adddbginfo(QString("���ý�����ܵ�����ʶ���¼�"));
	}
}

void set::MeasurePulsePerMeterDlg()
{
	//ֻ�����а��Ӷ������ڹ���״̬�ſ��Բ���Ħ��ϵ��
	for(int i =0; i < pubcom.PrintNum; i++)
	{
		if(pubcom.presetprintstatus[i] == 3)
		{
			QMessageBox msgBox;
			msgBox.setText("����Ħ��ϵ��ʱ��ͷ���ܴ��ڹ���״̬��");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes);
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
	}

	measurepulse measurepulseinst;
	measurepulseobject = &measurepulseinst;
	measurepulseinst.setWindowFlags(Qt::CustomizeWindowHint);
	measurepulseinst.exec();
}

void set::backuplog_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("��ȷ���Ƿ����Ҫ������־��");
	msgBox.setWindowTitle("������־��");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
	tb->setText("ȷ��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::Cancel);
	tb2->setText("ȡ��");

	QMessageBox msgBox2;
	msgBox2.setText("���ڱ��ݣ���ȴ���");
	msgBox2.setWindowTitle("������...");
	msgBox2.setStandardButtons(QMessageBox::NoButton);

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes )
	{
		publiccaution.addevent("����ҳ��","���ݵ�U��","�û����ݵ�U��",1);

		//�ж�U���Ƿ����
		QDomNode para;
		para = GetParaByName("dev", "u�����������");
		bool ok;
		int drivernum = para.firstChildElement("value").text().toInt(&ok);

		QString driver;
		switch(drivernum) {
	case 1:
		if (QDir("D:").exists() == false)
		{

			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("d:");

		break;
	case 2:
		if (QDir("E:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("e:");

		break;
	case 3:
		if (QDir("F:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("f:");

		break;

	case 4:
		if (QDir("G:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("g:");

		break;

	default:
		if (QDir("E:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("e:");
		}

		msgBox2.show();

		//ȡ�õ�ǰĿ¼
		QString currentpath = QApplication::applicationDirPath();

		//��U�����½�Ŀ¼
		QString newpath;
		newpath = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
		QDir newdir;
		newpath = driver + QString('\\' ) + newpath;
		newdir.mkpath(newpath);

		//�����ļ���U��
		if (QFile::copy(currentpath + QString("\\aaa.xml"),newpath + QString("\\aaa.xml"))
			&& QFile::copy(currentpath + QString("\\tech"),newpath + QString("\\tech")))
		{
			msgBox2.hide();
			QMessageBox msgBox;
			msgBox.setText("������ɣ�");
			msgBox.setWindowTitle("�ɹ�");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

		}
		else
		{
			msgBox2.hide();
			QMessageBox msgBox;
			msgBox.setText("����ʧ�ܣ�");
			msgBox.setWindowTitle("ʧ��");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
		}
	}

}

//���ݻ���ָ��
void set::backupfinger_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("��ȷ���Ƿ����Ҫ���ݻ������кţ�");
	msgBox.setWindowTitle("���ݻ������кţ�");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
	tb->setText("ȷ��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::Cancel);
	tb2->setText("ȡ��");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes )
	{
		publiccaution.addevent("����ҳ��","���ݻ������к�","���ݻ������кŵ�U��",1);

		//�ж�U���Ƿ����
		QDomNode para;
		para = GetParaByName("dev", "u�����������");
		bool ok;
		int drivernum = para.firstChildElement("value").text().toInt(&ok);

		QString driver;
		switch(drivernum) {
	case 1:
		if (QDir("D:").exists() == false)
		{

			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("d:");

		break;
	case 2:
		if (QDir("E:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("e:");

		break;
	case 3:
		if (QDir("F:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("f:");

		break;

	case 4:
		if (QDir("G:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("g:");

		break;

	default:
		if (QDir("E:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("e:");
		}

		//���ļ�
		QFile file(driver + QString("\\ipcserial"));

		if (!file.open(QIODevice::WriteOnly))
			return;

		//���������к�
		char fingerfrombios[40];
		memset(fingerfrombios,0,40);
		int fingerfrombioslen = 0;

		//ȡ����ʵ��ֵ
		pcfingerprint fingerprintinst;
		if (fingerprintinst.getpcfingerprint(fingerfrombios,&fingerfrombioslen) != 0)
		{
			QMessageBox msgBox;
			msgBox.setText("���������кŴ���");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}

		//���ļ�д����������
		file.write(fingerfrombios, fingerfrombioslen);
		file.close();

		msgBox.setText("������ɣ�");
		msgBox.setWindowTitle("�ɹ�");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();

		/*

		//���Դ��룬�����кţ��鿴��ʵ��ֵ�Ƚ�
		//���ļ�
		QFile file2(driver + QString("\\ipcserial"));

		if (!file2.open(QIODevice::ReadOnly))
			return;

		std::cout << "file2.size():" << file2.size() << std::endl;
		std::cout << "fingerfrombioslen:" << fingerfrombioslen << std::endl;

		char* fingerfromfile = new char[file2.size()];
		file2.read(fingerfromfile,file2.size());

		for(int i = 0; i< fingerfrombioslen; i++)
		{
			if (fingerfromfile[i] != fingerfrombios[i])
			{
				std::cout << "file error" << std::endl;
				std::cout << fingerfrombios[i] << "    " << fingerfromfile[i] << std::endl;
			}
		}
		*/

	}
}

//�������֤
void set::updatelicense_clicked()
{
	int year,month,day;
	safenetinst.GetDogDate(&year,&month,&day);
	QDate newdate(year,month,day);

	QMessageBox msgBox;
	msgBox.setText("��ȷ���Ƿ����Ҫ�����������֤������֤U���б����˳����ṩ�����֤�ļ���");
	msgBox.setWindowTitle("�������֤��");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
	tb->setText("ȷ��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::Cancel);
	tb2->setText("ȡ��");

	int ret = msgBox.exec();

	if (ret == QMessageBox::Yes )
	{
		publiccaution.addevent("����ҳ��","����","��U���������֤",1);

		//�ж�U���Ƿ����
		QDomNode para;
		para = GetParaByName("dev", "u�����������");
		bool ok;
		int drivernum = para.firstChildElement("value").text().toInt(&ok);

		QString driver;
		switch(drivernum) {
	case 1:
		if (QDir("D:").exists() == false)
		{

			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("d:");

		break;
	case 2:
		if (QDir("E:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("e:");

		break;
	case 3:
		if (QDir("F:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("f:");

		break;

	case 4:
		if (QDir("G:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("g:");

		break;

	default:
		if (QDir("E:").exists() == false)
		{
			QMessageBox msgBox;
			msgBox.setText("U�̲����ڣ������U�̣�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
			return;
		}
		driver = QString("e:");
		}

		//���ļ�
		QFile file(driver + QString("\\updatelic"));

		if (!file.open(QIODevice::ReadOnly))
		{
			QMessageBox msgBox;
			msgBox.setText("���֤�ļ������ڣ�");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();
    		return;
		}

		//���������к�
		char fingerfrombios[40];
		memset(fingerfrombios,0,40);
		int fingerfrombioslen = 0;

		//ȡ����ʵ��ֵ
		pcfingerprint fingerprintinst;
		if (fingerprintinst.getpcfingerprint(fingerfrombios,&fingerfrombioslen) != 0)
		{
			QMessageBox msgBox;
			msgBox.setText("���������кŴ���");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}

		//���ļ�������������
		QByteArray filedata = file.readAll();
		file.close();

		//�ļ���СУ��
		if ((filedata.length() % 16) != 0)
		{

			QMessageBox msgBox;
			msgBox.setText("�ļ�У�����");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;		
		}

		//����
		unsigned char plainlicense[80];
		unsigned char cipherlicense[80];

		memset(plainlicense,0,80);
		memset(cipherlicense,0,80);

		//������֯
		for (int i = 0; i< filedata.length(); i++)
		{
			cipherlicense[i] = filedata[i];
		}

		//����
		if (safenetinst.AESDecrypt(cipherlicense,plainlicense,filedata.length()) == 1)
		{
			std::cout << "AESDecrypt ʧ��";
			return;
		}

		//����ʵ�����Ա����к�
		bool sameflag = true;
		//���ȱȶ�
		if (plainlicense[9] == (unsigned char)fingerfrombioslen)
		{
			//�Ƚ�����
			for (int i = 0; i< fingerfrombioslen; ++i)
			{
				if (plainlicense[i+10] != (unsigned char)fingerfrombios[i])
				{
					sameflag = false;
					break;
				}
			}
		}
		else
		{
			sameflag = false;
		}

		if (sameflag == false)
		{
			QMessageBox msgBox;
			msgBox.setText("���֤����");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return;
		}

		//д���ڵ����ܹ�
		if (safenetinst.writedata((char *)plainlicense,0,9) != 0)
		{
			QMessageBox msgBox;
			msgBox.setText("д���ܹ�����");
			msgBox.setWindowTitle("����");
			msgBox.setStandardButtons(QMessageBox::Yes );
			QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
			tb->setText("ȷ��");
			msgBox.exec();

			return ;

		}

		QMessageBox msgBox;
		msgBox.setText("�����ɹ���");
		msgBox.setWindowTitle("�ɹ�");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
		msgBox.exec();
		return;
	}
}
void set::clearlog_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("<font size = 4>��ȷ���Ƿ����Ҫ�����־��</font>");
	msgBox.setWindowTitle("�����־��");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("��");

	int ret = msgBox.exec();
	if (ret == QMessageBox::Yes )
	{
		publiccaution.addevent("����ҳ��","�����־","�û������־",1);
		mydb.ClearOtherRec();
	}

}

//�Ƿ��¼����
void set::btoprec_clicked()
{
	if (pubcom.bloprecflag == true)
	{
		pubcom.bloprecflag = false;
		btoprec->setStyleSheet(unchoosestyle);
	}
	else
	{
		pubcom.bloprecflag = true;
		btoprec->setStyleSheet(choosestyle2);
	}

	QDomNode paranode = GetParaByName("system", QString("��¼����"));
	if (pubcom.bloprecflag == 1)
	{
		paranode.firstChildElement("value").firstChild().setNodeValue(
			QString::number(1));
	}
	else
	{
		paranode.firstChildElement("value").firstChild().setNodeValue(
			QString::number(0));
	}

	//����XML�ļ�
	QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
	if (!file.open(QFile::WriteOnly))
	{

		QMessageBox msgBox;
		msgBox.setText("�������ļ�ʧ�ܣ�");
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

//�Ƿ��¼�쳣
void set::btexceptrec_clicked()
{
	if (pubcom.blexceptrecflag == true)
	{
		pubcom.blexceptrecflag = false;
		btexceptrec->setStyleSheet(unchoosestyle);
	}
	else
	{
		pubcom.blexceptrecflag = true;
		btexceptrec->setStyleSheet(choosestyle2);
	}

	QDomNode paranode = GetParaByName("system", QString("��¼�쳣"));
	if (pubcom.blexceptrecflag == 1)
	{
		paranode.firstChildElement("value").firstChild().setNodeValue(
			QString::number(1));
	}
	else
	{
		paranode.firstChildElement("value").firstChild().setNodeValue(
			QString::number(0));
	}

	//����XML�ļ�
	QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
	if (!file.open(QFile::WriteOnly))
	{
		QMessageBox msgBox;
		msgBox.setText("�������ļ�ʧ�ܣ�");
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

//������Լ�¼
void set::btcleardbginfo_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("<font size = 4>��ȷ���Ƿ����Ҫ������Լ�¼��</font>");
	msgBox.setWindowTitle("������Լ�¼��");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("��");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("��");

	int ret = msgBox.exec();
	if (ret  == QMessageBox::Yes )
	{
	mydb.ClearDbginfoRec();
	}
}


