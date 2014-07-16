/*
 * DB.cpp
 *
 *  Created on: 2009-11-24
 *      Author: Owner
 */

#include <QString>
#include "DB.h"
#include <iostream>
#include <Windows.h>
#include <QMessageBox>
#include <QFile>
#include <QApplication>

DB::DB()
{
	sqldbopen = false;

	SQLQueueflag = false; //0：当前记录写入SQLQueue   1：当前记录写入SQLQueue2
	SQLQueuechg = false; //0 队列没有改变 1队列改变了
	SQLQueuechg2 = false; //0 队列没有改变 1队列改变了

}

DB::~DB()
{
	// TODO Auto-generated destructor stub
	sqldb.close();
}

void DB::close()
{
}
//新增事件记录
bool DB::init()
{
	if (QFile::exists(QApplication::applicationDirPath() + QString("/tech")))
	{
		//数据库初始化
		sqldb = QSqlDatabase::addDatabase("QSQLITE","conn");

		sqldb.setDatabaseName(QApplication::applicationDirPath() + QString("/tech"));

		if (!sqldb.open())
		{
			sqldbopen = false;
			return false;
		}
		else
		{
			sqldbopen = true;
			return true;
		}
	}
	else
	{
		sqldbopen = false;
		return false;
	}
}

//取得id的值
bool DB::GetValue(const QString& id, int& value)
{
	SQLmutex.lock();
	QSqlQuery query = sqldb.exec(QString("SELECT value FROM save where id='") + id + QString("'"));
    SQLmutex.unlock();

	if (query.first())
	{ 
		bool ok;
		value = query.value(0).toInt(&ok);
		return true;
	}
	else
	{
		return false;
	}
}

//保存ID的值
bool DB::SaveValue(const QString& id, const int value)
{
	SQLQueueflagmutex.lock();
	if (SQLQueueflag == false)
	{
		SQLQueue.append(QString("update save set value =") + QString::number(value) + QString("  where id = '") + id + QString("'"));
		SQLQueuechg = true;
	}
	else
	{
		SQLQueue2.append(QString("update save set value =") + QString::number(value) + QString("  where id = '") + id + QString("'"));
		SQLQueuechg2 = true;
    }
	SQLQueueflagmutex.unlock();

	return true;
}

//新增ID
bool DB::NewId(const QString& id,const int value,const QString& text)
{
	SQLQueueflagmutex.lock();

	if (SQLQueueflag == false)
	{
		SQLQueue.append(QString("insert into save(id,value,text)  values('") + id + QString("',") + QString::number(value) + QString(",'") + text + QString("')"));
		SQLQueuechg = true;
	}
	else
	{
		SQLQueue2.append(QString("insert into save(id,value,text)  values('") + id + QString("',") + QString::number(value) + QString(",'") + text + QString("')"));
		SQLQueuechg2 = true;
	}
	SQLQueueflagmutex.unlock();
	return true;

}

//保存历史故障记录
bool DB::NewHisCauRec(const QString& LOCATION, const QString& CAUTION,
		const QString& DETAIL, const QString& TIME)
{
	SQLQueueflagmutex.lock();

	if (SQLQueueflag == false)
	{
		SQLQueue.append(QString("insert into historycaution(LOCATION,CAUTION,DETAIL,TIME)  values('") + LOCATION + QString("','") + CAUTION + QString("','") + DETAIL + QString("','") + TIME + QString("')"));
		SQLQueuechg = true;
	}
	else
	{
		SQLQueue2.append(QString("insert into historycaution(LOCATION,CAUTION,DETAIL,TIME)  values('") + LOCATION + QString("','") + CAUTION + QString("','") + DETAIL + QString("','") + TIME + QString("')"));
		SQLQueuechg2 = true;
	}
	SQLQueueflagmutex.unlock();
	return true;

}

//新增事件记录
bool DB::NewEventRec(const QString& LOCATION, const QString& EVENT,
		const QString& DETAIL, const QString& TIME)
{
	SQLQueueflagmutex.lock();

	if (SQLQueueflag == false)
	{
		SQLQueue.append(QString("insert into event(LOCATION,EVENT,DETAIL,TIME)  values('") + LOCATION + QString("','") + EVENT + QString("','") + DETAIL + QString("','") + TIME + QString("')"));
		SQLQueuechg = true;
	}
	else
	{
		SQLQueue2.append(QString("insert into event(LOCATION,EVENT,DETAIL,TIME)  values('") + LOCATION + QString("','") + EVENT + QString("','") + DETAIL + QString("','") + TIME + QString("')"));
		SQLQueuechg2 = true;
	}

	SQLQueueflagmutex.unlock();
	return true;
}


//新增调试记录
bool DB::NewDbginfoRec(const QString& DETAIL,const QString& TIME)
{
	SQLQueueflagmutex.lock();

	if (SQLQueueflag == false)
	{
		SQLQueue.append(QString("insert into dbginfo(DETAIL,TIME)  values('")  + DETAIL + QString("','") + TIME + QString("')"));
		SQLQueuechg = true;
	}
	else
	{
		SQLQueue2.append(QString("insert into dbginfo(DETAIL,TIME)  values('")  + DETAIL + QString("','") + TIME + QString("')"));
		SQLQueuechg2 = true;
	}

	SQLQueueflagmutex.unlock();
	return true;
}

//清空调试记录
bool DB::ClearDbginfoRec()
{
	SQLQueueflagmutex.lock();

	if (SQLQueueflag == false)
	{
		SQLQueue.append(QString("delete from dbginfo where 1"));
		SQLQueuechg = true;
	}
	else
	{
		SQLQueue2.append(QString("delete from dbginfo where 1"));
		SQLQueuechg2 = true;
	}

	SQLQueueflagmutex.unlock();
	return true;
}

//清空除调试记录外的其他记录
bool DB::ClearOtherRec()
{
	SQLQueueflagmutex.lock();

	if (SQLQueueflag == false)
	{
		SQLQueue.append(QString("delete from event where 1"));
		SQLQueue.append(QString("delete from historycaution where 1"));

		SQLQueuechg = true;
	}
	else
	{
		SQLQueue2.append(QString("delete from event where 1"));
		SQLQueue2.append(QString("delete from historycaution where 1"));
		
		SQLQueuechg2 = true;
	}

	SQLQueueflagmutex.unlock();
	return true;
}

//