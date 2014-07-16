#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QMessageBox>
#include "serialif.h"
#include "caution.h"
#include "canif.h"


//�򿪴����豸
bool Open(struct serialif* pserialif)
{

	pserialif->devmutex.lock();

	//���ò��򿪶˿�
	pserialif->hCom = CreateFileA(pserialif->Name.toAscii(), GENERIC_READ
		| GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (pserialif->hCom == (HANDLE) - 1)
	{
		QMessageBox msgBox;
		msgBox.setText("û�м�⵽���ܹ���");
		msgBox.setWindowTitle("����");
		msgBox.setStandardButtons(QMessageBox::Yes );
		QAbstractButton* tb = msgBox.button(QMessageBox::Yes);
		tb->setText("ȷ��");
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


		//�첽Ҫ���ó�ʱʱ��
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

//�رմ����豸
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


//ȡ�ô���״̬ true �� false�ر�
void GetSerialStatus(struct serialif* pserialif,bool& status)
{
	status = pserialif->deviceopen;

}

//ȡ�ô��ڵľ��
void GetSerialHandle(struct serialif* pserialif,HANDLE& h)
{
	h = pserialif->hCom;
}

//���ô�������
void SetSerialName(struct serialif* pserialif,QString strName)
{
	pserialif->Name = strName;

}

//���ô��ڲ�����
void SetSerialBaudRate(struct serialif* pserialif,QString strBaudRate)
{
	pserialif->BaudRate = strBaudRate;

}

//���ô����ֽ�λ��
void SetSerialByteSize(struct serialif* pserialif,QString strByteSize)
{
	pserialif->ByteSize = strByteSize;

}

//���ô���У�鷽ʽ
void SetSerialParity(struct serialif* pserialif,QString strParity)
{
	pserialif->Parity = strParity;

}

//���ô���ֹͣλ
void SetSerialStopBits(struct serialif* pserialif,QString strStopBits)
{
	pserialif->StopBits = strStopBits;

}

//����RTS������
void SetSerialfRtsControl(struct serialif* pserialif,QString strfRtsControl)
{
	pserialif->fRtsControl = strfRtsControl;

}


