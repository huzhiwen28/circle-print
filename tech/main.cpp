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

#include "tech.h"

#include <QtGui>
#include <QApplication>
#include <QFile>
#include <QObject>
#include <iostream>
#include <QDomNode>
#include <QSettings>
#include <QStyle>
#include <QPlastiqueStyle>
#include <QWindowsStyle>
#include <QWindowsXPStyle>
#include <QCleanlooksStyle>
#include "welcom.h"
#include "canif.h"
#include "backend.h"
#include "backend2.h"
#include "DBBackend.h"

#include <QtSql>
#include <QTableView>
#include <QSqlTableModel>

#include "CMotor.h"
#include "DB.h"

#include "MemExport.h"
#include "set.h"
#include "password.h"
#include "opprint.h"
#include "caution.h"
#include "serialif.h"
#include "measurepulse.h"
#include "pcfingerprint.h"
#include "safenet.h"
#include "modbus.h"

double f_follow = 0.0;

QSettings settings("HTK", "cicle print");
const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraname);
QDomDocument* doc;

extern QString dlgbkcolor;
extern long screentimer;
canif candevice;
struct _modbusRTU modbusRTU;
struct _regs regs;

//加密狗检测
safenet safenetinst;

//事件消息传递对象
QObject* backendobject;
QObject* interfaceobject;
QObject* a4nobject;
QObject* selectprintobject;
QObject* measurepulseobject;
tech* ptech;

//A4N电机
CKSMCA4 *ksmc;

DB mydb;


extern QObject* setobject;
extern QObject* opobject;

//指向set的对话框
extern set* pDlgset;

//指向网头操作的对话框
extern opprint* pDlgop;

//定时器处理函数，空函数
void WINAPI onTimeFunc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2)
{

}

//屏保程序使用的事件控制器
KeyPressEater::KeyPressEater(QObject *parent) :
QObject(parent)
{

}
KeyPressEater::~KeyPressEater()
{

}

bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
	//鼠标按键事件
	if ( event->type() == QEvent::MouseButtonDblClick
		|| event->type() == QEvent::MouseButtonPress
		|| event->type() == QEvent::MouseButtonRelease
		|| event->type() == QEvent::MouseMove)
	{
		screentimer = 0;
		return QObject::eventFilter(obj, event);
	}
	
#ifndef TEST
	else if(event->type() == QEvent::KeyPress)//过滤‘esc’按键
	{
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		if (ke->key() == Qt::Key_Escape) {
			qDebug("esc");
			return true;
		}
		return QObject::eventFilter(obj, event);
	}
#endif
	
	else
	{
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}


int main(int argc, char *argv[])
{
	//休眠3秒
	Sleep(3000);

	//QApplication::setStyle(new QCleanlooksStyle);
	QApplication a(argc, argv);

	//库放置的位置，这个是为了便于程序搜索DLL
	QStringList ttt = a.libraryPaths();
	ttt.append(QCoreApplication::applicationDirPath());
	ttt.append(QCoreApplication::applicationDirPath() + QString("/plugins"));
	ttt.append(QCoreApplication::applicationDirPath() + QString("/kerneldlls"));
	a.setLibraryPaths(ttt);

	KeyPressEater *pkeyPressEater = new KeyPressEater();
	a.installEventFilter(pkeyPressEater);

	a.setStyleSheet("QTableWidget  { color: black }");
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("GBK"));


#ifdef HAVEDOG
	if (safenetinst.open() != 0)
	{
		/*
		如果未找到带有所请求的开发商 ID 及许可证 ID 的硬件狗，或没有有效的许可证，
		SFNTGetLicense 将返回错误。
		*/ 
		QMessageBox msgBox;
		msgBox.setText("没有检测到加密狗！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;

	}

	if (safenetinst.AESreqack() != 0)
	{
		/*
		如果未找到带有所请求的开发商 ID 及许可证 ID 的硬件狗，或没有有效的许可证，
		SFNTGetLicense 将返回错误。
		*/ 
		QMessageBox msgBox;
		msgBox.setText("没有检测到加密狗！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;
	}


	//检测机器序列号
	char fingerfrombios[40];
	char fingerfromdog[40];

	memset(fingerfrombios,0,40);
	memset(fingerfromdog,0,40);

	int fingerfrombioslen = 0;
	int fingerfromdoglen = 0;

	//取机器实际值
	pcfingerprint fingerprintinst;
	if (fingerprintinst.getpcfingerprint(fingerfrombios,&fingerfrombioslen) != 0)
	{
		QMessageBox msgBox;
		msgBox.setText("读机器序列号错误！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;

	}

	char licdate[20];
	memset(licdate,0,20);

	//查看加密狗中原来的日期
	if (safenetinst.readdata(licdate,0,9) == 0)
	{
		//日期没有设置
		if ( licdate[0] == '1'
			&& licdate[1] == '1'
			&&licdate[2] == '1'
			&&licdate[3] == '1'
			&&licdate[4] == '1'
			&&licdate[5] == '1'
			&&licdate[6] == '1'
			&&licdate[7] == '1'
			&&licdate[8] == '1')
		{
			memset(licdate,0,20);

			//取加密狗日期
			int year,month,day;
			safenetinst.GetDogDate(&year,&month,&day);
			QDate newdate(year+1,month,day);

			//第一次运行设置日期
			licdate[0] = '1';
			strncpy(licdate+1,newdate.toString("yyyyMMdd").toAscii(),8);

			//写日期
			if (safenetinst.writedata(licdate,0,9) != 0)
			{
				QMessageBox msgBox;
				msgBox.setText("写加密狗错误！");
				msgBox.setWindowTitle("错误");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();

				return 1;

			}

		}

	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText("读加密狗错误！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;

	}

	//查看加密狗原来的值序列号
	if (safenetinst.readdata(fingerfromdog,10,6) == 0)
	{
		//第一次运行,序列号没有设置
		if (fingerfromdog[0] == '1'
			&& fingerfromdog[1] == '1'
			&& fingerfromdog[2] == '1'
			&& fingerfromdog[3] == '1'
			&& fingerfromdog[4] == '1'
			&& fingerfromdog[5] == '1'
			)
		{

			//构造保存到加密狗的串，第一个字节为长度，后续字节为序列值
			fingerfromdog[0] = (unsigned char) fingerfrombioslen;
			for (int i = 0;i < fingerfrombioslen; ++i)
			{
				fingerfromdog[i+1] = fingerfrombios[i];
			}
			//将值保存到加密狗
			fingerfromdoglen = fingerfrombioslen +1;
			if (safenetinst.writedata(fingerfromdog,9,fingerfromdoglen) != 0)
			{
				QMessageBox msgBox;
				msgBox.setText("写加密狗错误！");
				msgBox.setWindowTitle("错误");
				msgBox.setStandardButtons(QMessageBox::Yes );
				QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
				tb->setText("确定");
				msgBox.exec();

				return 1;

			}

		} 
		else
		{
			//非第一次运行，取出值和机器的实际值比对
			if (safenetinst.readdata(fingerfromdog,9,fingerfrombioslen + 1) == 0)
			{
				bool sameflag = true;
				//长度比对
				if (fingerfromdog[0] == (unsigned char)fingerfrombioslen)
				{
					//比较内容
					for (int i = 0; i< fingerfrombioslen; ++i)
					{
						if (fingerfromdog[i+1] != fingerfrombios[i])
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
					msgBox.setText("许可证错误！");
					msgBox.setWindowTitle("错误");
					msgBox.setStandardButtons(QMessageBox::Yes );
					QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
					tb->setText("确定");
					msgBox.exec();

					return 1;
				}
			}
		}
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText("读加密狗错误！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;
	}
#endif



	//文件是否齐全XML模块在应用启动后才开始认得编码
	QFile file(QApplication::applicationDirPath() + QString("/aaa.xml"));
	if (!file.exists())
	{
		QMessageBox msgBox;
		msgBox.setText("找不到配置文件");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;
	}

	doc = new QDomDocument("mydocument");
	if (!file.open(QIODevice::ReadWrite))
	{
		QMessageBox msgBox;
		msgBox.setText("配置文件打开失败");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;
	}

	QString errorMsg;
	int errline;
	int errcol;

	if (!doc->setContent(&file, &errorMsg, &errline, &errcol))
	{
		std::cout << "errline,errcol:" << errline << "," << errcol;
		QMessageBox msgBox;
		msgBox.setText(errorMsg);
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		file.close();
		return 1;
	}
	file.close();

	QDomNode para;

	//取得网头数量
	para = GetParaByName("tune", "全机套色数");
	bool ok;
	pubcom.PrintNum = para.firstChildElement("value").text().toInt(&ok);

	//全局变量初始化，由于全局变量依赖于XML文件，所以必须放在XML初始化后面
	pubcom.init();

	//数据库初始化
	if (mydb.init() == false)
	{
		QMessageBox msgBox;
		msgBox.setText("找不到记录文件");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;
	}

	//后台初始化
	DBBackend DBBackendinst;
	DBBackendinst.start();


	//密码是否存在，如果不存在，那么使用默认密码和权限
	if (settings.value("prod/pass").toString().isNull())
	{
		para = GetParaByName("defaultright", "工艺");
		if (para.isNull())
		{
			publiccaution.adddbginfo("工艺 para.isNull()");
			return 1;
		}
		settings.setValue("prod/pass", para.firstChildElement("pass").text());
		settings.setValue("prod/modright",
			para.firstChildElement("userright").text());
	}
	if (settings.value("mach/pass").toString().isNull())
	{
		para = GetParaByName("defaultright", "机器");
		if (para.isNull())
		{
			publiccaution.adddbginfo("机器 para.isNull()");
			return 1;
		}
		settings.setValue("mach/pass", para.firstChildElement("pass").text());
		settings.setValue("mach/modright",
			para.firstChildElement("userright").text());
	}
	if (settings.value("tune/pass").toString().isNull())
	{
		para = GetParaByName("defaultright", "调试");
		if (para.isNull())
		{
			publiccaution.adddbginfo("调试 para.isNull()");
			return 1;
		}
		settings.setValue("tune/pass", para.firstChildElement("pass").text());
		settings.setValue("tune/modright",
			para.firstChildElement("userright").text());
	}

	if (settings.value("screensaver/flag").toString().isNull())
	{
		para = GetParaByName("screensaver", "开关");
		if (para.isNull())
		{
			publiccaution.adddbginfo("开关 para.isNull()");
			return 1;
		}
		settings.setValue("screensaver/flag", para.firstChildElement("default").text());

		para = GetParaByName("screensaver", "等待时间");
		if (para.isNull())
		{
			publiccaution.adddbginfo("等待时间 para.isNull()");
			return 1;
		}
		settings.setValue("screensaver/time",
			para.firstChildElement("default").text());
	}


	//modbus 串口参数设置
	para = GetParaByName("dev", "串口选择");
	modbusRTU_slave_SetSerialName(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "串口通讯速率");
	modbusRTU_slave_SetSerialBaudRate(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "串口校验方式");
	modbusRTU_slave_SetSerialParity(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "串口字节位数");
	modbusRTU_slave_SetSerialByteSize(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "串口停止位");
	modbusRTU_slave_SetSerialStopBits(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "RTS流控制");
	modbusRTU_slave_SetSerialfRtsControl(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "CAN失败次数记为离线设置");
	pubcom.canfailedcnt = para.firstChildElement("value").text().toInt(&ok);

	//modbus参数设置
	para = GetParaByName("modbus", "ID编号");
	//设置modbus id
	set_modbusRTU_slave_ModbusID(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("modbus", "1.5T时间");
	//设置1.5T时间定义
	set_modbusRTU_slave_t15(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("modbus", "3.5T时间");
	//设置3.5T时间定义
	set_modbusRTU_slave_t35(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("modbus", "应答等待时间");
	//设置应答间隔时间
	set_modbusRTU_slave_answaittime(&modbusRTU,para.firstChildElement("value").text().toInt(&ok));

	memset(&(modbusRTU.ov1),0,sizeof(OVERLAPPED));
	memset(&(modbusRTU.ov2),0,sizeof(OVERLAPPED));
	memset(&(modbusRTU.ov3),0,sizeof(OVERLAPPED));

	modbusRTU.ov1.hEvent = CreateEvent(NULL,true,false,NULL);
	modbusRTU.ov2.hEvent = CreateEvent(NULL,true,false,NULL);
	modbusRTU.ov3.hEvent = CreateEvent(NULL,true,false,NULL);


	//网头状态值的复位
	for(int i = 0; i < 16; ++i)
	{
		int x,y,z;
		mydb.GetValue(QString("x") + QString::number(i) ,x);
		mydb.GetValue(QString("y") + QString::number(i) ,y);
		mydb.GetValue(QString("z") + QString::number(i) ,z);
		pubcom.xstatus[i] = x;
		pubcom.ystatus[i] = y;
		pubcom.zstatus[i] = z;
	}

	//CAN卡打开
	if ( candevice.open() == false)
	{
		QMessageBox msgBox;
		msgBox.setText("CAN通讯卡卡打开失败");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		publiccaution.addcaution(PCCAUTIONLOCATION,PCNOCANCATION);
		return 1;
	}

	//检测运动卡，待补充
	ksmc = new CKSMCA4;
#ifdef HAVEA4NCARD

	//卡不能打开
	if(pubcom.cardok == false)
	{
		QMessageBox msgBox;
		msgBox.setText("运动卡打开失败,请重新打开程序");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		return 1;
	}
#endif
	//根据网头数量增加马达
	for (int ii = 0; ii < pubcom.PrintNum; ii++)
	{
		if (pubcom.activeprintlist.contains(ii) == true)
		{
			ksmc->AddMotor(ii + 1);
		}
	}

	para = GetParaByName("prod", "找零速度");
	ksmc->SetHMSpeed(para.firstChildElement("value").text().toInt(&ok));

	//设置参数
	para = GetParaByName("prod", "高速找零速度");
	ksmc->SetHMSpeedHigh(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("prod", "对花速度");
	ksmc->SetJogSpeed(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("prod", "高速对花速度");
	ksmc->SetJogSpeedHigh(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("tune", "网头周长");
	ksmc->SetGirth(para.firstChildElement("value").text().toFloat(&ok));

	para = GetParaByName("tune", "跟随滞后量");
	pubcom.Follow = para.firstChildElement("value").text().toFloat(&ok)*0.01;
	ksmc->SetFollowDelay(pubcom.Follow);

	para = GetParaByName("tune", "机头间隔距离");
	ksmc->SetOffset(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("machsub1", "全机最高速限制");
	ksmc->SetMaxSpeed(para.firstChildElement("value").text().toInt(&ok)*1000);

	para = GetParaByName("machsub1", "抬网自转速度及旋向调整");
	ksmc->SetCCWJogSpeed(para.firstChildElement("value").text().toInt(&ok));

	ksmc->SetOutPut(-1, 0);

	para = GetParaByName("machsub1", "主编码器选正负");
	ksmc->SetPulseDirect(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("tune", "编码器分辨率");
	ksmc->SetPulsePerCircle(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("tune", "摩擦系数");
	ksmc->SetPulserPerMeter(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "伺服电机变速箱输出轴齿数");
	ksmc->SetToothNum1(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "手脉输入信号滤波参数");
	ksmc->SetFollowFilter(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("dev", "伺服电机变速箱速比");
	ksmc->SetServGearBoxScale(para.firstChildElement("value").text().toDouble(&ok));

	para = GetParaByName("tune", "伺服电机正反转选择");
	pubcom.ServoDir = para.firstChildElement("value").text().toInt(&ok);

	para = GetParaByName("tune", "网头齿数");
	ksmc->SetToothNum2(para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("machsub1", "上浆磁台对应磁台编号");
	pubcom.Shangjiangcitaibianhao = para.firstChildElement("value").text().toInt(&ok);

	int paraVal = 0;

	//驱动器参数设置
	para = GetParaByName("driver", "第一位置增益");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			//ksmc->SetDriverPar(index+1,16,paraVal);
			ksmc->SetDriverPar(index+1,16,10);

		}
	}

	para = GetParaByName("driver", "第一速度增益");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			//ksmc->SetDriverPar(index+1,17,paraVal);
			ksmc->SetDriverPar(index+1,17,10);

		}
	}

	para = GetParaByName("driver", "第一速度积分时间常数");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			//ksmc->SetDriverPar(index+1,18,paraVal);
			ksmc->SetDriverPar(index+1,18,10);

		}
	}

	para = GetParaByName("driver", "第一速度检测过滤");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			//ksmc->SetDriverPar(index+1,19,paraVal);
			ksmc->SetDriverPar(index+1,19,5);
		}
	}

	para = GetParaByName("driver", "第一扭矩过滤时间常数");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,20,paraVal);

		}
	}

	para = GetParaByName("driver", "第二位置增益");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,24,paraVal);

		}
	}

	para = GetParaByName("driver", "第二速度增益");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,25,paraVal);

		}
	}

	para = GetParaByName("driver", "第二速度积分时间常数");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,26,paraVal);

		}
	}

	para = GetParaByName("driver", "第二速度检测过滤");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,27,paraVal);

		}
	}

	para = GetParaByName("driver", "第二扭矩过滤时间常数");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,28,paraVal);

		}
	}

	para = GetParaByName("driver", "第二增益设置");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,48,paraVal);

		}
	}

	para = GetParaByName("driver", "增益切换模式");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,49,paraVal);

		}
	}

	para = GetParaByName("driver", "增益切换延迟时间");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,50,paraVal);

		}
	}
	para = GetParaByName("driver", "增益切换水平");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,51,paraVal);

		}
	}
	para = GetParaByName("driver", "增益切换滞后");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,52,paraVal);

		}
	}

	para = GetParaByName("driver", "位置环增益切换时间");
	paraVal = para.firstChildElement("value").text().toInt(&ok);

	for ( int index = 0; index < pubcom.PrintNum; index++ )
	{
		int networkcanset = -1;

		ksmc->GetDriverPar(index+1,10,networkcanset);
		//参数为0则表示可以通过网络设置
		if (networkcanset == 0)
		{
			ksmc->SetDriverPar(index+1,53,paraVal);

		}
	}

	//初始化马达
	ksmc->InitMotor(-1);

	//激活马达
	ksmc->SetActiveMotor(-1);

	ksmc->StartPLC();

	para = GetParaByName("prod", "低速到高速对花时间");
	pubcom.LowToHighSpeedtime = para.firstChildElement("value").text().toInt(&ok);


	//参数值刷新到寄存器区
	initRegs(&regs);

	//PLCIO参数
	para = GetParaByName("plcio", "工控机开机允许位位置");
	SetPcEnableStartBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机主机加速位位置");
	SetPcMotorAccBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机主机减速位位置");
	SetPcMotorDecBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机报错位位置");
	SetPcErrBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机报警位位置");
	SetPcCauBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机开橡毯启动位位置");
	SetPcMotorOnBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机橡毯停止位位置");
	SetPcMotorOffBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机工作量满位位置");
	SetPcWorkFinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机设置心跳位位置");
	SetPcSetHeartBeatBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "工控机数据刷新请求位位置");
	SetPcFlushDataReqBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC主机开机请求位位置");
	SetPlcStartReqBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC主机停止位位置");
	SetPlcStopBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC报错位位置");
	SetPlcErrBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC报警位位置");
	SetPlcCauBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC进布运行位位置");
	SetPlcJinbuyunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC印花导带运行位位置");
	SetPlcYinhuadaodaiyunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC烘房网带1运行位位置");
	SetPlcHongfangwangdai1yunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC烘房网带2运行位位置");
	SetPlcHongfangwangdai2yunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC落布运行位位置");
	SetPlcLuobuyunxinBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcio", "PLC供浆磁力开关位位置");
	SetPlcGongjiangcilikaiguanBitIndex(&regs,para.firstChildElement("value").text().toInt(&ok));


	//PLCPARA参数
	para = GetParaByName("plcpara", "工控机输出IO字位置");
	SetPcOutIOWordIndex(&regs,para.firstChildElement("value").text().toInt(&ok));

	para = GetParaByName("plcpara", "工控机计米双字位置");
	SetPcWorkRecDWordIndex(&regs,para.firstChildElement("value").text().toInt(&ok));


	//输入IO处理的回调函数设置
	SetPlcStartReqFp(&regs,PlcStartReqHook);
	SetPlcStopFp(&regs,PlcStopHook);
	SetPlcErrFp(&regs,PlcErrHook);
	SetPlcCauFp(&regs,PlcCauHook);
	SetPlcJinbuyunxinFp(&regs,PlcJinbuyunxinHook);
	SetPlcYinhuadaodaiyunxinFp(&regs,PlcYinhuadaodaiyunxinHook);
	SetPlcHongfangwangdai1yunxinFp(&regs,PlcHongfangwangdai1yunxinHook);
	SetPlcHongfangwangdai2yunxinFp(&regs,PlcHongfangwangdai2yunxinHook);
	SetPlcLuobuyunxinFp(&regs,PlcLuobuyunxinHook);
	SetPlcGongjiangcilikaiguanFp(&regs,PlcGongjiangcilikaiguanHook);
	SetPlcGongjiangcilidaxiaogaibianFp(&regs,PlcGongjiangcilidaxiaogaibianHook);

	welcom welcominst;
#ifdef ONIPC
	welcominst.setWindowState( Qt::WindowFullScreen );
#endif
	welcominst.setWindowFlags(Qt::FramelessWindowHint);

	tech w;
	w.init();
	interfaceobject = &w;
	ptech = &w;
	w.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
	w.setWindowState(Qt::WindowFullScreen);
#endif

	w.setWindowFlags(Qt::FramelessWindowHint);
	w.show();

	welcominst.exec();

    //多媒体定时器，为了提高工控机线程调度精度而加入，微软内部的耦合，万恶的不确定性
	MMRESULT timer_id;
	timer_id = timeSetEvent(1, 1, (LPTIMECALLBACK)onTimeFunc, DWORD(1), TIME_PERIODIC);

	//后台线程，分别有其优先级
	backend backendinst;
	backendinst.start();
	backendinst.setPriority(QThread::HighestPriority);
	backendobject = &backendinst;

	backend2 backend2inst;
	backend2inst.start();
    backend2inst.setPriority(QThread::HighestPriority);

	modbusmain modbusmaininst(&modbusRTU);
	modbusmaininst.start();
	modbusmaininst.setPriority(QThread::TimeCriticalPriority);

#ifdef HAVESERIAL

	//打开串口
	if (modbusRTU_slave_SerialOpen(&modbusRTU) == false)
	{
		QMessageBox msgBox;
		msgBox.setText("串口打开失败");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();

		publiccaution.addcaution(SERIAL,SERIALNOEXIST);
	}

#endif
	a.exec();

#ifdef HAVESERIAL
	//关闭串口
	modbusRTU_slave_SerialClose(&modbusRTU);
	Sleep(100);
#endif

	//释放事件内核对象
	CloseHandle(modbusRTU.ov1.hEvent);
	CloseHandle(modbusRTU.ov2.hEvent);
	CloseHandle(modbusRTU.ov3.hEvent);

	//确保数据刷新到数据库文件中
	while ((mydb.SQLQueuechg == true) 
		|| (mydb.SQLQueuechg2 == true))
	{
		;
	}
	printf("database flush OK\n");

	pubcom.quitthread = true;

	modbusmaininst.wait();
	printf("modbusmaininst stop OK\n");

	backendinst.wait();
	printf("modbusmaininst stop OK\n");

	backend2inst.wait();
	printf("backend2inst stop OK\n");

	DBBackendinst.wait();
	printf("DBBackendinst stop OK\n");

	//释放定时器
	timeKillEvent(timer_id);        

#ifdef POWEROFF
	//关机
	if (pubcom.PowerOff == true)
	{
		HANDLE hToken;
		TOKEN_PRIVILEGES tkp;
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (GetVersionEx(&osvi) == 0)
			return false;
		// Determine the platform
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			// Windows NT 3.51, Windows NT 4.0, Windows 2000,
			// Windows XP, or Windows .NET Server
			if (!OpenProcessToken(GetCurrentProcess(),
				TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
				return false;
			LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
			tkp.PrivilegeCount = 1; // one privilege to set
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
		}
		ExitWindowsEx(EWX_SHUTDOWN,0);
	}
#endif

	return  0;
}

