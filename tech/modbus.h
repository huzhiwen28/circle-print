/*
* backend.h
*
*  Created on: 2010-11-8
*      Author: Owner
*/

#ifndef MODBUS_H_
#define MODBUS_H_

#include <qthread>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <Windows.h>
#include "serialif.h"

struct _serialif
{
	//�豸��
	QMutex devmutex;

	//��������
	int Name;

	//���ڲ�����
	int BaudRate;

	//�ֽ�λ��
	int ByteSize;

	//����У�鷽ʽ
	int Parity;

	//����ֹͣλ
	int StopBits;

	//RTS���ط�ʽ
	int fRtsControl;

	//�豸�Ƿ��?
	bool deviceopen;

	//���ھ��
	HANDLE hCom;

};

struct _modbusRTU{

	unsigned char Status; // 0 ��ʼ�� 1���� 2������
	unsigned char inbuff[256];
	unsigned int inlen;
	unsigned char outbuff[256];
	unsigned int outlen;

	//��ȷ��ʱ��
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER oldtick;   // A point in time
	LARGE_INTEGER nowtick;   // A point in time

	//modbus id
	int modbusid;

	//1.5Tʱ�䶨��us
	unsigned int t15;

	//3.5Tʱ�䶨��us
	unsigned int t35;

	//Ӧ����ʱ��ms
	unsigned int answaittime;

	OVERLAPPED ov1;
	OVERLAPPED ov2;
	OVERLAPPED ov3;

	//�����豸
	struct _serialif com;

};

//����1.5Tʱ�䶨��
void set_modbusRTU_slave_t15(struct _modbusRTU* modbus,unsigned int t);

//����3.5Tʱ�䶨��
void set_modbusRTU_slave_t35(struct _modbusRTU* modbus,unsigned int t);

//����Ӧ����ʱ��
void set_modbusRTU_slave_answaittime(struct _modbusRTU* modbus,unsigned int t);

//����modbus id
void set_modbusRTU_slave_ModbusID(struct _modbusRTU* modbus,unsigned int t);

//�򿪴����豸
bool modbusRTU_slave_SerialOpen(struct _modbusRTU* modbus);

//�رմ����豸
bool modbusRTU_slave_SerialClose(struct _modbusRTU* modbus);

//���ô�������
void modbusRTU_slave_SetSerialName(struct _modbusRTU* modbus,int Name);

//���ô��ڲ�����
void modbusRTU_slave_SetSerialBaudRate(struct _modbusRTU* modbus,int BaudRate);

//���ô���У�鷽ʽ
void modbusRTU_slave_SetSerialParity(struct _modbusRTU* modbus,int Parity);

//���ô����ֽ�λ��
void modbusRTU_slave_SetSerialByteSize(struct _modbusRTU* modbus,int ByteSize);

//���ô���ֹͣλ
void modbusRTU_slave_SetSerialStopBits(struct _modbusRTU* modbus,int StopBits);

//����RTS������
void modbusRTU_slave_SetSerialfRtsControl(struct _modbusRTU* modbus,int fRtsControl);

class modbusmain: public QThread
{

	Q_OBJECT

public:
	QTime t;
	int count;
	struct _modbusRTU* pmodbusRTU;

	modbusmain(struct _modbusRTU* pmod);
	virtual ~modbusmain();

protected:
	virtual void run();
};


#endif /* MODBUS_H_ */
