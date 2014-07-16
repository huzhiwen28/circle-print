/*
 * mach4event.h
 * ����mach4�������¼�
 *  Created on: 2008-7-31
 *      Author: Owner
 */

#ifndef MACH4EVENT_H_
#define MACH4EVENT_H_
#include <QEvent>
#include <QString>
#include "customevent.h"

//�˶����������¼�
class mach4event:public QEvent
{
public:
	mach4event(): QEvent((QEvent::Type)MACH4_EVENT) {};
	virtual ~mach4event();
	//����
	char cmd;
	char status;
	//����
	char data[10];
};

//���淢�����¼�
class interfaceevent:public QEvent
{
public:
	interfaceevent(): QEvent((QEvent::Type)INTERFACE_EVENT) {};
	virtual ~interfaceevent();
	//����
	char cmd;
	char status;
	//����
	char data[10];
};

//��̨�������¼�
class backendevent:public QEvent
{
public:
	backendevent(): QEvent((QEvent::Type)BACKEND_EVENT) {};
	virtual ~backendevent();
	//����
	char cmd;
	char status;
	//����
	char data[10];
};

#endif /* MACH4EVENT_H_ */
