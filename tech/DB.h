/*
 * DB.h
 *
 *  Created on: 2009-11-24
 *      Author: Owner
 *      数据库操作封装
 */

#ifndef DB_H_
#define DB_H_
#include <QtSql>
#include <QTableView>
#include <QSqlTableModel>
#include <QString>
class DB
{
public:
	DB();
	virtual ~DB();
	bool init();
	void close();

	//取得id的值
	bool GetValue(const  QString& id,int& value);

	//保存id的值
	bool SaveValue(const QString& id,const int value);

    //新增ID
	bool NewId(const QString& id,const int value,const QString& text);

	//保存历史故障记录
	bool NewHisCauRec(const QString& LOCATION,const QString& CAUTION,const QString& DETAIL,const QString& TIME);

	//新增事件记录
	bool NewEventRec(const QString& LOCATION,const QString& EVENT,const QString& DETAIL,const QString& TIME);

	//新增调试记录
	bool NewDbginfoRec(const QString& DETAIL,const QString& TIME);

	//清空调试记录
	bool ClearDbginfoRec();

	//清空除调试记录外的其他记录
	bool ClearOtherRec();


public:

	//SQL缓存队列
	QList<QString> SQLQueue;
	QList<QString> SQLQueue2;

	//数据库
	QSqlDatabase sqldb;

	//是否打开了数据库
	bool sqldbopen;

	bool SQLQueueflag; //0：当前记录写入SQLQueue   1：当前记录写入SQLQueue2
	//队列标识锁
	QMutex SQLQueueflagmutex;

	//操作锁，我对数据库系统自身的锁暂时不放心
	QMutex SQLmutex;

	//队列改变标识
	bool SQLQueuechg; //0 队列没有改变 1队列改变了
	bool SQLQueuechg2; //0 队列没有改变 1队列改变了
};

#endif /* DB_H_ */
