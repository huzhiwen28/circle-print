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

//故障列表
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

	//启动定时器
	timer = &mytimer;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));

	ui.TabCaution->setStyleSheet("font-size: 18px");
	ui.tab->setStyleSheet("font-size: 18px");
	ui.tab_2->setStyleSheet("font-size: 18px");
	ui.tab_3->setStyleSheet("font-size: 18px");
	ui.tab_4->setStyleSheet("font-size: 18px");


	ui.pushButton_4->setStyleSheet(
		"QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

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
		//根据当前的网头数量设置故障显示
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
	ui.label->setText(QString("第") + QString::number(hiscautionsqlcurpage) + QString("页/总") + QString::number(hiscautionsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id  limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");


	ui.label->setText(QString("第") + QString::number(hiscautionsqlcurpage) + QString("页/总") + QString::number(hiscautionsqlallpage)  + QString("页"));
	oldhiscautionval = value;
	mydb.SQLmutex.lock();
	model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
	model.setHeaderData(0, Qt::Horizontal, "位置");
	model.setHeaderData(1, Qt::Horizontal, "故障");
	model.setHeaderData(2, Qt::Horizontal, "详细信息");
	model.setHeaderData(3, Qt::Horizontal, "故障产生时间");

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
	ui.label_2->setText(QString("第") + QString::number(eventsqlcurpage) + QString("页/总") + QString::number(eventsqlallpage)  + QString("页"));


	sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	oldeventval = value;
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
	model2.setHeaderData(0, Qt::Horizontal, "位置");
	model2.setHeaderData(1, Qt::Horizontal, "事件");
	model2.setHeaderData(2, Qt::Horizontal, "详细信息");
	model2.setHeaderData(3, Qt::Horizontal, "事件产生时间");

	view2 = new QTableView(ui.tab_3);
	view2->setGeometry(30, 40, 961, 481);
	view2->setModel(&model2);
	view2->setColumnWidth(0,160);
	view2->setColumnWidth(1,200);
	view2->setColumnWidth(2,260);
	view2->setColumnWidth(3,300);
	autoflushhiscaution = false;
	ui.pushButton_11->setStyleSheet(unchoosestyle);

	//如果记录调试信息
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
		ui.label_3->setText(QString("第") + QString::number(dbginfosqlcurpage) + QString("页/总") + QString::number(dbginfosqlallpage)  + QString("页"));


		sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
		olddbginfoval = value;

		mydb.SQLmutex.lock();
		model3.setQuery(sqlstr,mydb.sqldb);
		mydb.SQLmutex.unlock();
		model3.setHeaderData(0, Qt::Horizontal, "详细信息");
		model3.setHeaderData(1, Qt::Horizontal, "信息产生时间");

		view3 = new QTableView(ui.tab_4);
		view3->setGeometry(30, 40, 961, 481);
		view3->setModel(&model3);
		view3->setColumnWidth(0,620);
		view3->setColumnWidth(1,300);

		ui.pushButton_15->setStyleSheet(unchoosestyle);
		autoflushdbginfo = false;
	}
	else//删除调试信息TAB页面
	{
		ui.TabCaution->removeTab(3);
	}

	flushtimer = 0;

}

caution::~caution()
{

}

//动态TAB页
void caution::init(int modright)
{
	publiccaution.addevent("故障页面","进入","用户进入故障页面",1);

	//返回主界面了
	pubcom.CurrentWndMutex.lock();
	pubcom.CurrentWnd = 3;
	pubcom.CurrentWndMutex.unlock();

	//定时器在最后才工作
	timer->start(100); //100ms定时
	return;
}

void caution::on_pushButton_4_clicked()
{
	publiccaution.addevent("故障页面","退出","用户退出故障页面",1);
	close();
}

//定时处理的任务
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
			//根据当前的网头数量设置故障显示
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

		//日期变化了
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
			//刷新历史
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
					ui.label->setText(QString("第") + QString::number(hiscautionsqlcurpage) + QString("页/总") + QString::number(hiscautionsqlallpage)  + QString("页"));


					QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id  limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
					mydb.SQLmutex.lock();
                    model.setQuery(sqlstr,mydb.sqldb);
					mydb.SQLmutex.unlock();

				}
			}

			//刷新事件
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
					ui.label_2->setText(QString("第") + QString::number(eventsqlcurpage) + QString("页/总") + QString::number(eventsqlallpage)  + QString("页"));


					QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
					mydb.SQLmutex.lock();
                    model2.setQuery(sqlstr,mydb.sqldb);
					mydb.SQLmutex.unlock();

				}
			}
			//刷新调试信息
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
					ui.label_3->setText(QString("第") + QString::number(dbginfosqlcurpage) + QString("页/总") + QString::number(dbginfosqlallpage)  + QString("页"));


					QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");					mydb.SQLmutex.lock();
                    model3.setQuery(sqlstr,mydb.sqldb);
					mydb.SQLmutex.unlock();

				}
			}
		}
	}
}

//添加故障
int CautionList::addcaution(const int location, const int caution,
							const int para)
{
	switch(caution)
	{
	//通讯压制版本
	case VERCAUTION:
		if(hascaution(location,COMCAUTION) == true)
		{
			return true;
		}
		//CAN卡不存在压制通讯故障和网络故障
	case COMCAUTION:
	case CANNETCAUTION:
		//return true;
		if(hascaution(PCCAUTIONLOCATION,PCNOCANCATION) == true)
		{
			return true;
		}
		//串口不存在压制串口离线
	case SERIALLOST:
		if(hascaution(SERIAL,SERIALNOEXIST) == true)
		{
			return true;
		}
		//运动卡不存在压制伺服电机故障
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
//历史故障记录
int CautionList::addhistorycaution(const int location,const int caution,const int para)
{
	mydb.NewHisCauRec(pubcom.cautionlocationidstrmap.value(
		location),pubcom.cautionidstrmap.value(caution),"",QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
	return true;
}

//增加事件记录
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

//增加事件记录
int CautionList::addevent(const QString location,const QString event,const QString detail,char eventtype)
{
	if ((eventtype == 0 && pubcom.blexceptrecflag == 1) 
		||(eventtype == 1 && pubcom.bloprecflag == 1) )
	{
		mydb.NewEventRec(location,event,detail,QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
	}
	return true;
}

//新增调试信息
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


//删除故障
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

//删除故障
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

//是否有板子的故障
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

//取得故障列表数量
int CautionList::count()
{
	int number = 0;
	mutex.lockForRead();
	number = cautionlist.count();
	mutex.unlock();
	return number;
}

//根据index取得故障
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

//取得故障字符串
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

//是否有某个故障
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

//是否有某个故障
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

//取得某块板子的故障列表
int CautionList::getcautions(const int location, QList<t_caution>& cautions)
{
	return true;
}

//对外IO映射故障提示，收集板子和PC的故障决定
bool CautionList::IOHasCaution()
{
	QLinkedList<t_caution>::iterator i;

	mutex.lockForRead();
	for (i = cautionlist.begin(); i != cautionlist.end();++i)
	{
		if ( (*i).code == VERCAUTION //版本不匹配
			||(*i).code == PARCAUTION //参数错误
			||(*i).code == ZEROCAUTION //对零故障
			||(*i).code == PCERRCANCMD //错误的命令
			||(*i).code == ZEROSIGNOUTTIME //对零信号超时
			||(*i).code == XMOTORBOOTERR //X向马达错误
			||(*i).code == YMOTORBOOTERR //Y向马达错误
			||(*i).code == PRINTBUMPCAUTION //网头浆泵问题
			||(*i).code == MAGICANSOUTTIME //磁台应答超时
			||(*i).code == BUMPANSOUTTIME //浆泵应答超时
			||(*i).code == COMCAUTION //板子离线 
			||(*i).code == CANNETCAUTION //CAN网络离线
			||(*i).code == EPRROMERR //EPPROM错误
			)
		{
			mutex.unlock();
			return true;
		}
	}
	mutex.unlock();
	return false;
}

//对外IO映射，报错，收集板子和PC的故障决定
bool CautionList::IOHasErr()
{
	QLinkedList<t_caution>::iterator i;

	mutex.lockForRead();
	for (i = cautionlist.begin(); i != cautionlist.end();++i)
	{
		if ((*i).code == PRINTMOTORCAUTION //网头伺服问题
			||(*i).code == REPIDCAUTION  //重复的CANID
			||(*i).code == PCNOA4NCAUTION //IPC机上无运动卡
			||(*i).code == PCNOCANCATION //IPC机上无CAN卡
			||(*i).code == PCSYSCATION //IPC系统故障
			||(*i).code == A4NANSOUTTIME //运动卡应答超时
			||(*i).code == SERIALNOEXIST //串口不存在
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

	publiccaution.addevent("故障页面-历史故障","第一页按钮","点击第一页按钮",1);

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

	ui.label->setText(QString("第") + QString::number(hiscautionsqlcurpage) + QString("页/总") + QString::number(hiscautionsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id  limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
    model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_2_clicked()
{
	publiccaution.addevent("故障页面-历史故障","下一页按钮","点击下一页按钮",1);

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
	ui.label->setText(QString("第") + QString::number(hiscautionsqlcurpage) + QString("页/总") + QString::number(hiscautionsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
    model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_3_clicked()
{
	publiccaution.addevent("故障页面-历史故障","上一页按钮","点击上一页按钮",1);

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
	ui.label->setText(QString("第") + QString::number(hiscautionsqlcurpage) + QString("页/总") + QString::number(hiscautionsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_5_clicked()
{
	publiccaution.addevent("故障页面-历史故障","最后一页按钮","点击最后一页按钮",1);

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
	ui.label->setText(QString("第") + QString::number(hiscautionsqlcurpage) + QString("页/总") + QString::number(hiscautionsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,CAUTION,DETAIL,TIME FROM historycaution order by id  limit ") + QString::number((hiscautionsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_8_clicked()
{
	publiccaution.addevent("故障页面-事件记录","第一页按钮","点击第一页按钮",1);

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
	ui.label_2->setText(QString("第") + QString::number(eventsqlcurpage) + QString("页/总") + QString::number(eventsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id  limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_7_clicked()
{
	publiccaution.addevent("故障页面-事件记录","下一页按钮","点击下一页按钮",1);

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
	ui.label_2->setText(QString("第") + QString::number(eventsqlcurpage) + QString("页/总") + QString::number(eventsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id  limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_9_clicked()
{
	publiccaution.addevent("故障页面-事件记录","上一页按钮","点击上一页按钮",1);

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
	ui.label_2->setText(QString("第") + QString::number(eventsqlcurpage) + QString("页/总") + QString::number(eventsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_6_clicked()
{
	publiccaution.addevent("故障页面-事件记录","最后一页按钮","点击最后一页按钮",1);

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
	ui.label_2->setText(QString("第") + QString::number(eventsqlcurpage) + QString("页/总") + QString::number(eventsqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT LOCATION,EVENT,DETAIL,TIME FROM event order by id limit ") + QString::number((eventsqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model2.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_10_clicked()
{
	publiccaution.addevent("故障页面-历史故障","自动刷新按钮","点击自动刷新按钮",1);

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
	publiccaution.addevent("故障页面-事件记录","自动刷新按钮","点击自动刷新按钮",1);

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
	publiccaution.addevent("故障页面-调试信息","第一页按钮","点击第一页按钮",1);

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
	ui.label_3->setText(QString("第") + QString::number(dbginfosqlcurpage) + QString("页/总") + QString::number(dbginfosqlallpage)  + QString("页"));

	QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id  limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model3.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();
}

void caution::on_pushButton_16_clicked()
{
	publiccaution.addevent("故障页面-调试信息","上一页按钮","点击上一页按钮",1);

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
	ui.label_3->setText(QString("第") + QString::number(dbginfosqlcurpage) + QString("页/总") + QString::number(dbginfosqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id  limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model3.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_14_clicked()
{
	publiccaution.addevent("故障页面-调试信息","下一页按钮","点击下一页按钮",1);

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
	ui.label_3->setText(QString("第") + QString::number(dbginfosqlcurpage) + QString("页/总") + QString::number(dbginfosqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model3.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_13_clicked()
{
	publiccaution.addevent("故障页面-调试信息","最后一页按钮","点击最后一页按钮",1);

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
	ui.label_2->setText(QString("第") + QString::number(dbginfosqlcurpage) + QString("页/总") + QString::number(dbginfosqlallpage)  + QString("页"));


	QString sqlstr = QString("SELECT DETAIL,TIME FROM dbginfo order by id limit ") + QString::number((dbginfosqlcurpage -1)*15) + QString(",15");
	mydb.SQLmutex.lock();
	model3.setQuery(sqlstr,mydb.sqldb);
	mydb.SQLmutex.unlock();

}

void caution::on_pushButton_15_clicked()
{
	publiccaution.addevent("故障页面-调试信息","自动刷新按钮","点击自动刷新按钮",1);

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



