/*
 * customevent.h
 * 自定义事件的ID分配表
 *  Created on: 2008-7-31
 *      Author: Owner
 */

#ifndef CUSTOMEVENT_H_
#define CUSTOMEVENT_H_
#include <QEvent>

const int CUSTOM_EVENT_BEGIN = QEvent::User; //自定义的事件类型的开始值
const int MACH4_EVENT = CUSTOM_EVENT_BEGIN + 1; //MACH4运动控制卡的事件
const int BACKEND_EVENT = CUSTOM_EVENT_BEGIN + 2; //后台的事件
const int INTERFACE_EVENT = CUSTOM_EVENT_BEGIN + 3; //界面的事件

#endif /* CUSTOMEVENT_H_ */
