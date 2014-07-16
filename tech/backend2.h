/*
* backend2.h
*
*  Created on: 2009-8-8
*      Author: Owner
*/

#ifndef BACKEND2_H_
#define BACKEND2_H_

#include <qthread>
#include <QTime>
#include <QTimer>
#include <QThread>
#include "custevent.h"
#include <Windows.h>

class backend2: public QThread
{

	Q_OBJECT

public:

	backend2();
	virtual ~backend2();

protected:
	virtual void run();
};

#endif /* BACKEND2_H_ */