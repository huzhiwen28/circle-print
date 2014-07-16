#ifndef __C_MOTOR_KSMCA4__
#define __C_MOTOR_KSMCA4__

//#define NULL	0
#include <QMutex>

class CMotor{

public:
	CMotor(int nIndex);
	~CMotor();

	bool	InitPara();
	bool	SetMtrActive();
	bool	SetMtrOff();
	bool	SetMtrHMSpeed(double dHMSpeed);
	bool	SetMtrJogSpeed(double dJogSpeed);
	bool	SetMtrCCWJogSpeed(double dCCWJogSpeed);
	bool	SetMtrMaxSpeed(double dMaxSpeed);
	bool	EnableMtrMasterFollowing();
	bool	DisableMtrMasterFollowing();
	bool	GetMtrFollowing();
	bool    SetOffset(double dOffset);//设置网头安装位置偏移
	bool    GetOffset(double& dOffset);//取得网头安装位置偏移

	bool	SetMtrJogTA(int nTA);
	bool	SetMtrJogTS(int nTS);
	bool	JogStep(double dDist);
	bool	JogTo(double dPos);
	bool	JogPlus();
	bool	JogMinus();
	bool	JogStop();

	bool	IsReady();
	bool	SetCaution(bool bl);//设置故障状态
	bool 	GetStatus(int& Status);
	bool    GetLocation(double& loction);//取得电机的当前位置

private:
	int		m_nMtrIndex;
	int 	m_nStatus;//0: 初始化状态     1： 就绪  2： 跟随  3： 反转 4：保留  5： 出错
	double     m_dOffset;//网头和理论安装位置的偏移，mm
};

struct ErrorInfo{
	int nLocation;
	int nCode;
};


class CKSMCA4{

public:
	CKSMCA4();					//
	~CKSMCA4();					//

	//Controller
	bool	AddMotor(int nIndex);				//追加一个马达
	bool	DelMotor(int nIndex=-1);			//删除一个马达，默认删除所有
	bool	InitMotor(int nIndex=-1);			//初始化一个马达，默认初始化所有
	bool	SetActiveMotor(int nIndex=-1);		//马达闭环
	bool	SetOffMotor(int nIndex=-1);			//Kill 马达

	//马达状态转换命令
	bool	EnableMasterFollowing(int nIndex=-1);	//使能跟随运动
	bool	DisableMasterFollowing(int nIndex=-1);	//关闭跟随运动
	bool	Home(int nIndex=-1);					//作找零
	bool	MoveHome(int nIndex=-1,int nDirect=0, int nTA=100, int nTS=100);		//作找零旋转
	bool	MoveHomeHigh(int nIndex=-1,int nDirect=0, int nTA=100, int nTS=200);	//作找零旋转,高速
    bool	MoveIncPTP(int nIndex, double dDist, int nTA=100, int nTS=100);		//步进微动
	bool    MoveIncStepPTP(int nIndex, double dStepDist, int nTA=100, int nTS=100);      //步进微动step
	bool	MoveAbsPTP(int nIndex, double dPos, int nTA=100, int nTS=200);		//移动到某一个位置
	bool	MoveDirect(int nIndex, int nDirect=0, int nTA=100, int nTS=50);		//持续超正、负向移动
	bool	MoveDirectHigh(int nIndex, int nDirect=0, int nTA=100, int nTS=50);	//持续超正、负向移动
	bool	MoveCCW(int nIndex, int nTA=100, int nTS=50);						//反转
	bool	MoveStop(int nIndex=-1);											//马达移动停止
	bool 	GetStatus(int nIndex,int& Status);
	bool    GetLocation(int nIndex,double& loc);
	bool    StartPLC();
	bool    StopPLC();

	//parameters
	bool	SetHMSpeed(double dHMSpeed);				//设置找零速度
	bool    SetHMSpeedHigh(double dHMSpeed);			//设置高速找零速度
	bool	SetJogSpeed(double dJogSpeed);				//设置微动、连续运动速度，用于对花
	bool	SetJogSpeedHigh(double dJogSpeed);			//设置微动、连续运动速度，用于对花
	bool	SetCCWJogSpeed(double dCCWJogSpeed);		//设置反转速度
	bool	SetPulseDirect(int nDirect);				//设置“主编码器选正负”
	bool	SetPulserPerMeter(int nPulserPerMeter);		//橡毯的每米脉冲数
	bool	SetMaxSpeed(double dMaxSpeed);				//整机最大速度
	bool	SetOffset(double dOffset);					//网头偏移量
	bool	SetToothNum1(int nToothNum1);				//小齿轮齿数
	bool	SetToothNum2(int nToothNum2);				//大齿轮齿数
	bool	SetGirth(double nGirth);						//圆网周长
	bool	SetPulsePerCircle(int nPulsePerCircle);		//电机一圈的脉冲数、分辨率
	bool	SetFollowDelay(double dDelay);				//跟随滞后量
	bool	SetOutPut(int nIndex=-1, int nVal=0);		//IO输出点
	bool	GetInPut(int nIndex, int &nVal);			//IO输入点
	bool    SetFollowFilter(double dFollowFilter);      //设置跟随过滤时间
	bool    SetServGearBoxScale(double dScale);         //设置伺服变速箱速比
	bool    SetMotor1Offset(double Offset);             //设置网头1的安装偏差


	//check error and get error infomation
	int		CheckKSMCA4();
	bool	GetErrorLocation(int nIndex, int &nLocation);
	bool	GetErrorCode(int nIndex, int &nCode);

	bool	CheckKSMCReady();
	bool	GetEncoderCount(double& count);      //取得主编码器位置

	bool GetMainMotorSpeed(double &Speed);       //取得主电机速度，1s周期计算
	bool GetMotorSpeed(int nIndex,double &Speed);
	bool GetPulsePerBigCircle(int& nPulsePerBigCircle);  //取得大圈的脉冲数
	bool GetOffset(double& dOffset);                    //取得网头间距，mm
	bool GetGirth(double& nGirth);						     //取圆网周长
	bool GetPulsePerUnit(double& dPulsePerUnit);         //取得单位长度的脉冲数
	bool GetPulsePerMeter(unsigned int& nNumber);		//取得每米的脉冲数
	bool GetMotorOffset(int nIndex,double& nOffset);		// 取得网头的安装偏差值
	bool SetMotorOffset(int nIndex,double dOffset);		// 设置网头的安装偏差值
	bool GetMotorInPosStatus(int nIndex,int& Status);    //取得网头是否到位信息
	bool GetMotor1Offset(double& Offset);               //取得网头1的安装偏差
	bool RememberPrintLoction(int nIndex);                          //记录网头电机的位置，通过读取运动卡
	bool RestorePrintLocation(int nIndex);                          //恢复网头电机的位置
	bool SetPrintLocation(int nIndex,double location);           //设置网头电机的位置


	//测试函数
	bool test(int nIndex, double CirNum, int nTA, int nTS);

	//设置驱动器参数，nIndex:驱动器编号 parIndex：参数编号 parVal：参数值
	bool SetDriverPar(int nIndex,int parIndex,int parVal);

	//读取驱动器参数
	bool GetDriverPar(int nIndex,int parIndex,int &ParVal);

private:
	CMotor	*m_pMotor[32];
	int     m_nInstallOffset[32];       //网头安装位置的偏差
	//parameters from UserSetting
	double	m_dHMSpeed;					//对零速度
	double	m_dHMSpeedHigh;				//高速对零速度
	double	m_dJogSpeed;				//对花速度
	double	m_dJogSpeedHigh;			//高速对花速度
	double	m_dCCWJogSpeed;				//反转速度
	int		m_bPulseDirect;				//主编码器方向
	int		m_nPulserPerMeter;			//磨擦系数
	double	m_dMaxSpeed;				//整机最大速度
	double	m_dOffset;					//园网间距
	int		m_nToothNum1;				//小齿轮齿数
	int		m_nToothNum2;				//大齿轮齿数
	double		m_nGirth;					//园网周长
	int		m_nPulsePerCircle;			//电机分辨率
	double  m_nPulsePerBigCircle;       //圆网转一圈的脉冲

	double	m_dPulsePerUnit;			//每毫米脉冲数

	double	m_dFollowScale;				//跟随比例

	double	m_dDelay;					//跟随滞后比例
	double  m_dServGearBoxScale;        //伺服变速箱速比
	double  m_dMotor1Offset;            //网头1的安装偏差

	double     m_dPrintLocation[32];    //网头电机的位置

	//当前的故障的数目
	int		m_nErrNum;
	struct ErrorInfo	m_stError[10];

	bool	SetErrorInfo(int nLocation, int nCode);
	bool	CalPulsePerUnit();
	bool	CalFollowScale();

	//对运动卡板操作要独占
	QMutex Mutex;
};


#endif
