/*
 * mach4event.h
 * 定义mach4发出的事件
 *  Created on: 2008-7-31
 *      Author: Owner
 */

#ifndef MACH4EVENT_H_
#define MACH4EVENT_H_
#include <QEvent>
#include <QString>
#include "customevent.h"

//运动卡发出的事件
class mach4event:public QEvent
{
public:
	mach4event(): QEvent((QEvent::Type)MACH4_EVENT) {};
	virtual ~mach4event();
	//命令
	char cmd;
	char status;
	//数据
	char data[10];
};

//界面发出的事件
class interfaceevent:public QEvent
{
public:
	interfaceevent(): QEvent((QEvent::Type)INTERFACE_EVENT) {};
	virtual ~interfaceevent();
	//命令
	char cmd;
	char status;
	//数据
	char data[10];
};

//后台发出的事件
class backendevent:public QEvent
{
public:
	backendevent(): QEvent((QEvent::Type)BACKEND_EVENT) {};
	virtual ~backendevent();
	//命令
	char cmd;
	char status;
	//数据
	char data[10];
};

#endif /* MACH4EVENT_H_ */
