
#include <iostream>
#include <QList>
#include "tech.h"
#include "DB.h"
#include "DBBackend.h"
#include "canif.h"

//Êý¾Ý¿â
extern DB mydb;

DBBackend::DBBackend()
{

}

DBBackend::~DBBackend()
{

}

void DBBackend::run()
{
	while (pubcom.quitthread == false)
	{
		if (mydb.sqldbopen == true)
		{
			if (mydb.SQLQueueflag == 0)
			{
				if (mydb.SQLQueuechg == 1)
				{
					mydb.SQLQueueflagmutex.lock();
					mydb.SQLQueueflag = 1;
					mydb.SQLQueueflagmutex.unlock();
					QList<QString>::iterator it;

					mydb.SQLmutex.lock();
					mydb.sqldb.transaction();
					for (it = mydb.SQLQueue.begin(); it != mydb.SQLQueue.end();++it)
					{
						mydb.sqldb.exec(*it);
					}
					mydb.sqldb.commit();
					mydb.SQLmutex.unlock();

					mydb.SQLQueue.clear();

					mydb.SQLQueuechg = 0;
				}
			}
			else if(mydb.SQLQueueflag == 1)
			{
				if (mydb.SQLQueuechg2 == 1)
				{
					mydb.SQLQueueflagmutex.lock();
					mydb.SQLQueueflag = 0;
					mydb.SQLQueueflagmutex.unlock();

					QList<QString>::iterator it;

					mydb.SQLmutex.lock();
					mydb.sqldb.transaction();
					for (it = mydb.SQLQueue2.begin(); it != mydb.SQLQueue2.end();++it)
					{
						mydb.sqldb.exec(*it);
					}
					mydb.sqldb.commit();
					mydb.SQLmutex.unlock();

					mydb.SQLQueue2.clear();

					mydb.SQLQueuechg2 = 0;
				}
			}
		}
		msleep(4000);
	}

}

