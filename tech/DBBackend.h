/*
* DBBackend.h
*
*  Created on: 2009-8-8
*      Author: Owner
*/

#ifndef DBBACKEND_H_
#define DBBACKEND_H_

#include <qthread>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <QtSql>
#include <QMutex>

//数据库后台
class DBBackend: public QThread
{

	Q_OBJECT

public:

	DBBackend();
	virtual ~DBBackend();

protected:
	virtual void run();

};

#endif /* DBBACKEND_H_ */