/*
* serialif.h
*
*  Created on: 2010-4-6
*  Author: huzhiwen 
*   串口设备封装
*/

#ifndef SERIALIF_H_
#define SERIALIF_H_

#include <Windows.h>
#include <stdio.h>
#include <QMutex>


/**************************************************
 *                         输入
***************************************************/
//心跳，一个字节
#define SERIALHEARTBEATINDEX 0


struct serialif
{
	//设备锁
	QMutex devmutex;

	//串口名称
	QString Name;

	//串口波特率
	QString BaudRate;

	//字节位数
	QString ByteSize;

	//串口校验方式
	QString Parity;

	//串口停止位
	QString StopBits;

	//RTS流控方式
	QString fRtsControl;

	//设备是否打开?
	bool deviceopen;

	//串口句柄
	HANDLE hCom;

};

//打开串口设备
bool Open(struct serialif* pserialif);

//关闭串口设备
bool Close(struct serialif* pserialif);

//取得串口状态 true 打开 false关闭
void GetSerialStatus(struct serialif* pserialif,bool& status);

//取得串口的句柄
void GetSerialHandle(struct serialif* pserialif,HANDLE& h);

//设置串口名称
void SetSerialName(struct serialif* pserialif,QString strName);

//设置串口波特率
void SetSerialBaudRate(struct serialif* pserialif,QString strBaudRate);

//设置串口字节位数
void SetSerialByteSize(struct serialif* pserialif,QString ByteSize);

//设置串口校验方式
void SetSerialParity(struct serialif* pserialif,QString strParity);

//设置串口停止位
void SetSerialStopBits(struct serialif* pserialif,QString strStopBits);

//设置RTS流控制
void SetSerialfRtsControl(struct serialif* pserialif,QString strfRtsControl);

#endif /* SERIALIF_H_ */