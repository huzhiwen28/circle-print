/*
* serialif.h
*
*  Created on: 2010-4-6
*  Author: huzhiwen 
*   �����豸��װ
*/

#ifndef SERIALIF_H_
#define SERIALIF_H_

#include <Windows.h>
#include <stdio.h>
#include <QMutex>


/**************************************************
 *                         ����
***************************************************/
//������һ���ֽ�
#define SERIALHEARTBEATINDEX 0


struct serialif
{
	//�豸��
	QMutex devmutex;

	//��������
	QString Name;

	//���ڲ�����
	QString BaudRate;

	//�ֽ�λ��
	QString ByteSize;

	//����У�鷽ʽ
	QString Parity;

	//����ֹͣλ
	QString StopBits;

	//RTS���ط�ʽ
	QString fRtsControl;

	//�豸�Ƿ��?
	bool deviceopen;

	//���ھ��
	HANDLE hCom;

};

//�򿪴����豸
bool Open(struct serialif* pserialif);

//�رմ����豸
bool Close(struct serialif* pserialif);

//ȡ�ô���״̬ true �� false�ر�
void GetSerialStatus(struct serialif* pserialif,bool& status);

//ȡ�ô��ڵľ��
void GetSerialHandle(struct serialif* pserialif,HANDLE& h);

//���ô�������
void SetSerialName(struct serialif* pserialif,QString strName);

//���ô��ڲ�����
void SetSerialBaudRate(struct serialif* pserialif,QString strBaudRate);

//���ô����ֽ�λ��
void SetSerialByteSize(struct serialif* pserialif,QString ByteSize);

//���ô���У�鷽ʽ
void SetSerialParity(struct serialif* pserialif,QString strParity);

//���ô���ֹͣλ
void SetSerialStopBits(struct serialif* pserialif,QString strStopBits);

//����RTS������
void SetSerialfRtsControl(struct serialif* pserialif,QString strfRtsControl);

#endif /* SERIALIF_H_ */