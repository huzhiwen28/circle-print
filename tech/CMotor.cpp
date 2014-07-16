#include "CMotor.h"
#include "MemExport.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <Windows.h>
#include <QDomNode>
#include "canif.h"

const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraname);


CMotor::CMotor(int nIndex)
{
	m_nMtrIndex = nIndex-1;
	char str[32];
	sprintf(str,"#%d=1", 60000+m_nMtrIndex*200 );
	Dbgout("#%d=1\n", 60000+m_nMtrIndex*200);
	getCmdResponse(str, NULL);

	//m_bMasterFollowing = false;
	m_nStatus = 0;
}


CMotor::~CMotor()
{
	double dVal=0.0;
	getMacroVar(66402+m_nMtrIndex*100, &dVal);
	if( dVal == 1.0 )
	{
		char str[32];
		sprintf(str,"$%dkill", m_nMtrIndex+1);
		getCmdResponse(str, NULL);
	}
	
	Sleep(100); //休眠100ms

	getMacroVar(66400+m_nMtrIndex*100, &dVal);
	if( dVal == 1.0 )
	{
		char str[32];
		sprintf(str,"#%d=0", 60000+m_nMtrIndex*200 );
		getCmdResponse(str, NULL);
	}
}

//初始化
bool CMotor::InitPara()
{
	m_nStatus = 0;
	return true;
}

//马达闭环
bool CMotor::SetMtrActive()
{
	char str[32];
	sprintf(str, "$%dJ/", m_nMtrIndex+1);
	getCmdResponse(str, NULL);
	Dbgout("SetMtrActive $%dJ/\n", m_nMtrIndex+1);
	m_nStatus = 1;

	return true;
}

//马达开环
bool CMotor::SetMtrOff()
{
	char str[32];
	sprintf(str, "$%dKILL", m_nMtrIndex+1);
	getCmdResponse(str, NULL);

	Dbgout("$%dKILL\n", m_nMtrIndex+1);
	return true;
}

//对零速度
bool CMotor::SetMtrHMSpeed(double dHMSpeed)
{
	char str[32];
	sprintf(str, "#%d=%.6f", 60022+m_nMtrIndex*200, dHMSpeed);
	Dbgout("CMotor::SetMtrHMSpeed #%d=%.6f\n", 60022+m_nMtrIndex*200, dHMSpeed);
	getCmdResponse(str, NULL);
	return true;
}

//点动速度
bool CMotor::SetMtrJogSpeed(double dJogSpeed)
{
	char str[32];
	sprintf(str, "#%d=%.6f", 60022+m_nMtrIndex*200, dJogSpeed);
	Dbgout("CMotor::SetMtrJogSpeed #%d=%.6f\n", 60022+m_nMtrIndex*200, dJogSpeed);
	getCmdResponse(str, NULL);
	return true;
}
//反转速度
bool CMotor::SetMtrCCWJogSpeed(double dCCWJogSpeed)
{
	char str[32];
	sprintf(str, "#%d=%.6f", 60022+m_nMtrIndex*200, dCCWJogSpeed);
	Dbgout("CMotor::SetMtrCCWJogSpeed #%d=%.6f\n", 60022+m_nMtrIndex*200, dCCWJogSpeed);
	getCmdResponse(str, NULL);
	return true;
}

//设置最大的速度
bool CMotor::SetMtrMaxSpeed(double dMaxSpeed)
{
	char str[32];
	sprintf(str, "#%d=%.6f", 60016+m_nMtrIndex*200, dMaxSpeed);
	Dbgout("#%d=%.6f\n", 60016+m_nMtrIndex*200, dMaxSpeed);
	getCmdResponse(str, NULL);
	return true;
}

//设置网头安装位置偏移
bool  CMotor::SetOffset(double dOffset)
{
	m_dOffset = dOffset;
	return true;
}

//取得网头安装位置偏移
bool  CMotor::GetOffset(double& dOffset)
{
	dOffset = m_dOffset;
	return true;
}

//允许跟随
bool CMotor::EnableMtrMasterFollowing()
{
	if(m_nStatus == 1)
	{
		m_nStatus = 2;
		return true;
	}
	else
	{
		return false;
	}
}

//停止跟随
bool CMotor::DisableMtrMasterFollowing()
{
	if(m_nStatus == 2)
	{
		m_nStatus = 1;
		return true;
	}
	else
	{
		return false;
	}
}

//取得跟随状态
bool CMotor::GetMtrFollowing()
{
	if(m_nStatus == 2)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//加速度时间
bool CMotor::SetMtrJogTA(int nTA)
{
	char str[32];
	sprintf(str, "#%d=%d", 60020+m_nMtrIndex*200, nTA);
	getCmdResponse(str, NULL);
	Dbgout("CMotor::SetMtrJogTA #%d=%d\n", 60020+m_nMtrIndex*200, nTA);
	return true;
}
//加加速度时间
bool CMotor::SetMtrJogTS(int nTS)
{
	char str[32];
	sprintf(str, "#%d=%d", 60021+m_nMtrIndex*200, nTS);
	Dbgout("CMotor::SetMtrJogTS #%d=%d\n", 60021+m_nMtrIndex*200, nTS);
	getCmdResponse(str, NULL);
	return true;
}

//点动相对多少步
bool CMotor::JogStep(double dDist)
{
	char str[32];
	sprintf(str, "$%dJ:%d", m_nMtrIndex+1, (int)dDist);

	//只有在准备状态或者跟随状态才支持JOG点动
	if(m_nStatus == 1 ||
			m_nStatus == 2)
	{
		Dbgout("CMotor::JogStep\n");
		getCmdResponse(str, NULL);
	}
	return true;
}
//点动到某个绝对位置
bool CMotor::JogTo(double dPos)
{
	char str[32];
	sprintf(str, "$%dJ=%d", m_nMtrIndex+1, (int)dPos);
	//只有在准备状态或者跟随状态才支持JOG点动
	if(m_nStatus == 1 ||
			m_nStatus == 2)
	{
		Dbgout("CMotor::JogTo\n");
		getCmdResponse(str, NULL);
	}
	return true;
}

//正转
bool CMotor::JogPlus()
{
	char str[32];
	sprintf(str, "$%dJ+", m_nMtrIndex+1);
	//只有在准备状态
	//if(m_nStatus == 1)
	{
		Dbgout("CMotor::JogPlus\n");
		getCmdResponse(str, NULL);
	}
	return true;
}

//反转
bool CMotor::JogMinus()
{
	char str[32];
	sprintf(str, "$%dJ-", m_nMtrIndex+1);
	//只有在准备状态
	//if(m_nStatus == 1)
	{
		Dbgout("CMotor::JogMinus\n");
		getCmdResponse(str, NULL);
	}
	return true;
}

//停止转动
bool CMotor::JogStop()
{
	char str[32];
	sprintf(str, "$%dJ/", m_nMtrIndex+1);
	//if(m_nStatus == 2)
	{
		Dbgout("CMotor::JogStop\n");
		getCmdResponse(str, NULL);
		m_nStatus = 1;
	}
	return true;
}

//是否闭环了？
bool CMotor::IsReady()
{
	double dd= 0.0;
	getMacroVar(66402+m_nMtrIndex*100, &dd);
	if( dd == 1.0 )
		return true;
	else
		return false;
}

//取得电机的当前位置
bool CMotor::GetLocation(double& loction)
{
	double dd = 0.0;
	getMacroVar(66432+m_nMtrIndex*100, &dd);
	loction = dd;
	return true;
}

//取得当前状态
bool CMotor::GetStatus(int& Status)
{
	Status = m_nStatus;
	return true;
}
//设置故障
bool CMotor::SetCaution(bool bl)
{

	//进入故障状态

	if(bl == true)
	{
		m_nStatus = 5;
	}
	else//退出故障状态
	{
		m_nStatus = 1;
	}

	return true;
}

CKSMCA4::CKSMCA4()
{
	bool retval = false;
	pciCardOpen();
	Sleep(1000);//休眠1000毫秒
	retval = GetCmdID();
	if(retval == false)
	{
		Sleep(1000);
		FreeCmdID();
		printf("3\n");
		Sleep(1000);
		pciCardClose();
		printf("4\n");
		Sleep(1000);
		pciCardOpen();
		printf("5\n");
		Sleep(1000);
		retval = GetCmdID();
		printf("6\n");
	}

	pubcom.cardok = retval;

	int ii = 0;
	for( ii = 0; ii < 32; ii++ )
	{
		m_pMotor[ii]  = NULL;
		m_dPrintLocation[ii] = -1;
	}
	//default value
	m_bPulseDirect = 0;
	m_dCCWJogSpeed = 0;
	m_dHMSpeed = 0;
	m_dJogSpeed = 0;
	m_dMaxSpeed = 0;
	m_dOffset = 0;
	m_dPulsePerUnit = 0;
	m_nErrNum = 0;
	m_nGirth = 0;
	m_nPulsePerCircle = 0;
	m_nPulserPerMeter = 0;
	m_nToothNum1 = 0;
	m_nToothNum2 = 0;
	memset( m_stError, 0, 10*sizeof(struct ErrorInfo));

	//init KSMCA4 para
	//getCmdResponse("$$$", NULL);

	getCmdResponse("#79930=3", NULL);	//pulse direct default to Positive
	getCmdResponse("#69637=0", NULL);	//disable handwheel following
	getCmdResponse("#69639=0", NULL);	//clear following speed
	getCmdResponse("#69643=0", NULL);	//clear following motor
	getCmdResponse("#69608=0", NULL);	//RTI周期
	getCmdResponse("#69605=1", NULL);	//PPLC周期
	getCmdResponse("#69607=0", NULL);	//伺服周期

	//设置网头1的偏差
	bool ok;
	QDomNode para;
	para = GetParaByName("machsub2", QString("网头") + QString::number(1) + QString("纵向位置偏差"));
	SetMotor1Offset(para.firstChildElement("value").text().toDouble(&ok));
}

CKSMCA4::~CKSMCA4()
{
	StopPLC();

	DelMotor(-1);

	FreeCmdID();
	pciCardClose();
}

bool CKSMCA4::GetStatus(int nIndex,int& Status)
{
	Mutex.lock();
	bool ret = false;

	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	ret = m_pMotor[nIndex-1]->GetStatus(Status);
	Mutex.unlock();

	return ret;
}

bool CKSMCA4::AddMotor(int nIndex)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1] = new CMotor(nIndex);
	Mutex.unlock();
	return true;
}

bool CKSMCA4::DelMotor(int nIndex)
{
	Mutex.lock();
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 32; ii++ )
		{
			if( m_pMotor[ii] != NULL )
			{
				delete m_pMotor[ii];
				m_pMotor[ii] = NULL;
			}
		}
		Mutex.unlock();
		return true;
	}
	else if( nIndex <=0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}

	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	delete m_pMotor[nIndex-1];
	m_pMotor[nIndex-1] = NULL;

	Mutex.unlock();
	return true;
}

bool CKSMCA4::InitMotor(int nIndex)
{
	bool ok;
	QDomNode para;

	Mutex.lock();
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 32; ii++ )
		{
			if( m_pMotor[ii] != NULL )
			{
				m_pMotor[ii]->InitPara();
				m_pMotor[ii]->SetMtrMaxSpeed(m_dMaxSpeed);
				m_pMotor[ii]->SetMtrOff();

				para = GetParaByName("machsub2", QString("网头") + QString::number(ii+1) + QString("纵向位置偏差"));

				m_pMotor[ii]->SetOffset(para.firstChildElement("value").text().toDouble(&ok));
			}
		}
		Mutex.unlock();
		return true;
	}
	else if( nIndex <=0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}

	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}

	m_pMotor[nIndex-1]->InitPara();
	m_pMotor[nIndex-1]->SetMtrMaxSpeed(m_dMaxSpeed);
	m_pMotor[nIndex-1]->SetMtrOff();

	para = GetParaByName("machsub2", QString("网头") + QString::number(nIndex) + QString("纵向位置偏差"));

	m_pMotor[nIndex-1]->SetOffset(para.firstChildElement("value").text().toDouble(&ok));

	Mutex.unlock();
	return true;
}

bool CKSMCA4::SetActiveMotor(int nIndex)
{
	Mutex.lock();
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 32; ii++ )
		{
			if( m_pMotor[ii] != NULL )
			{
				m_pMotor[ii]->SetMtrActive();
			}
		}
		Mutex.unlock();
		return true;
	}
	else if( nIndex <=0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}

	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrActive();

    Mutex.unlock();
	return true;
}

bool CKSMCA4::SetOffMotor(int nIndex)
{
	Mutex.lock();
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 32; ii++ )
		{
			if( m_pMotor[ii] != NULL )
			{
				m_pMotor[ii]->SetMtrOff();
			}
		}
		Mutex.unlock();
		return true;
	}
	else if( nIndex <=0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}

	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrOff();

	Mutex.unlock();
	return true;

}

//进入跟随
bool CKSMCA4::EnableMasterFollowing(int nIndex)
{
	Dbgout("Enable %d\n",nIndex);
	Mutex.lock();
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 32; ii++ )
		{
			if( m_pMotor[ii] != NULL)
			{
				m_pMotor[ii]->EnableMtrMasterFollowing();
			}
		}
	}
	else if( nIndex <=0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	else
	{
		if( m_pMotor[nIndex-1] == NULL )
		{
			Mutex.unlock();
			return false;
		}
		m_pMotor[nIndex-1]->EnableMtrMasterFollowing();
	}

	int nMFMask = 0;
	for( int ii = 0; ii < 32; ii++ )
	{
		if( m_pMotor[ii] == NULL )
			continue;
		if( m_pMotor[ii]->GetMtrFollowing() )
		{
			nMFMask += 1<<ii;
		}
	}
	char str[32];
	sprintf(str, "#69643=%d", nMFMask);
	getCmdResponse(str, NULL);
	Dbgout("%s\n",str);

	CalFollowScale();
	sprintf(str, "#69639=%.6f", m_dFollowScale);
	getCmdResponse(str, NULL);
	Dbgout("%s\n",str);

	getCmdResponse("#69637=1", NULL);
	Dbgout("#69637=1\n");

	Mutex.unlock();
	return true;
}

bool CKSMCA4::DisableMasterFollowing(int nIndex)
{
	Mutex.lock();
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 32; ii++ )
		{
			if( m_pMotor[ii] != NULL )
			{
				m_pMotor[ii]->DisableMtrMasterFollowing();
			}
		}
	}
	else if( nIndex <=0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	else
	{
		if( m_pMotor[nIndex-1] == NULL )
		{
			Mutex.unlock();
			return false;
		}
		m_pMotor[nIndex-1]->DisableMtrMasterFollowing();
	}

	int nMFMask = 0;
	for( int ii = 0; ii < 32; ii++ )
	{
		if( m_pMotor[ii] == NULL )
			continue;
		if( m_pMotor[ii]->GetMtrFollowing() )
		{
			nMFMask += 1<<ii;
		}
	}
	char str[32];
	sprintf(str, "#69643=%d", nMFMask);
	getCmdResponse(str, NULL);
	if( nMFMask == 0 )
		getCmdResponse("#69637=0", NULL);
	Mutex.unlock();
	return true;
}

bool CKSMCA4::MoveIncPTP(int nIndex, double dDist, int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}

	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrJogSpeed(m_dJogSpeed*m_dPulsePerUnit/60000);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);
	m_pMotor[nIndex-1]->JogStep(dDist*m_dPulsePerUnit);

	Mutex.unlock();
	return true;
}

bool CKSMCA4::MoveIncStepPTP(int nIndex, double dStepDist, int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}

	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrJogSpeed(m_dHMSpeedHigh*m_dPulsePerUnit/60000);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);
	m_pMotor[nIndex-1]->JogStep(dStepDist);

	Mutex.unlock();
	return true;
}

bool CKSMCA4::MoveAbsPTP(int nIndex, double dPos, int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrJogSpeed(m_dHMSpeedHigh*m_dPulsePerUnit/60000);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);
	m_pMotor[nIndex-1]->JogTo(dPos);
	Mutex.unlock();
	return true;

}

//对花旋转
bool CKSMCA4::MoveDirect(int nIndex,int nDirect, int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrJogSpeed(m_dJogSpeed*m_dPulsePerUnit/60000);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);
	if( (nDirect == 0))
		m_pMotor[nIndex-1]->JogPlus();
	else
		m_pMotor[nIndex-1]->JogMinus();
	Mutex.unlock();
	return true;
}



//对花高速旋转
bool CKSMCA4::MoveDirectHigh(int nIndex,int nDirect, int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrJogSpeed(m_dJogSpeedHigh*m_dPulsePerUnit/60000);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);
	//if( (nDirect == 0 && m_bPulseDirect == 0) || (nDirect==1 && m_bPulseDirect == 1))
	//胡志文 修改 调试方便
	if( (nDirect == 0))
		m_pMotor[nIndex-1]->JogPlus();
	else
		m_pMotor[nIndex-1]->JogMinus();
	Mutex.unlock();
	return true;
}


//反向旋转
bool CKSMCA4::MoveCCW(int nIndex, int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrCCWJogSpeed(m_dCCWJogSpeed*m_dPulsePerUnit/60000);
	//m_pMotor[nIndex-1]->SetMtrCCWJogSpeed(50);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);

	if( pubcom.ServoDir == 1 )
		m_pMotor[nIndex-1]->JogMinus();
	else
		m_pMotor[nIndex-1]->JogPlus();

	Mutex.unlock();
	return true;
}

//停止转动
bool CKSMCA4::MoveStop(int nIndex)
{
	Mutex.lock();
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 32; ii++ )
		{
			if( m_pMotor[ii] == NULL )
				continue;
			m_pMotor[ii]->JogStop();
		}
	}
	else if( nIndex > 0 && nIndex <= 32 )
	{
		if( m_pMotor[nIndex-1] == NULL )
		{
			Mutex.unlock();
			return false;
		}
		m_pMotor[nIndex-1]->JogStop();
	}
	else
	{
		Mutex.unlock();
		return false;
	}

	Mutex.unlock();
	return true;
}

bool CKSMCA4::SetHMSpeed(double dHMSpeed)
{
	m_dHMSpeed = dHMSpeed;
	return true;
}

//设置高速找零速度
bool CKSMCA4::SetHMSpeedHigh(double dHMSpeed)
{
	m_dHMSpeedHigh = dHMSpeed;
	return true;
}

bool CKSMCA4::SetJogSpeed(double dJogSpeed)
{
	m_dJogSpeed = dJogSpeed;
	return true;
}

bool CKSMCA4::SetJogSpeedHigh(double dJogSpeed)
{
	m_dJogSpeedHigh = dJogSpeed;
	return true;
}

bool CKSMCA4::SetCCWJogSpeed(double dCCWJogSpeed)
{
	m_dCCWJogSpeed = dCCWJogSpeed;
	return true;
}

//设置主编码器方向
bool CKSMCA4::SetPulseDirect(int nDirect)
{
	if( nDirect == 0 )
	{
		m_bPulseDirect = 0;
		getCmdResponse("#79930=3", NULL);
	}
	else
	{
		m_bPulseDirect = 1;
		getCmdResponse("#79930=7", NULL);
	}

	return true;
}
//橡毯每米主编码器脉冲数
bool CKSMCA4::SetPulserPerMeter(int nPulserPerMeter)
{
	m_nPulserPerMeter = nPulserPerMeter;
	CalFollowScale();

	char str[32];
	sprintf(str, "#100500=%d", m_nPulserPerMeter);
	getCmdResponse(str, NULL);

	return true;
}

//最大速度，起保护作用
bool CKSMCA4::SetMaxSpeed(double dMaxSpeed)
{
	Mutex.lock();
	m_dMaxSpeed = dMaxSpeed;
	for( int ii = 0; ii < 32; ii++ )
	{
		if( m_pMotor[ii] == NULL )
			continue;
		m_pMotor[ii]->SetMtrMaxSpeed(dMaxSpeed*m_dPulsePerUnit/60000);
	}
	Mutex.unlock();
	return true;
}

//设置网头的安装偏差值
bool CKSMCA4::SetMotorOffset(int nIndex,double dOffset)
{
	Mutex.lock();
	bool ret = false;
	if( nIndex > 0 && nIndex <= 32 )
	{
		if( m_pMotor[nIndex-1] == NULL )
		{
			Mutex.unlock();
			return false;
		}
		ret = m_pMotor[nIndex-1]->SetOffset(dOffset);
		Mutex.unlock();
		return 	ret;
	}
	else
	{
		Mutex.unlock();
		return false;
	}
}

//取得网头的安装偏差值
bool CKSMCA4::GetMotorOffset(int nIndex,double& dOffset)
{
	Mutex.lock();
	bool ret = false;
	if( nIndex > 0 && nIndex <= 32 )
	{
		if( m_pMotor[nIndex-1] == NULL )
		{
			Mutex.unlock();
			return false;
		}
		ret = m_pMotor[nIndex-1]->GetOffset(dOffset);
		Mutex.unlock();
		return ret;
	}
	else
	{
		Mutex.unlock();
		return false;
	}
}

//设置每个网头的理论间距
bool CKSMCA4::SetOffset(double dOffset)
{
	m_dOffset = dOffset;
	return true;
}

bool CKSMCA4::SetToothNum1(int nToothNum1)
{
	m_nToothNum1 = nToothNum1;
	CalPulsePerUnit();
	return true;
}

//设置伺服变速箱速比
bool CKSMCA4::SetServGearBoxScale(double dScale)
{
	m_dServGearBoxScale = dScale;
	CalPulsePerUnit();
	return true;
}

bool CKSMCA4::SetToothNum2(int nToothNum2)
{
	m_nToothNum2 = nToothNum2;
	CalPulsePerUnit();
	return true;
}

//设置圆网的周长
bool CKSMCA4::SetGirth(double nGirth)
{
	m_nGirth = nGirth;
	CalPulsePerUnit();
	return true;
}
//伺服电机每转，伺服电机上的编码器脉冲数
bool CKSMCA4::SetPulsePerCircle(int nPulsePerCircle)
{
	m_nPulsePerCircle = nPulsePerCircle;
	CalPulsePerUnit();
	return true;
}

//在主循环中调用，取得板子和伺服的故障
int CKSMCA4::CheckKSMCA4()
{
	Mutex.lock();
	double dCode;

	memset(m_stError, 0, sizeof( struct ErrorInfo)*10 );
	m_nErrNum = 0;

	getMacroVar(69847, &dCode);

	//0位置卡板故障
	if( dCode != 0 )
	{
		SetErrorInfo(0, (int)dCode);
	}

	// 1-32伺服的故障
	for( int ii = 0; ii < 32; ii++ )
	{
		//记录是否有故障
		bool hascaution = false;

		if( m_pMotor[ii] == NULL )
			continue;
		//check Motor && Amp error
		getMacroVar(66408+ii*100, &dCode);
		if( dCode != 0 )
		{
			int status;
			m_pMotor[ii]->GetStatus(status);
			if(status != 5)
			{
				m_pMotor[ii]->SetCaution(true);
			}
			hascaution = true;
			if( SetErrorInfo(ii+1, (int)dCode) == false )
				break;
		}
		getMacroVar(78009+ii*50, &dCode);
		if( dCode != 0 )
		{
			int status;
			m_pMotor[ii]->GetStatus(status);
			if(status != 5)
			{
				m_pMotor[ii]->SetCaution(true);
			}
			hascaution = true;
			if( SetErrorInfo(ii+1, (int)dCode) == false )
				break;
		}
		int status;
		m_pMotor[ii]->GetStatus(status);

		//消除故障
		if(hascaution == false
				&& status == 5)
		{
			m_pMotor[ii]->SetCaution(false);
		}
	}

	Mutex.unlock();
	return m_nErrNum;
}

//取得故障的位置
bool CKSMCA4::GetErrorLocation(int nIndex, int &nLocation)
{
	nLocation = 0;
	if( m_nErrNum == 0 || nIndex <=0 || nIndex > m_nErrNum )
		return false;
	nLocation = m_stError[nIndex-1].nLocation;
	return true;
}

//取得故障码
bool CKSMCA4::GetErrorCode(int nIndex, int &nCode)
{
	nCode = 0;
	if( m_nErrNum == 0 || nIndex <=0 || nIndex > m_nErrNum )
		return false;
	nCode = m_stError[nIndex-1].nCode;
	return true;
}

bool CKSMCA4::SetErrorInfo(int nLocation, int nCode)
{
	if( m_nErrNum >= 10 )
		return false;
	m_stError[m_nErrNum].nLocation = nLocation;
	m_stError[m_nErrNum].nCode = nCode;

	m_nErrNum++;
	return true;
}

bool CKSMCA4::SetFollowDelay(double dDelay)
{
	m_dDelay = dDelay;
	CalFollowScale();
	return true;
}

//IO输出
bool CKSMCA4::SetOutPut(int nIndex, int nVal)
{
	char str[32];
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 16; ii++ )
		{
			sprintf(str, "#%d=%d", 79900+ii, nVal);
			getCmdResponse(str, NULL);
		}

	}
	else if( nIndex > 0 && nIndex <= 16)
	{
		sprintf(str, "#%d=%d", 79900+(nIndex-1), nVal);
		getCmdResponse(str, NULL);
	}
	else
	{
		return false;
	}
	return true;
}

//IO输入
bool CKSMCA4::GetInPut(int nIndex, int &nVal)
{
	double dd = 0.0;

	if( nIndex <= 0 || nIndex > 16)
		return false;

	getMacroVar(79955+nIndex-1, &dd);
	nVal = (int)dd;
	return true;
}

bool CKSMCA4::Home(int nIndex)
{
	Mutex.lock();
	if( nIndex == -1 )
	{
		for( int ii = 0; ii < 32; ii++ )
		{
			if( m_pMotor[ii] == NULL )
				continue;
			m_pMotor[ii]->SetMtrHMSpeed(m_dHMSpeed*m_dPulsePerUnit/60000);
		}
	}
	else if( nIndex > 0 && nIndex <= 32 )
	{
		if( m_pMotor[nIndex-1] == NULL )
		{
			Mutex.unlock();
			return false;
		}

		m_pMotor[nIndex-1]->SetMtrHMSpeed(m_dHMSpeed*m_dPulsePerUnit/60000);
	}
	else
	{
		Mutex.unlock();
		return false;
	}
    Mutex.unlock();
	return true;
}


//作找零旋转
bool CKSMCA4::MoveHome(int nIndex,int nDirect,int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrJogSpeed(m_dHMSpeed*m_dPulsePerUnit/60000);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);

	if( (nDirect == 0))
		m_pMotor[nIndex-1]->JogPlus();
	else
		m_pMotor[nIndex-1]->JogMinus();
	Mutex.unlock();
	return true;
}

//作找零旋转，高速
bool CKSMCA4::MoveHomeHigh(int nIndex,int nDirect,int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrJogSpeed(m_dHMSpeedHigh*m_dPulsePerUnit/60000);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);

	if( (nDirect == 0))
		m_pMotor[nIndex-1]->JogPlus();
	else
		m_pMotor[nIndex-1]->JogMinus();
	Mutex.unlock();
	return true;
}

//当前伺服电机编码器位置
bool CKSMCA4::GetLocation(int nIndex,double& loc)
{
	Mutex.lock();
	double dd = 0.0;
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->GetLocation(dd);

	loc = dd;
	Mutex.unlock();
	return true;
}

//取得主编码器位置
bool CKSMCA4::GetEncoderCount(double& count)
{
	double dd= 0.0;
	getMacroVar(79952, &dd);
	count = dd;
	return true;
}

//每毫米脉冲
bool CKSMCA4::CalPulsePerUnit()
{
	if( m_nGirth <= 0 || m_nPulsePerCircle <= 0 || m_nToothNum1 <= 0 || m_nToothNum2 <= 0 )
	{
		m_dPulsePerUnit = 0.0;
		return false;
	}
	m_dPulsePerUnit = m_nPulsePerCircle * m_dServGearBoxScale;
	m_dPulsePerUnit *= m_nToothNum2;
	m_dPulsePerUnit /= m_nToothNum1;
	m_dPulsePerUnit /= m_nGirth;

	Dbgout("m_dPulsePerUnit:%f\n",m_dPulsePerUnit);

	CalFollowScale();

	m_nPulsePerBigCircle = m_nPulsePerCircle * m_dServGearBoxScale;
	m_nPulsePerBigCircle *= m_nToothNum2;
	m_nPulsePerBigCircle /= m_nToothNum1;
	Dbgout("m_nPulsePerBigCircle:%d\n",m_nPulsePerBigCircle);

	return true;
}

//跟随比率计算
bool CKSMCA4::CalFollowScale()
{
	if( m_dPulsePerUnit <= 0.0 || m_nPulserPerMeter <= 0.0 )
	{
		m_dFollowScale = 0.0;
	}

	m_dFollowScale = m_dPulsePerUnit*1000/m_nPulserPerMeter*32;
	m_dFollowScale *= (1+m_dDelay);
	char str[32];
	sprintf(str, "#69639=%.6f", m_dFollowScale);
	getCmdResponse(str, NULL);

	return true;
}

bool CKSMCA4::CheckKSMCReady()
{
	Mutex.lock();
	int ii = 0;
	double dd = 0.0;
	getMacroVar(69639, &dd);
	if( (fabs(dd - m_dFollowScale) > 0.000001))
	{
		Mutex.unlock();
		return false;
	}

	getMacroVar(69637, &dd);
	if( dd == 0 )
	{
		Mutex.unlock();
		return false;
	}

	int nMFMask = 0;
	for( ii = 0; ii < 32; ii++ )
	{
		if( m_pMotor[ii] == NULL )
			continue;
		if( m_pMotor[ii]->GetMtrFollowing() )
		{
			nMFMask += 1<<ii;
		}
	}
	getMacroVar(69643, &dd);
	if( dd != nMFMask )
	{
		Mutex.unlock();
		return false;
	}

	for( ii = 0; ii < 32; ii++ )
	{
		if( m_pMotor[ii] == NULL )
			continue;

		if( !m_pMotor[ii]->IsReady() )
		{
			Mutex.unlock();
			return false;
		}
	}
	Mutex.unlock();
	return true;
}

//设置跟随过滤时间，越小，刚性越好，但是震荡比较厉害
bool CKSMCA4::SetFollowFilter(double dFollowFilter)
{
	char str[32];
	sprintf(str, "#69636=%.6f", dFollowFilter);
	getCmdResponse(str, NULL);

	return true;
}

//取得圆网转速，100504用户自定义宏，在PLC程序中定义的
bool CKSMCA4::GetMainMotorSpeed(double &Speed)
{
	double dd = 0;
	getMacroVar(100504, &dd);
	Speed = dd;
	return true;
}

//取得伺服电机的转速
bool CKSMCA4::GetMotorSpeed(int nIndex,double &Speed)
{
	if (nIndex > 16 
		|| nIndex  < 1)
	{
		return false;
	}
	else
	{
		double dd = 0;
		getMacroVar(66465 + (nIndex -1)*100, &dd);
		Speed = dd;
		return true;
	}
}

//启动PLC
bool CKSMCA4::StartPLC()
{
	char str[32];
	sprintf(str, "#100500=%d", m_nPulserPerMeter);
	getCmdResponse(str, NULL);
	//PLC
	getCmdResponse("ena p2", NULL);	//start the plc
    return true;
}
	
//停止PLC
bool CKSMCA4::StopPLC()
{
	//PLC
	getCmdResponse("Dis p2", NULL);	//stop the plc
	return true;

}

//取得大圈的脉冲数
bool CKSMCA4::GetPulsePerBigCircle(int& nPulsePerBigCircle)
{
	nPulsePerBigCircle = m_nPulsePerBigCircle;
	return true;
}

//取得网头间距，mm
bool CKSMCA4::GetOffset(double& dOffset)
{
	dOffset = m_dOffset;
	return true;
}

//取圆网周长
bool CKSMCA4::GetGirth(double& nGirth)
{
	nGirth = m_nGirth;
	return true;
}

//取得单位长度的脉冲数
bool CKSMCA4::GetPulsePerUnit(double& dPulsePerUnit)
{
	dPulsePerUnit = m_dPulsePerUnit;
	return true;
}

//取得每米的脉冲数
bool CKSMCA4::GetPulsePerMeter(unsigned int& nNumber)
{
	nNumber = m_nPulserPerMeter;
	return true;
}


 //取得网头电机是否到位信息
bool CKSMCA4::GetMotorInPosStatus(int nIndex,int& Status)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}

	double dd = 0;
	getMacroVar(66406+100*(nIndex - 1),&dd);
	Status = (int)dd;

	Mutex.unlock();
	return true;
}

//设置驱动器参数，nIndex:驱动器编号 parIndex：参数编号 parVal：参数值
bool CKSMCA4::SetDriverPar(int nIndex,int parIndex,int parVal)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}

	char str[32];
	memset(str,0,32);
	sprintf(str,"#%d=%d", 73000+(nIndex-1)*150 + parIndex,parVal);
	Dbgout("%s\n", str);
	getCmdResponse(str, NULL);

	Mutex.unlock();
	return true;
}

//读取驱动器参数
bool CKSMCA4::GetDriverPar(int nIndex,int parIndex,int &ParVal)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}

	double dd = 0;
	getMacroVar(73000+150*(nIndex - 1) + parIndex,&dd);
	ParVal = (int)dd;

	Mutex.unlock();
	return true;
}

//记录网头电机的位置
bool CKSMCA4::RememberPrintLoction(int nIndex)                         
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}

	m_pMotor[nIndex - 1]->GetLocation(m_dPrintLocation[nIndex - 1]);

	Mutex.unlock();
	return true;

}

//设置网头电机的位置
bool CKSMCA4::SetPrintLocation(int nIndex,double location)           
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_dPrintLocation[nIndex - 1] = location;

	Mutex.unlock();
	return true;

}


//恢复网头电机的位置
bool CKSMCA4::RestorePrintLocation(int nIndex)                          
{
	if( nIndex <= 0 || nIndex > 32 )
	{
		return false;
	}
	if( m_pMotor[nIndex-1] == NULL )
	{
		return false;
	}

	//如果网头没有对零，初始值的情况下面
	if(m_dPrintLocation[nIndex -1] == -1)
	{
		return false;
	}

	//计算记录的位置和当前位置的差值
	double CurrentLoc = 0;
	m_pMotor[nIndex - 1]->GetLocation(CurrentLoc);
	
	if (CurrentLoc > m_dPrintLocation[nIndex-1])
	{
		double distance = CurrentLoc - m_dPrintLocation[nIndex-1];

		//取整
		distance = (double)((((long)(distance*10)) % ((long)(m_nPulsePerBigCircle*10)))/10);
		//反转
		MoveIncStepPTP(nIndex,0-distance);
		//printf("反转%f\n",0-distance);
	}
	else
	{
		double distance = m_dPrintLocation[nIndex-1] - CurrentLoc;

		//取整
		distance = (double)((((long)(distance*10)) % ((long)(m_nPulsePerBigCircle*10)))/10);
		//正转
		MoveIncStepPTP(nIndex,distance);
		//printf("正转%f\n",distance);

	}

	return true;

}

//测试函数
bool CKSMCA4::test(int nIndex, double CirNum, int nTA, int nTS)
{
	Mutex.lock();
	if( nIndex <= 0 || nIndex > 32 )
	{
		Mutex.unlock();
		return false;
	}

	if( m_pMotor[nIndex-1] == NULL )
	{
		Mutex.unlock();
		return false;
	}
	m_pMotor[nIndex-1]->SetMtrJogSpeed(10000*m_dPulsePerUnit/60000);
	m_pMotor[nIndex-1]->SetMtrJogTA(nTA);
	m_pMotor[nIndex-1]->SetMtrJogTS(nTS);
	//反转可以保持对零标识一致
	m_pMotor[nIndex-1]->JogStep(-(CirNum*m_nPulsePerBigCircle));

	Mutex.unlock();
	return true;
}

//设置网头1的安装偏差
bool   CKSMCA4::SetMotor1Offset(double Offset)
{
	m_dMotor1Offset = Offset;
	return true;
}

//设置网头1的安装偏差
bool   CKSMCA4::GetMotor1Offset(double& Offset)
{
	Offset = m_dMotor1Offset;
	return true;
}
