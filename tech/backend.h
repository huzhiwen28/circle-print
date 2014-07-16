/*
 * backend.h
 *
 *  Created on: 2009-8-8
 *      Author: Owner
 */

#ifndef BACKEND_H_
#define BACKEND_H_

#include <qthread>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <QMap>
#include "custevent.h"

class backend: public QThread
{

	Q_OBJECT

public:
    QTime t;

	backend();
	virtual ~backend();
	void flushtext(QString text);

protected:
	virtual void run();
	void customEvent(QEvent *e);

	//网头是否已经收到消息
	QMap<unsigned int, bool> heartbeatmap;

};


#endif /* BACKEND_H_ */
