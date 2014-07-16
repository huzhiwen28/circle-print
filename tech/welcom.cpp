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

#include "welcom.h"
#include <QEvent>
#include <iostream>
#include <Windows.h>
#include <QTimer>
#include <QDateTime>
#include "canif.h"
#include <QMessageBox>
#include "canidcmd.h"
#include "caution.h"
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QPainter>
#include <QPushButton>
#include <QImage>
#include <QAbstractButton>
#include <iostream>
#include <QTextStream>
#include <QFile>
#include <QSettings>
#include <QDateTime>
#include <QMessageBox>
#include <QDomNodeList>
#include <QTextStream>
#include "CMotor.h"
#include "caution.h"


//全局的can设备
extern canif candevice;
const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);

//A4N电机
extern CKSMCA4 *ksmc;


welcom::welcom(QWidget *parent) :
QDialog(parent)
{
	pubcom.initflag = 0;
	timercnt = 0;
	ui.setupUi(this);
	ui.label->setPixmap(QPixmap(":/images/init.PNG").scaledToWidth(1024));

	//启动定时器
	timer = &mytimer;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));
	timer->start(100); //100ms定时

	//至少这么长时间显示LOGO
	QTimer::singleShot(200, this, SLOT(system_init()));
}

welcom::~welcom()
{

}

void welcom::mousePressEvent(QMouseEvent *event)
{
	//如果启动完毕，只要用户点击画面，就会退出LOGO界面，进入主界面
	if (pubcom.initflag == 1)
	{
		this->accept();
	}
}

void welcom::system_init()
{
	pubcom.machprintstatus = 0;

	//通讯测试
	tcmd tempcmd;
	tempcmd.cmd = COMTESTCMD;
	tempcmd.id = PCCANBDID;
	tempcmd.para1 = 0;
	tempcmd.para2 = 0;

	//广播的方式测试板子是否在线
	if (candevice.sendcmd(ALLID, &tempcmd) == false)
	{
		//系统故障，发送命令失败
		publiccaution.addevent("CAN通讯","通讯测试失败","初始化通讯测试失败");

		//因为发送失败，肯定导致板子的通讯故障
		QList<int>::iterator i;
		for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
		{
			pubcom.nodeonlinemap.insert((*i), false);

			if ((*i) >= PRINT1BDID && (*i) <= PRINT16BDID)
			{
				if (pubcom.presetprintstatus[*i] != 1)
				{
					//设置板子不在线故障
					publiccaution.addcaution(pubcom.bdcautionmap.value((*i)),
						COMCAUTION);
				}
			}
			else
			{
				//设置板子不在线故障
				publiccaution.addcaution(pubcom.bdcautionmap.value((*i)),
					COMCAUTION);
			}
		}
	}
	else
	{
		Sleep(100); //休眠100ms

		//收命令
		candevice.refresh();

		//查看是否有应答的命令,进而判断出板子是否在线
		QList<int>::iterator i;
		for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
		{
			QList<tcmd> cmdlist;
			int count = candevice.getcmd((*i), COMTESTCMD, 0x00,0x01,
				cmdlist);

			//有应答
			if (count >= 1)
			{
				pubcom.nodeonlinemap.insert((*i), true);
			}
			//else if(count > 1)
			//{
			//	//故障，重复ID
			//	publiccaution.addcaution(pubcom.bdcautionmap.value(
			//		((*i))), REPIDCAUTION);
			//}
			else
			{
				pubcom.nodeonlinemap.insert((*i), false);

				//对于网头，是否是故障，得看当前网头是否激活
				if ((*i) >= PRINT1BDID && (*i) <= PRINT16BDID)
				{
					if (pubcom.presetprintstatus[*i] != 1)
					{
						//设置板子不在线故障
						publiccaution.addcaution(
							pubcom.bdcautionmap.value((*i)), COMCAUTION);
					}
				}
				else
				{
					//设置板子不在线故障
					publiccaution.addcaution(pubcom.bdcautionmap.value((*i)),
						COMCAUTION);
				}
			}
		}

		//在成功通讯后才版本测试，否则没有意义
		tempcmd.cmd = VERTESTCMD;
		tempcmd.id = PCCANBDID;
		tempcmd.para1 = 0;
		tempcmd.para2 = 1;

		//广播方式发送版本命令
		if (candevice.sendcmd(ALLID, &tempcmd) == false)
		{
			//系统故障，发送命令失败
		}
		else
		{
			Sleep(100); //休眠100ms

			//收命令
			candevice.refresh();

			//查看板子版本是否适配
			QList<int>::iterator i;
			for (i = pubcom.bdlist.begin(); i != pubcom.bdlist.end(); ++i)
			{
				//只有板子在线才确认是否有适配情况
				if (pubcom.nodeonlinemap.value(*i) == true && candevice.getcmd(
					(*i), VERTESTCMD, &tempcmd) == true)
				{
					//比较版本情况
					//版本适配
					if (pubcom.chkver(tempcmd.para1, tempcmd.para2) == true)
					{
						//OK,不处理
					}
					else//版本不适配
					{
						//对于网头，是否是故障，得看当前网头是否激活
						if ((*i) >= PRINT1BDID && (*i) <= PRINT16BDID)
						{
							if (pubcom.presetprintstatus[*i] != 1)
							{
								//设置版本故障
								publiccaution.addcaution(
									pubcom.bdcautionmap.value((*i)),
									VERTESTCMD);
							}
						}
						else
						{
							//设置版本故障
							publiccaution.addcaution(pubcom.bdcautionmap.value(
								(*i)), VERTESTCMD);
						}
					}
				}
			}
		}
	}


	//刷新参数
	{
		bool ok;
		QDomNode para;
		tbdonlineparacmdpair t;
		tcmd tempcmd;

		//浆泵
		pubcom.bdonlineparacmdpair[BUMPIOBDID].clear();

		for (int i = 1; i < 16; ++i)
		{
			QString paraname = QString("浆泵") + QString::number(i) + QString("输出点");
			para = GetParaByName("machsub4", paraname);

			tempcmd.cmd = SETBUMPIONOCMD; ;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = i-1;
			tempcmd.para2
				= para.firstChildElement("value").text().toInt(&ok) - 1;

			t.sendcmd.canid = pubcom.bdnodemap.value(BUMPIOBDID);
			t.sendcmd.cmd = tempcmd; 
			t.sendcmd.sendsucceed = false;

			tempcmd.cmd = SETBUMPIONOCMD;
			tempcmd.id = BUMPIOBDID;
			tempcmd.para1 = i-1;

			t.recvcmd.cmd = tempcmd;
			t.recvcmd.recvsucceed = false;
			pubcom.bdonlineparacmdpair[BUMPIOBDID].append(t);

		}

		//磁台
		pubcom.bdonlineparacmdpair[MAGICIOBDID].clear();

		for (int i = 1; i < 16; ++i)
		{
			QString paraname = QString("磁台") + QString::number(i) + QString("输出点");
			para = GetParaByName("machsub5", paraname);

			tempcmd.cmd = SETMAGICIONOCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = i-1;
			tempcmd.para2
				= para.firstChildElement("value").text().toInt(&ok) - 1;

			t.sendcmd.canid = pubcom.bdnodemap.value(MAGICIOBDID);
			t.sendcmd.cmd = tempcmd; 
			t.sendcmd.sendsucceed = false;

			tempcmd.cmd = SETMAGICIONOCMD;
			tempcmd.id = MAGICIOBDID;
			tempcmd.para1 = i-1;

			t.recvcmd.cmd = tempcmd;
			t.recvcmd.recvsucceed = false;
			pubcom.bdonlineparacmdpair[MAGICIOBDID].append(t);

		}

		for (int i = 1; i < 16; ++i)
		{
			QString paraname = QString("磁台") + QString::number(i) + QString("模拟量输出点");
			para = GetParaByName("machsub6", paraname);

			tempcmd.cmd = SETMAGICANANOCMD;
			tempcmd.id = PCCANBDID;
			tempcmd.para1 = i-1;
			tempcmd.para2
				= para.firstChildElement("value").text().toInt(&ok) - 1;

			t.sendcmd.canid = pubcom.bdnodemap.value(MAGICIOBDID);
			t.sendcmd.cmd = tempcmd; 
			t.sendcmd.sendsucceed = false;

			tempcmd.cmd = SETMAGICANANOCMD;
			tempcmd.id = MAGICIOBDID;
			tempcmd.para1 = i-1;

			t.recvcmd.cmd = tempcmd;
			t.recvcmd.recvsucceed = false;
			pubcom.bdonlineparacmdpair[MAGICIOBDID].append(t);
		}

		//网头
		for (int ii = 0; ii < pubcom.PrintNum; ++ii)
		{
			//清除参数
			pubcom.bdonlineparacmdpair[ii].clear();

			//对于在线的网头才发送参数
			if (pubcom.nodeonlinemap.value(ii) == true)
			{
				bool ok;
				QDomNode para;
				tbdonlineparacmdpair t;
				tcmd tempcmd;

				para = GetParaByName("prod", "抬网延时");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x03;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x03;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);


				para = GetParaByName("prod", "下网延时");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x04;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x04;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "横向对花速度");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x01;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x01;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "斜向对花速度");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x02;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x02;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("machsub1", "网头调速允许按钮是否工作");
				bool btwork = para.firstChildElement("value").text().toInt(&ok);
				if (btwork == true)
				{

					para = GetParaByName("prod", "网头调速允许");

					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = 0x05;
					if (para.firstChildElement("value").text().toInt(&ok) == 0)
					{
						tempcmd.para2 = 2;
						//printf("网头调速禁止\n");
					}
					else
					{
						tempcmd.para2 = 1;
						//printf("网头调速允许\n");
					}

					t.sendcmd.canid = pubcom.bdnodemap.value(ii);
					t.sendcmd.cmd = tempcmd; 
					t.sendcmd.sendsucceed = false;

					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = ii;
					tempcmd.para1 = 0x05;
					tempcmd.para2 = 0x00;

					t.recvcmd.cmd = tempcmd;
					t.recvcmd.recvsucceed = false;
					pubcom.bdonlineparacmdpair[ii].append(t);
				}


				para = GetParaByName("prod", "横/斜向电机强制开");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x07;
				if (para.firstChildElement("value").text().toInt(&ok) == 0)
				{
					tempcmd.para2 = 2;
				}
				else
				{
					tempcmd.para2 = 1;
				}

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x07;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "抬网倒转延时");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x08;
				tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x08;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "浆泵开启时间");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x06;
				tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x06;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("prod", "磁台初始磁力");

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x09;
				tempcmd.para2 = para.firstChildElement("value").text().toInt(&ok);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x09;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("machsub1", "抬/下网气缸延时");

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x02;
				tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x02;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);


				para = GetParaByName("machsub1", "张/松网气缸延时");

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x03;
				tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x03;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("machsub1", "锁/松料杆气缸延时");

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x04;
				tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x04;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				para = GetParaByName("machsub1", "料杆升降气缸延时");

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x05;
				tempcmd.para2 = (unsigned char)(para.firstChildElement("value").text().toFloat(&ok) * 10);

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x05;
				tempcmd.para2 = 0x00;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				QString paraname = QString("网头") + QString::number(ii+ 1) + QString("对应浆泵编号");
				para = GetParaByName("machsub3", paraname);

				tempcmd.cmd = SETBUMPNOCMD ;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = ii;
				tempcmd.para2
					= para.firstChildElement("value").text().toInt(&ok) - 1;

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETBUMPNOCMD;
				tempcmd.id = ii;
				tempcmd.para1 = ii;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);

				//全机套色数
				tempcmd.cmd = SETPRINTMACHPARACMD ;
				tempcmd.id = PCCANBDID;
				tempcmd.para1 = 0x01;
				tempcmd.para2 = pubcom.PrintNum;

				t.sendcmd.canid = pubcom.bdnodemap.value(ii);
				t.sendcmd.cmd = tempcmd; 
				t.sendcmd.sendsucceed = false;

				tempcmd.cmd = SETPRINTMACHPARACMD;
				tempcmd.id = ii;
				tempcmd.para1 = 0x01;

				t.recvcmd.cmd = tempcmd;
				t.recvcmd.recvsucceed = false;
				pubcom.bdonlineparacmdpair[ii].append(t);
			}
		}

		//发送
		QMap<int,QList<tbdonlineparacmdpair> >::iterator it;
		for (it = pubcom.bdonlineparacmdpair.begin(); it != pubcom.bdonlineparacmdpair.end(); ++it)
		{
			QList<tbdonlineparacmdpair>::iterator itt;
			for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
			{
				//接收失败就发送
				if ((*itt).recvcmd.recvsucceed == false)
				{
					if (candevice.sendcmd((*itt).sendcmd.canid,
						&((*itt).sendcmd.cmd)) == false)
					{
						//产生事件，提醒用户注意
						publiccaution.addevent(
							pubcom.bdcautionmap.value(it.key()),
							SENDCMDFAILEVENT,"发送CAN命令失败");
					}
					else
					{
						(*itt).sendcmd.sendsucceed = true;
					}
				}

				//每个命令见间隔10ms
				Sleep(10);
			}
		}

		//接收
		Sleep(50);

		//收命令
		candevice.refresh();

		for (it = pubcom.bdonlineparacmdpair.begin(); it != pubcom.bdonlineparacmdpair.end(); ++it)
		{
			QList<tbdonlineparacmdpair>::iterator itt;
			for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
			{
				//成功发送并且没有收到才接收
				if ( (*itt).sendcmd.sendsucceed == true 
					&& (*itt).recvcmd.recvsucceed == false )
				{
					//对于浆泵编号设置命令，要特殊处理
					if ((*itt).sendcmd.cmd.cmd == SETBUMPNOCMD)
					{
						QList<tcmd> cmdlist;
						int count = candevice.getcmd((*itt).recvcmd.cmd.id,
							(*itt).recvcmd.cmd.cmd, cmdlist);

						//接收到了
						if (count >= 1)
						{
							(*itt).recvcmd.recvsucceed = true;
						}
					} 
					else
					{
						QList<tcmd> cmdlist;
						int count = candevice.getcmd((*itt).recvcmd.cmd.id,
							(*itt).recvcmd.cmd.cmd,(*itt).recvcmd.cmd.para1, cmdlist);

						//接收到了
						if (count >= 1)
						{
							(*itt).recvcmd.recvsucceed = true;
						}
					}
				}
			}
		}
	}


	//网头板状态命令，单个发送和接收
	for (int ii = 0; ii < pubcom.PrintNum; ++ii)
	{
		//发送网头状态命令
		tempcmd.cmd = SETMACHSTATUSCMD;
		tempcmd.id = PCCANBDID;
		tempcmd.para1 = ii;
		tempcmd.para2 = pubcom.presetprintstatus[ii];

		//发送接收
		if (candevice.sendcmd(pubcom.bdnodemap.value(ii), &tempcmd) == true)
		{
			Sleep(50); //休眠50ms

			//收命令
			candevice.refresh();

			if (candevice.getcmd(ii, SETMACHSTATUSCMD, &tempcmd) == true)
			{
				//根据返回值设置网头状态
				if (tempcmd.para2 == 0x01)
				{
					pubcom.printstatus[ii] = tempcmd.para1;
					continue;
				}
			}
		}
	}

	
	//刷新参数
	{
		bool ok;
		QDomNode para;
		tbdonlineparacmdpair t;
		tcmd tempcmd;

		//网头
		for (int ii = 0; ii < pubcom.PrintNum; ++ii)
		{
			//清除参数
			pubcom.bdonlineparacmdpair[ii].clear();

			//对于在线的网头才发送参数
			if (pubcom.nodeonlinemap.value(ii) == true)
			{
				bool ok;
				QDomNode para;
				tbdonlineparacmdpair t;
				tcmd tempcmd;


				para = GetParaByName("machsub1", "网头调速允许按钮是否工作");
				bool btwork = para.firstChildElement("value").text().toInt(&ok);
				if (btwork == true)
				{

					para = GetParaByName("prod", "网头调速允许");

					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = PCCANBDID;
					tempcmd.para1 = 0x05;
					if (para.firstChildElement("value").text().toInt(&ok) == 0)
					{
						tempcmd.para2 = 2;
						printf("网头调速禁止\n");
					}
					else
					{
						tempcmd.para2 = 1;
						printf("网头调速允许\n");
					}

					t.sendcmd.canid = pubcom.bdnodemap.value(ii);
					t.sendcmd.cmd = tempcmd; 
					t.sendcmd.sendsucceed = false;

					tempcmd.cmd = SETPRINTPARACMD;
					tempcmd.id = ii;
					tempcmd.para1 = 0x05;
					tempcmd.para2 = 0x00;

					t.recvcmd.cmd = tempcmd;
					t.recvcmd.recvsucceed = false;
					pubcom.bdonlineparacmdpair[ii].append(t);
				}
			}
		}

		//发送
		QMap<int,QList<tbdonlineparacmdpair> >::iterator it;
		for (it = pubcom.bdonlineparacmdpair.begin(); it != pubcom.bdonlineparacmdpair.end(); ++it)
		{
			QList<tbdonlineparacmdpair>::iterator itt;
			for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
			{
				//接收失败就发送
				if ((*itt).recvcmd.recvsucceed == false)
				{
					if (candevice.sendcmd((*itt).sendcmd.canid,
						&((*itt).sendcmd.cmd)) == false)
					{
						//产生事件，提醒用户注意
						publiccaution.addevent(
							pubcom.bdcautionmap.value(it.key()),
							SENDCMDFAILEVENT,"发送CAN命令失败");
					}
					else
					{
						(*itt).sendcmd.sendsucceed = true;
					}
				}

				//每个命令见间隔10ms
				Sleep(10);
			}
		}

		//接收
		Sleep(50);

		//收命令
		candevice.refresh();

		for (it = pubcom.bdonlineparacmdpair.begin(); it != pubcom.bdonlineparacmdpair.end(); ++it)
		{
			QList<tbdonlineparacmdpair>::iterator itt;
			for (itt = pubcom.bdonlineparacmdpair[it.key()].begin(); itt != pubcom.bdonlineparacmdpair[it.key()].end(); ++ itt)
			{
				//成功发送并且没有收到才接收
				if ( (*itt).sendcmd.sendsucceed == true 
					&& (*itt).recvcmd.recvsucceed == false )
				{
					//对于浆泵编号设置命令，要特殊处理
					if ((*itt).sendcmd.cmd.cmd == SETBUMPNOCMD)
					{
						QList<tcmd> cmdlist;
						int count = candevice.getcmd((*itt).recvcmd.cmd.id,
							(*itt).recvcmd.cmd.cmd, cmdlist);

						//接收到了
						if (count >= 1)
						{
							(*itt).recvcmd.recvsucceed = true;
						}
					} 
					else
					{
						QList<tcmd> cmdlist;
						int count = candevice.getcmd((*itt).recvcmd.cmd.id,
							(*itt).recvcmd.cmd.cmd,(*itt).recvcmd.cmd.para1, cmdlist);

						//接收到了
						if (count >= 1)
						{
							(*itt).recvcmd.recvsucceed = true;
						}
					}
				}
			}
		}
	}


	//启动完毕进入就绪状态
	pubcom.machprintstatus = 1;

	pubcom.initflag = 1;

}


//定时处理的任务
void welcom::timeupdate()
{
	if (timercnt < 50)
	{
		timercnt++;
	}
	else if (pubcom.initflag == 1)
	{
		this->accept();
	}
}