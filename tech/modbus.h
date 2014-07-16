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
	//设备锁
	QMutex devmutex;

	//串口名称
	int Name;

	//串口波特率
	int BaudRate;

	//字节位数
	int ByteSize;

	//串口校验方式
	int Parity;

	//串口停止位
	int StopBits;

	//RTS流控方式
	int fRtsControl;

	//设备是否打开?
	bool deviceopen;

	//串口句柄
	HANDLE hCom;

};

struct _modbusRTU{

	unsigned char Status; // 0 初始化 1空闲 2接受中
	unsigned char inbuff[256];
	unsigned int inlen;
	unsigned char outbuff[256];
	unsigned int outlen;

	//精确定时器
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER oldtick;   // A point in time
	LARGE_INTEGER nowtick;   // A point in time

	//modbus id
	int modbusid;

	//1.5T时间定义us
	unsigned int t15;

	//3.5T时间定义us
	unsigned int t35;

	//应答间隔时间ms
	unsigned int answaittime;

	OVERLAPPED ov1;
	OVERLAPPED ov2;
	OVERLAPPED ov3;

	//串口设备
	struct _serialif com;

};

//设置1.5T时间定义
void set_modbusRTU_slave_t15(struct _modbusRTU* modbus,unsigned int t);

//设置3.5T时间定义
void set_modbusRTU_slave_t35(struct _modbusRTU* modbus,unsigned int t);

//设置应答间隔时间
void set_modbusRTU_slave_answaittime(struct _modbusRTU* modbus,unsigned int t);

//设置modbus id
void set_modbusRTU_slave_ModbusID(struct _modbusRTU* modbus,unsigned int t);

//打开串口设备
bool modbusRTU_slave_SerialOpen(struct _modbusRTU* modbus);

//关闭串口设备
bool modbusRTU_slave_SerialClose(struct _modbusRTU* modbus);

//设置串口名称
void modbusRTU_slave_SetSerialName(struct _modbusRTU* modbus,int Name);

//设置串口波特率
void modbusRTU_slave_SetSerialBaudRate(struct _modbusRTU* modbus,int BaudRate);

//设置串口校验方式
void modbusRTU_slave_SetSerialParity(struct _modbusRTU* modbus,int Parity);

//设置串口字节位数
void modbusRTU_slave_SetSerialByteSize(struct _modbusRTU* modbus,int ByteSize);

//设置串口停止位
void modbusRTU_slave_SetSerialStopBits(struct _modbusRTU* modbus,int StopBits);

//设置RTS流控制
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
