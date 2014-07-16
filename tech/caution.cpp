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
#include <QTimer>
#include <QString>
#include <QUrl>
#include <QTextCodec>
#include <QPainter>
#include <iostream>
#include "caution.h"
#include "tech.h"
#include "canif.h"
#include "DB.h"

QString cautionstyle = QString("background-color: #AA0000; font-size: 40px");
QString normalstyle = QString("font-size: 25px");

extern QPicture topbarpic;
extern QString choosestyle2;
extern QString unchoosestyle;
extern char cautionbkflag ;
extern char cautionbkchgflag ;
extern char datetimechgflag ;
extern char cautionstrchgflag ;

extern QString datetimestyle;
extern QString cautionstyle;

//�����б�
CautionList publiccaution;

extern DB mydb;

caution::caution(QWidget *parent) :
QDialog(parent)
{
	ui.setupUi(this);

	topbar = new QLabel(this);
	topbar->setGeometry(QRect(0,0,1024,50));
	topbar->setPicture(topbarpic);

	datetime = new QLabel(topbar);
	datetime->setGeometry(QRect(790,10,231,31));
	datetime->setStyleSheet(datetimestyle);

	//������ʱ��
	timer = &mytimer;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));

	ui.TabCaution->setStyleSheet("font-size: 18px");
	ui.tab->setStyleSheet("font-size: 18px");
	ui.tab_2->setStyleSheet("font-size: 18px");
	ui.tab_3->setStyleSheet("font-size: 18px");
	ui.tab_4->setStyleSheet("font-size: 18px");


	ui.pushButton_4->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px ����_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	lightmask.load(":/images/caution_mask.PNG");

	greenlight.load(":/images/caution_green.PNG");
	greenlight.setMask(lightmask);
	redlight.load(":/images/caution_red.PNG");
	redlight.setMask(lightmask);
	yellowlight.load(":/images/caution_yellow.PNG");
	yellowlight.setMask(lightmask);

	int x = 40;
	int y = 20;
	int step = 120;
	for (int i = 0; i < 24; i++)
	{
		if(i < 7)
		{
			cautionslightstatus[i] = false;

			cautionslight[i] = new QLabel(ui.tab);
			cautionslight[i]->setPixmap(greenlight);
			cautionslight[i]->setGeometry(x + step * (i % 8), y + (i / 8) * 100,
				60, 70);

			cautionslighttext[i] = new QLabel(ui.tab);
			cautionslighttext[i]->setGeometry(x + step * (i % 8), y + (i / 8) * 100
				+ 65, 80, 30);
			cautionslighttext[i]->setText(pubcom.cautionlocationidstrmap.value(i));
		}
		//���ݵ�ǰ����ͷ�������ù�����ʾ
		else if(i < (7 + pubcom.PrintNum)  && i > 6)
		{
			cautionslightstatus[i] = false;

			cautionslight[i] = new QLabel(ui.tab);
			cautionslight[i]->setPixmap(greenlight);
			cautionslight[i]->setGeometry(x + step * (i % 8), y + (i / 8) * 100,
				60, 70);

			cautionslighttext[i] = new QLabel(ui.tab);
			cautionslighttext[i]->setGeometry(x + step * (i % 8), y + (i / 8) * 100
				+ 65, 80, 30);
			cautionslighttext[i]->setText(pubcom.cautionlocationidstrmap.value(i));
		}
	}

	hiscautionsqlcurpage = 0;
	hiscautionsqlallpage = 0;
	eventsqlcurpage = 0;
	eventsqlallpage = 0;
	dbginfosqlcurpage = 0;
	dbginfosqlallpage = 0;

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from historycaution"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok); 
	}

	hiscautionsqlallpage = (value/15) + ((value%15) ? 1:0);
	hiscautionsqlcurpage = hiscautionsqlallpage;
	ui.label->setText(QString("��") + QString::number(hiscautionsqlcurpage) + QString("ҳ/��") + QString::number(hiscautionsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id  limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");


	ui.label->setText(QString("��") + QString::number(hiscautionsqlcurpage) + QString("ҳ/��") + QString::number(hiscautionsqlallpage)  + QString("ҳ"));
	oldhiscautionval = value;
	mydb.SQLmutex.lock();
	model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
	model.setHeaderData(0, Qt::Horizontal, "λ��");
	model.setHeaderData(1, Qt::Horizontal, "����");
	model.setHeaderData(2, Qt::Horizontal, "��ϸ��Ϣ");
	model.setHeaderData(3, Qt::Horizontal, "���ϲ���ʱ��");

	view = new QTableView(ui.tab_2);

	view->setGeometry(30, 40, 961, 481);
	view->setModel(&model);
	view->setColumnWidth(0,160);
	view->setColumnWidth(1,200);
	view->setColumnWidth(2,260);
	view->setColumnWidth(3,300);
	autoflushevent = false;
	ui.pushButton_10->setStyleSheet(unchoosestyle);


	mydb.SQLmutex.lock();
	tsql = mydb.sqldb.exec(QString("select count(*) from event"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	eventsqlallpage = (value/15) + ((value%15) ? 1:0);
	eventsqlcurpage = eventsqlallpage;
	ui.label_2->setText(QString("��") + QString::number(eventsqlcurpage) + QString("ҳ/��") + QString::number(eventsqlallpage)  + QString("ҳ"));


	sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	oldeventval = value;
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
	model2.setHeaderData(0, Qt::Horizontal, "λ��");
	model2.setHeaderData(1, Qt::Horizontal, "�¼�");
	model2.setHeaderData(2, Qt::Horizontal, "��ϸ��Ϣ");
	model2.setHeaderData(3, Qt::Horizontal, "�¼�����ʱ��");

	view2 = new QTableView(ui.tab_3);
	view2->setGeometry(30, 40, 961, 481);
	view2->setModel(&model2);
	view2->setColumnWidth(0,160);
	view2->setColumnWidth(1,200);
	view2->setColumnWidth(2,260);
	view2->setColumnWidth(3,300);
	autoflushhiscaution = false;
	ui.pushButton_11->setStyleSheet(unchoosestyle);

	//�����¼������Ϣ
	if (pubcom.bldbginforecflag == true)
	{
		mydb.SQLmutex.lock();
        tsql = mydb.sqldb.exec(QString("select count(*) from dbginfo"));
		mydb.SQLmutex.unlock();

		if (tsql.first())
		{
			bool ok;
			value = tsql.value(0).toInt(&ok);
		}
		dbginfosqlallpage = (value/15) + ((value%15) ? 1:0);
		dbginfosqlcurpage = dbginfosqlallpage;
		ui.label_3->setText(QString("��") + QString::number(dbginfosqlcurpage) + QString("ҳ/��") + QString::number(dbginfosqlallpage)  + QString("ҳ"));


		sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
		olddbginfoval = value;

		mydb.SQLmutex.lock();
		model3.setQuery(sqlstr,mydb.sqldb);
		mydb.SQLmutex.unlock();
		model3.setHeaderData(0, Qt::Horizontal, "��ϸ��Ϣ");
		model3.setHeaderData(1, Qt::Horizontal, "��Ϣ����ʱ��");

		view3 = new QTableView(ui.tab_4);
		view3->setGeometry(30, 40, 961, 481);
		view3->setModel(&model3);
		view3->setColumnWidth(0,620);
		view3->setColumnWidth(1,300);

		ui.pushButton_15->setStyleSheet(unchoosestyle);
		autoflushdbginfo = false;
	}
	else//ɾ��������ϢTABҳ��
	{
		ui.TabCaution->removeTab(3);
	}

	flushtimer = 0;

}

caution::~caution()
{

}

//��̬TABҳ
void caution::init(int modright)
{
	publiccaution.addevent("����ҳ��","����","�û��������ҳ��",1);

	//������������
	pubcom.CurrentWndMutex.lock();
	pubcom.CurrentWnd = 3;
	pubcom.CurrentWndMutex.unlock();

	//��ʱ�������Ź���
	timer->start(100); //100ms��ʱ
	return;
}

void caution::on_pushButton_4_clicked()
{
	publiccaution.addevent("����ҳ��","�˳�","�û��˳�����ҳ��",1);
	close();
}

//��ʱ���������
void caution::timeupdate()
{
	if (pubcom.CurrentWnd == 3)
	{
		bool cautionchg = false;
		for (int i = 0; i < 24; i++)
		{
			if(i < 7)
			{
				if (publiccaution.hascaution(i) == true && cautionslightstatus[i]
				== false)
				{
					cautionchg = true;
					cautionslight[i]->setPixmap(redlight);
					cautionslightstatus[i] = true;
				}
				else if (publiccaution.hascaution(i) == false && cautionslightstatus[i]
				== true)
				{
					cautionchg = true;
					cautionslight[i]->setPixmap(greenlight);
					cautionslightstatus[i] = false;
				}
			}
			//���ݵ�ǰ����ͷ�������ù�����ʾ
			else if(i < (7 + pubcom.PrintNum)  && i > 6)
			{
				if (publiccaution.hascaution(i) == true && cautionslightstatus[i]
				== false)
				{
					cautionchg = true;
					cautionslight[i]->setPixmap(redlight);
					cautionslightstatus[i] = true;
				}
				else if (publiccaution.hascaution(i) == false && cautionslightstatus[i]
				== true)
				{
					cautionchg = true;
					cautionslight[i]->setPixmap(greenlight);
					cautionslightstatus[i] = false;
				}
			}
		}

		//���ڱ仯��
		if (datetimechgflag == 1)
		{
			datetimechgflag = 0;
			datetime->setText(QDateTime::currentDateTime().toString(
				"yyyy-MM-dd hh:mm:ss"));
		}

		if(cautionchg == true)
		{
			ui.curcaution->clear();
			QString caustr;
			publiccaution.cautionstr(caustr);
			ui.curcaution->insertPlainText(caustr);
		}

		if (flushtimer < 40)
		{
			flushtimer ++;
		}
		else
		{
			flushtimer = 0;
		}

		if (flushtimer == 40)
		{
			//ˢ����ʷ
			if (autoflushhiscaution == true)
			{
				int value = 0;
				mydb.SQLmutex.lock();
				QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from historycaution"));
				mydb.SQLmutex.unlock();

				if (tsql.first())
				{
					bool ok;
					value = tsql.value(0).toInt(&ok);
				}
				else
				{
					QSqlError t= tsql.lastError();
					publiccaution.adddbginfo(t.text());
				}


				if (value != oldhiscautionval)
				{
					oldhiscautionval = value;

					hiscautionsqlallpage = (value/15) + ((value%15) ? 1:0);
					hiscautionsqlcurpage = hiscautionsqlallpage;
					ui.label->setText(QString("��") + QString::number(hiscautionsqlcurpage) + QString("ҳ/��") + QString::number(hiscautionsqlallpage)  + QString("ҳ"));


					QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id  limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
					mydb.SQLmutex.lock();
                    model.setQuery(sqlstr,mydb.sqldb);
					mydb.SQLmutex.unlock();

				}
			}

			//ˢ���¼�
			if (autoflushevent == true)
			{
				int value = 0;
				mydb.SQLmutex.lock();
				QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from event"));
				mydb.SQLmutex.unlock();

				if (tsql.first())
				{
					bool ok;
					value = tsql.value(0).toInt(&ok);
				}
				else
				{
					QSqlError t= tsql.lastError();
					publiccaution.adddbginfo(t.text());
				}

				if (oldeventval != value)
				{
					oldeventval = value;

					eventsqlallpage = (value/15) + ((value%15) ? 1:0);
					eventsqlcurpage = eventsqlallpage;
					ui.label_2->setText(QString("��") + QString::number(eventsqlcurpage) + QString("ҳ/��") + QString::number(eventsqlallpage)  + QString("ҳ"));


					QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
					mydb.SQLmutex.lock();
                    model2.setQuery(sqlstr,mydb.sqldb);
					mydb.SQLmutex.unlock();

				}
			}
			//ˢ�µ�����Ϣ
			if (autoflushdbginfo == true)
			{
				int value = 0;

				mydb.SQLmutex.lock();
				QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from dbginfo"));
				mydb.SQLmutex.unlock();

				if (tsql.first())
				{
					bool ok;
					value = tsql.value(0).toInt(&ok);
				}
				else
				{
					QSqlError t= tsql.lastError();
					publiccaution.adddbginfo(t.text());
				}
				if (olddbginfoval != value)
				{
					olddbginfoval = value;

					dbginfosqlallpage = (value/15) + ((value%15) ? 1:0);
					dbginfosqlcurpage = dbginfosqlallpage;
					ui.label_3->setText(QString("��") + QString::number(dbginfosqlcurpage) + QString("ҳ/��") + QString::number(dbginfosqlallpage)  + QString("ҳ"));


					QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");					mydb.SQLmutex.lock();
                    model3.setQuery(sqlstr,mydb.sqldb);
					mydb.SQLmutex.unlock();

				}
			}
		}
	}
}

//��ӹ���
int CautionList::addcaution(const int location, const int caution,
							const int para)
{
	switch(caution)
	{
	//ͨѶѹ�ư汾
	case VERCAUTION:
		if(hascaution(location,COMCAUTION) == true)
		{
			return true;
		}
		//CAN��������ѹ��ͨѶ���Ϻ��������
	case COMCAUTION:
	case CANNETCAUTION:
		//return true;
		if(hascaution(PCCAUTIONLOCATION,PCNOCANCATION) == true)
		{
			return true;
		}
		//���ڲ�����ѹ�ƴ�������
	case SERIALLOST:
		if(hascaution(SERIAL,SERIALNOEXIST) == true)
		{
			return true;
		}
		//�˶���������ѹ���ŷ��������
	case PRINTMOTORCAUTION:
		if(hascaution(A4NCAUTIONLOCATION) == true)
		{
			return true;
		}
	}

	if (hascaution(location, caution, para) != true)
	{
		t_caution t;
		t.code = caution;
		t.location = location;
		t.para = para;
		t.datetime = QDateTime::currentDateTime();

		mutex.lockForWrite();
		cautionlist.append(t);
		mutex.unlock();

		addhistorycaution(location,caution,para);
	}

	return true;
}
//��ʷ���ϼ�¼
int CautionList::addhistorycaution(const int location,const int caution,const int para)
{
	mydb.NewHisCauRec(pubcom.cautionlocationidstrmap.value(
		location),pubcom.cautionidstrmap.value(caution),"",QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
	return true;
}

//�����¼���¼
int CautionList::addevent(const int location,const int event,const QString detail,char eventtype)
{
	if ((eventtype == 0 && pubcom.blexceptrecflag == 1) 
		||(eventtype == 1 && pubcom.bloprecflag == 1) )
	{
		mydb.NewEventRec(pubcom.cautionlocationidstrmap.value(
			location),pubcom.eventidstrmap.value(event),detail,QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
	}
	return true;
}

//�����¼���¼
int CautionList::addevent(const QString location,const QString event,const QString detail,char eventtype)
{
	if ((eventtype == 0 && pubcom.blexceptrecflag == 1) 
		||(eventtype == 1 && pubcom.bloprecflag == 1) )
	{
		mydb.NewEventRec(location,event,detail,QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
	}
	return true;
}

//����������Ϣ
int CautionList::adddbginfo(const QString detail)
{
	if ( pubcom.bldbginforecflag == 1 )
	{
		mydb.NewDbginfoRec(detail,QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
	}

#ifdef CONSOLE_OUTPUT
	std::cout << detail.toStdString() << std::endl;
#endif
	 
	return true;
}


//ɾ������
int CautionList::delcaution(const int location, const int caution)
{
	QLinkedList<t_caution>::iterator i;
	QLinkedList<t_caution>::iterator j;

	mutex.lockForWrite();
	for (i = cautionlist.begin(); i != cautionlist.end();)
	{
		j = i+1;
		if ((*i).code == caution && (*i).location == location)
		{
			cautionlist.erase(i);
		}
		i = j;
	}
	mutex.unlock();

	return true;
}

//ɾ������
int CautionList::delcaution(const int location)
{
	QLinkedList<t_caution>::iterator i;
	QLinkedList<t_caution>::iterator j;

	mutex.lockForWrite();
	for (i = cautionlist.begin(); i != cautionlist.end();)
	{
		j = i + 1;
		if ((*i).location == location)
		{
			cautionlist.erase(i);
		}
		i = j;
	}
	mutex.unlock();

	return true;
}

//�Ƿ��а��ӵĹ���
bool CautionList::hascaution(const int location)
{
	QLinkedList<t_caution>::iterator i;

	mutex.lockForRead();
	for (i = cautionlist.begin(); i != cautionlist.end(); ++i)
	{
		if ((*i).location == location)
		{
			mutex.unlock();
			return true;
		}
	}
	mutex.unlock();

	return false;
}

//ȡ�ù����б�����
int CautionList::count()
{
	int number = 0;
	mutex.lockForRead();
	number = cautionlist.count();
	mutex.unlock();
	return number;
}

//����indexȡ�ù���
int CautionList::getcautionbyindex(int index,t_caution& caution)
{

	mutex.lockForRead();
	if(index >= 0
		&& index < cautionlist.count())
	{
		int count = 0;
		QLinkedList<t_caution>::iterator it;
		for(it = cautionlist.begin(); it != cautionlist.end(); ++it)
		{
			if(count == index)
			{
				caution = *it;
				mutex.unlock();
				return true;
			}
			count ++;
		}
	}
	mutex.unlock();
	return false;
}

//ȡ�ù����ַ���
int CautionList::cautionstr(QString& str)
{
	QLinkedList<t_caution>::iterator i;
	mutex.lockForRead();

	for (i = cautionlist.begin(); i != cautionlist.end(); ++i)
	{
		str = str + pubcom.cautionlocationidstrmap.value((*i).location) + QString(": ") + pubcom.cautionidstrmap.value((*i).code) + (*i).datetime.toString(" yyyy.MM.dd hh:mm:ss") + QString("\n");
	}
	mutex.unlock();
	return true;
}

//�Ƿ���ĳ������
bool CautionList::hascaution(const int location, const int caution)
{
	QLinkedList<t_caution>::iterator i;

	mutex.lockForRead();
	for (i = cautionlist.begin(); i != cautionlist.end(); ++i)
	{
		if ((*i).code == caution && (*i).location == location)
		{
			mutex.unlock();
			return true;
		}
	}

	mutex.unlock();
	return false;
}

//�Ƿ���ĳ������
bool CautionList::hascaution(const int location, const int caution,
							 const int para)
{
	QLinkedList<t_caution>::iterator i;
	mutex.lockForRead();
	for (i = cautionlist.begin(); i != cautionlist.end(); ++i)
	{
		if ((*i).code == caution && (*i).location == location && (*i).para
			== para)
		{
			mutex.unlock();
			return true;
		}
	}
	mutex.unlock();
	return false;
}

//ȡ��ĳ����ӵĹ����б�
int CautionList::getcautions(const int location, QList<t_caution>& cautions)
{
	return true;
}

//����IOӳ�������ʾ���ռ����Ӻ�PC�Ĺ��Ͼ���
bool CautionList::IOHasCaution()
{
	QLinkedList<t_caution>::iterator i;

	mutex.lockForRead();
	for (i = cautionlist.begin(); i != cautionlist.end();++i)
	{
		if ( (*i).code == VERCAUTION //�汾��ƥ��
			||(*i).code == PARCAUTION //��������
			||(*i).code == ZEROCAUTION //�������
			||(*i).code == PCERRCANCMD //���������
			||(*i).code == ZEROSIGNOUTTIME //�����źų�ʱ
			||(*i).code == XMOTORBOOTERR //X��������
			||(*i).code == YMOTORBOOTERR //Y��������
			||(*i).code == PRINTBUMPCAUTION //��ͷ��������
			||(*i).code == MAGICANSOUTTIME //��̨Ӧ��ʱ
			||(*i).code == BUMPANSOUTTIME //����Ӧ��ʱ
			||(*i).code == COMCAUTION //�������� 
			||(*i).code == CANNETCAUTION //CAN��������
			||(*i).code == EPRROMERR //EPPROM����
			)
		{
			mutex.unlock();
			return true;
		}
	}
	mutex.unlock();
	return false;
}

//����IOӳ�䣬�����ռ����Ӻ�PC�Ĺ��Ͼ���
bool CautionList::IOHasErr()
{
	QLinkedList<t_caution>::iterator i;

	mutex.lockForRead();
	for (i = cautionlist.begin(); i != cautionlist.end();++i)
	{
		if ((*i).code == PRINTMOTORCAUTION //��ͷ�ŷ�����
			||(*i).code == REPIDCAUTION  //�ظ���CANID
			||(*i).code == PCNOA4NCAUTION //IPC�������˶���
			||(*i).code == PCNOCANCATION //IPC������CAN��
			||(*i).code == PCSYSCATION //IPCϵͳ����
			||(*i).code == A4NANSOUTTIME //�˶���Ӧ��ʱ
			||(*i).code == SERIALNOEXIST //���ڲ�����
			)
		{
			mutex.unlock();
			return true;
		}
	}
	mutex.unlock();
	return false;
}


void caution::on_pushButton_clicked()
{

	publiccaution.addevent("����ҳ��-��ʷ����","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;
	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from historycaution"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	hiscautionsqlallpage = (value/15) + ((value%15) ? 1:0);
	if (hiscautionsqlallpage == 0)
	{
		hiscautionsqlcurpage = 0;
	}
	else
	{
		hiscautionsqlcurpage = 1;
	}

	ui.label->setText(QString("��") + QString::number(hiscautionsqlcurpage) + QString("ҳ/��") + QString::number(hiscautionsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id  limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
    model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_2_clicked()
{
	publiccaution.addevent("����ҳ��-��ʷ����","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from historycaution"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	hiscautionsqlallpage = (value/15) + ((value%15) ? 1:0);
	if (hiscautionsqlcurpage < hiscautionsqlallpage)
	{
		hiscautionsqlcurpage ++;
	}
	ui.label->setText(QString("��") + QString::number(hiscautionsqlcurpage) + QString("ҳ/��") + QString::number(hiscautionsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
    model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_3_clicked()
{
	publiccaution.addevent("����ҳ��-��ʷ����","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from historycaution"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	hiscautionsqlallpage = (value/15) + ((value%15) ? 1:0);
	if (hiscautionsqlcurpage > 1)
	{
		hiscautionsqlcurpage --;
	}
	ui.label->setText(QString("��") + QString::number(hiscautionsqlcurpage) + QString("ҳ/��") + QString::number(hiscautionsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_5_clicked()
{
	publiccaution.addevent("����ҳ��-��ʷ����","���һҳ��ť","������һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from historycaution"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	hiscautionsqlallpage = (value/15) + ((value%15) ? 1:0);
	hiscautionsqlcurpage = hiscautionsqlallpage;
	ui.label->setText(QString("��") + QString::number(hiscautionsqlcurpage) + QString("ҳ/��") + QString::number(hiscautionsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id  limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_8_clicked()
{
	publiccaution.addevent("����ҳ��-�¼���¼","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from event"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	eventsqlallpage = (value/15) + ((value%15) ? 1:0);

	if (eventsqlallpage == 0)
	{
		eventsqlcurpage = 0;
	}
	else
	{
		eventsqlcurpage = 1;
    }
	ui.label_2->setText(QString("��") + QString::number(eventsqlcurpage) + QString("ҳ/��") + QString::number(eventsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id  limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_7_clicked()
{
	publiccaution.addevent("����ҳ��-�¼���¼","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from event"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	eventsqlallpage = (value/15) + ((value%15) ? 1:0);
	if (eventsqlcurpage < eventsqlallpage)
	{
		eventsqlcurpage ++;
	}
	ui.label_2->setText(QString("��") + QString::number(eventsqlcurpage) + QString("ҳ/��") + QString::number(eventsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id  limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_9_clicked()
{
	publiccaution.addevent("����ҳ��-�¼���¼","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from event"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	eventsqlallpage = (value/15) + ((value%15) ? 1:0);
	if (eventsqlcurpage > 1)
	{
		eventsqlcurpage --;
	}
	ui.label_2->setText(QString("��") + QString::number(eventsqlcurpage) + QString("ҳ/��") + QString::number(eventsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_6_clicked()
{
	publiccaution.addevent("����ҳ��-�¼���¼","���һҳ��ť","������һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from event"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	eventsqlallpage = (value/15) + ((value%15) ? 1:0);
	eventsqlcurpage = eventsqlallpage;
	ui.label_2->setText(QString("��") + QString::number(eventsqlcurpage) + QString("ҳ/��") + QString::number(eventsqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_10_clicked()
{
	publiccaution.addevent("����ҳ��-��ʷ����","�Զ�ˢ�°�ť","����Զ�ˢ�°�ť",1);

	if (autoflushhiscaution == true)
	{
		autoflushhiscaution = false;
		ui.pushButton_10->setStyleSheet(unchoosestyle);
	}
	else
	{
		autoflushhiscaution = true;
		ui.pushButton_10->setStyleSheet(choosestyle2);
	}
}

void caution::on_pushButton_11_clicked()
{
	publiccaution.addevent("����ҳ��-�¼���¼","�Զ�ˢ�°�ť","����Զ�ˢ�°�ť",1);

	if (autoflushevent == true)
	{
		autoflushevent = false;
		ui.pushButton_11->setStyleSheet(unchoosestyle);
	}
	else
	{
		autoflushevent = true;
		ui.pushButton_11->setStyleSheet(choosestyle2);
	}
}

void caution::on_pushButton_12_clicked()
{
	publiccaution.addevent("����ҳ��-������Ϣ","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from dbginfo"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	dbginfosqlallpage = (value/15) + ((value%15) ? 1:0);
	if (dbginfosqlallpage == 0)
	{
		dbginfosqlcurpage = 0;
	}
	else
	{
		dbginfosqlcurpage = 1;
	}
	ui.label_3->setText(QString("��") + QString::number(dbginfosqlcurpage) + QString("ҳ/��") + QString::number(dbginfosqlallpage)  + QString("ҳ"));

	QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id  limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model3.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_16_clicked()
{
	publiccaution.addevent("����ҳ��-������Ϣ","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from dbginfo"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	dbginfosqlallpage = (value/15) + ((value%15) ? 1:0);
	if (dbginfosqlcurpage < dbginfosqlallpage)
	{
		dbginfosqlcurpage ++;
	}
	ui.label_3->setText(QString("��") + QString::number(dbginfosqlcurpage) + QString("ҳ/��") + QString::number(dbginfosqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id  limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model3.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_14_clicked()
{
	publiccaution.addevent("����ҳ��-������Ϣ","��һҳ��ť","�����һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from dbginfo"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	dbginfosqlallpage = (value/15) + ((value%15) ? 1:0);
	if (dbginfosqlcurpage > 1)
	{
		dbginfosqlcurpage --;
	}
	ui.label_3->setText(QString("��") + QString::number(dbginfosqlcurpage) + QString("ҳ/��") + QString::number(dbginfosqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model3.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_13_clicked()
{
	publiccaution.addevent("����ҳ��-������Ϣ","���һҳ��ť","������һҳ��ť",1);

	int value = 0;

	mydb.SQLmutex.lock();
	QSqlQuery tsql = mydb.sqldb.exec(QString("select count(*) from dbginfo"));
	mydb.SQLmutex.unlock();

	if (tsql.first())
	{
		bool ok;
		value = tsql.value(0).toInt(&ok);
	}

	dbginfosqlallpage = (value/15) + ((value%15) ? 1:0);
	dbginfosqlcurpage = dbginfosqlallpage;
	ui.label_2->setText(QString("��") + QString::number(dbginfosqlcurpage) + QString("ҳ/��") + QString::number(dbginfosqlallpage)  + QString("ҳ"));


	QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model3.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_15_clicked()
{
	publiccaution.addevent("����ҳ��-������Ϣ","�Զ�ˢ�°�ť","����Զ�ˢ�°�ť",1);

	if (autoflushdbginfo == true)
	{
		autoflushdbginfo = false;
		ui.pushButton_15->setStyleSheet(unchoosestyle);
	}
	else
	{
		autoflushdbginfo = true;
		ui.pushButton_15->setStyleSheet(choosestyle2);
	}
}



