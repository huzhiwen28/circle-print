#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QMessageBox>
#include "serialif.h"
#include "caution.h"
#include "canif.h"


//打开串口设备
bool Open(struct serialif* pserialif)
{

	pserialif->devmutex.lock();

	//设置并打开端口
	pserialif->hCom = CreateFileA(pserialif->Name.toAscii(), GENERIC_READ
		| GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (pserialif->hCom == (HANDLE) - 1)
	{
		QMessageBox msgBox;
		msgBox.setText("没有检测到加密狗！");
		msgBox.setWindowTitle("错误");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("确定");
		msgBox.exec();
	}
	else
	{

		DCB wdcb;
		GetCommState(pserialif->hCom, &wdcb);

		wdcb.BaudRate = pserialif->BaudRate.toUInt();

		wdcb.ByteSize = pserialif->ByteSize.toUInt();

		if (pserialif->Parity == QString("EVEN"))
		{
			wdcb.Parity = EVENPARITY;
		}
		else if(pserialif->Parity == QString("ODD"))
		{
			wdcb.Parity = ODDPARITY;
		}
		else
		{
			wdcb.Parity = NOPARITY;
		}

		if (pserialif->StopBits == QString("2"))
		{
			wdcb.StopBits =TWOSTOPBITS;
		}
		else
		{
			wdcb.StopBits =ONESTOPBIT;
		}

		if (pserialif->fRtsControl == QString("disable"))
		{
			wdcb.fRtsControl = RTS_CONTROL_DISABLE;
		}
		else if(pserialif->fRtsControl == QString("handshake"))
		{
			wdcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
		}
		else if(pserialif->fRtsControl == QString("toggle"))
		{
			wdcb.fRtsControl = RTS_CONTROL_TOGGLE;
		}
		else
		{
			wdcb.fRtsControl = RTS_CONTROL_ENABLE;
		}

		SetCommState(pserialif->hCom, &wdcb);
		PurgeComm(pserialif->hCom, PURGE_TXCLEAR|PURGE_RXCLEAR);


		//异步要设置超时时间
		COMMTIMEOUTS m_timeout;
		m_timeout.ReadIntervalTimeout = 1000;
		m_timeout.ReadTotalTimeoutConstant = 1000;
		m_timeout.ReadTotalTimeoutMultiplier = 1000;
		m_timeout.WriteTotalTimeoutConstant = 1000;
		m_timeout.WriteTotalTimeoutMultiplier =1000;
		SetCommTimeouts(pserialif->hCom,&m_timeout);

		pserialif->deviceopen = true;
	}

	pserialif->devmutex.unlock();
	return true;
}

//关闭串口设备
bool Close(struct serialif* pserialif)
{
	pserialif->devmutex.lock();

	if (pserialif->deviceopen == true)
	{
		pserialif->deviceopen = false;
		CloseHandle(pserialif->hCom);
	}

	pserialif->devmutex.unlock();
	return true;
}


//取得串口状态 true 打开 false关闭
void GetSerialStatus(struct serialif* pserialif,bool& status)
{
	status = pserialif->deviceopen;

}

//取得串口的句柄
void GetSerialHandle(struct serialif* pserialif,HANDLE& h)
{
	h = pserialif->hCom;
}

//设置串口名称
void SetSerialName(struct serialif* pserialif,QString strName)
{
	pserialif->Name = strName;

}

//设置串口波特率
void SetSerialBaudRate(struct serialif* pserialif,QString strBaudRate)
{
	pserialif->BaudRate = strBaudRate;

}

//设置串口字节位数
void SetSerialByteSize(struct serialif* pserialif,QString strByteSize)
{
	pserialif->ByteSize = strByteSize;

}

//设置串口校验方式
void SetSerialParity(struct serialif* pserialif,QString strParity)
{
	pserialif->Parity = strParity;

}

//设置串口停止位
void SetSerialStopBits(struct serialif* pserialif,QString strStopBits)
{
	pserialif->StopBits = strStopBits;

}

//设置RTS流控制
void SetSerialfRtsControl(struct serialif* pserialif,QString strfRtsControl)
{
	pserialif->fRtsControl = strfRtsControl;

}


