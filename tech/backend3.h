/*
* backend3.h
*
*  Created on: 2009-8-8
*      Author: Owner
*/

#ifndef BACKEND3_H_
#define BACKEND3_H_

#include <qthread>
#include <QTime>
#include <QTimer>
#include <QThread>
#include "custevent.h"
#include <Windows.h>

class backend3: public QThread
{

	Q_OBJECT

public:

	backend3();
	virtual ~backend3();

protected:
	virtual void run();
};

#endif /* BACKEND3_H_ */