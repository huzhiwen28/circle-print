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

	SQLQueueflag = false; //0����ǰ��¼д��SQLQueue   1����ǰ��¼д��SQLQueue2
	SQLQueuechg = false; //0 ����û�иı� 1���иı���
	SQLQueuechg2 = false; //0 ����û�иı� 1���иı���

}

DB::~DB()
{
	// TODO Auto-generated destructor stub
	sqldb.close();
}

void DB::close()
{
}
//�����¼���¼
bool DB::init()
{
	if (QFile::exists(QApplication::applicationDirPath() + QString("/tech")))
	{
		//���ݿ��ʼ��
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

//ȡ��id��ֵ
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

//����ID��ֵ
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

//����ID
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

//������ʷ���ϼ�¼
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

//�����¼���¼
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


//�������Լ�¼
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

//��յ��Լ�¼
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

//��ճ����Լ�¼���������¼
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