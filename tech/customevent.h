/*
 * customevent.h
 * �Զ����¼���ID�����
 *  Created on: 2008-7-31
 *      Author: Owner
 */

#ifndef CUSTOMEVENT_H_
#define CUSTOMEVENT_H_
#include <QEvent>

const int CUSTOM_EVENT_BEGIN = QEvent::User; //�Զ�����¼����͵Ŀ�ʼֵ
const int MACH4_EVENT = CUSTOM_EVENT_BEGIN + 1; //MACH4�˶����ƿ����¼�
const int BACKEND_EVENT = CUSTOM_EVENT_BEGIN + 2; //��̨���¼�
const int INTERFACE_EVENT = CUSTOM_EVENT_BEGIN + 3; //������¼�

#endif /* CUSTOMEVENT_H_ */
