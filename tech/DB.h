/*
 * DB.h
 *
 *  Created on: 2009-11-24
 *      Author: Owner
 *      ���ݿ������װ
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

	//ȡ��id��ֵ
	bool GetValue(const  QString& id,int& value);

	//����id��ֵ
	bool SaveValue(const QString& id,const int value);

    //����ID
	bool NewId(const QString& id,const int value,const QString& text);

	//������ʷ���ϼ�¼
	bool NewHisCauRec(const QString& LOCATION,const QString& CAUTION,const QString& DETAIL,const QString& TIME);

	//�����¼���¼
	bool NewEventRec(const QString& LOCATION,const QString& EVENT,const QString& DETAIL,const QString& TIME);

	//�������Լ�¼
	bool NewDbginfoRec(const QString& DETAIL,const QString& TIME);

	//��յ��Լ�¼
	bool ClearDbginfoRec();

	//��ճ����Լ�¼���������¼
	bool ClearOtherRec();


public:

	//SQL�������
	QList<QString> SQLQueue;
	QList<QString> SQLQueue2;

	//���ݿ�
	QSqlDatabase sqldb;

	//�Ƿ�������ݿ�
	bool sqldbopen;

	bool SQLQueueflag; //0����ǰ��¼д��SQLQueue   1����ǰ��¼д��SQLQueue2
	//���б�ʶ��
	QMutex SQLQueueflagmutex;

	//���������Ҷ����ݿ�ϵͳ���������ʱ������
	QMutex SQLmutex;

	//���иı��ʶ
	bool SQLQueuechg; //0 ����û�иı� 1���иı���
	bool SQLQueuechg2; //0 ����û�иı� 1���иı���
};

#endif /* DB_H_ */
