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
	bool    SetOffset(double dOffset);//������ͷ��װλ��ƫ��
	bool    GetOffset(double& dOffset);//ȡ����ͷ��װλ��ƫ��

	bool	SetMtrJogTA(int nTA);
	bool	SetMtrJogTS(int nTS);
	bool	JogStep(double dDist);
	bool	JogTo(double dPos);
	bool	JogPlus();
	bool	JogMinus();
	bool	JogStop();

	bool	IsReady();
	bool	SetCaution(bool bl);//���ù���״̬
	bool 	GetStatus(int& Status);
	bool    GetLocation(double& loction);//ȡ�õ���ĵ�ǰλ��

private:
	int		m_nMtrIndex;
	int 	m_nStatus;//0: ��ʼ��״̬     1�� ����  2�� ����  3�� ��ת 4������  5�� ����
	double     m_dOffset;//��ͷ�����۰�װλ�õ�ƫ�ƣ�mm
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
	bool	AddMotor(int nIndex);				//׷��һ�����
	bool	DelMotor(int nIndex=-1);			//ɾ��һ����Ĭ��ɾ������
	bool	InitMotor(int nIndex=-1);			//��ʼ��һ����Ĭ�ϳ�ʼ������
	bool	SetActiveMotor(int nIndex=-1);		//���ջ�
	bool	SetOffMotor(int nIndex=-1);			//Kill ���

	//���״̬ת������
	bool	EnableMasterFollowing(int nIndex=-1);	//ʹ�ܸ����˶�
	bool	DisableMasterFollowing(int nIndex=-1);	//�رո����˶�
	bool	Home(int nIndex=-1);					//������
	bool	MoveHome(int nIndex=-1,int nDirect=0, int nTA=100, int nTS=100);		//��������ת
	bool	MoveHomeHigh(int nIndex=-1,int nDirect=0, int nTA=100, int nTS=200);	//��������ת,����
    bool	MoveIncPTP(int nIndex, double dDist, int nTA=100, int nTS=100);		//����΢��
	bool    MoveIncStepPTP(int nIndex, double dStepDist, int nTA=100, int nTS=100);      //����΢��step
	bool	MoveAbsPTP(int nIndex, double dPos, int nTA=100, int nTS=200);		//�ƶ���ĳһ��λ��
	bool	MoveDirect(int nIndex, int nDirect=0, int nTA=100, int nTS=50);		//���������������ƶ�
	bool	MoveDirectHigh(int nIndex, int nDirect=0, int nTA=100, int nTS=50);	//���������������ƶ�
	bool	MoveCCW(int nIndex, int nTA=100, int nTS=50);						//��ת
	bool	MoveStop(int nIndex=-1);											//����ƶ�ֹͣ
	bool 	GetStatus(int nIndex,int& Status);
	bool    GetLocation(int nIndex,double& loc);
	bool    StartPLC();
	bool    StopPLC();

	//parameters
	bool	SetHMSpeed(double dHMSpeed);				//���������ٶ�
	bool    SetHMSpeedHigh(double dHMSpeed);			//���ø��������ٶ�
	bool	SetJogSpeed(double dJogSpeed);				//����΢���������˶��ٶȣ����ڶԻ�
	bool	SetJogSpeedHigh(double dJogSpeed);			//����΢���������˶��ٶȣ����ڶԻ�
	bool	SetCCWJogSpeed(double dCCWJogSpeed);		//���÷�ת�ٶ�
	bool	SetPulseDirect(int nDirect);				//���á���������ѡ������
	bool	SetPulserPerMeter(int nPulserPerMeter);		//��̺��ÿ��������
	bool	SetMaxSpeed(double dMaxSpeed);				//��������ٶ�
	bool	SetOffset(double dOffset);					//��ͷƫ����
	bool	SetToothNum1(int nToothNum1);				//С���ֳ���
	bool	SetToothNum2(int nToothNum2);				//����ֳ���
	bool	SetGirth(double nGirth);						//Բ���ܳ�
	bool	SetPulsePerCircle(int nPulsePerCircle);		//���һȦ�����������ֱ���
	bool	SetFollowDelay(double dDelay);				//�����ͺ���
	bool	SetOutPut(int nIndex=-1, int nVal=0);		//IO�����
	bool	GetInPut(int nIndex, int &nVal);			//IO�����
	bool    SetFollowFilter(double dFollowFilter);      //���ø������ʱ��
	bool    SetServGearBoxScale(double dScale);         //�����ŷ��������ٱ�
	bool    SetMotor1Offset(double Offset);             //������ͷ1�İ�װƫ��


	//check error and get error infomation
	int		CheckKSMCA4();
	bool	GetErrorLocation(int nIndex, int &nLocation);
	bool	GetErrorCode(int nIndex, int &nCode);

	bool	CheckKSMCReady();
	bool	GetEncoderCount(double& count);      //ȡ����������λ��

	bool GetMainMotorSpeed(double &Speed);       //ȡ��������ٶȣ�1s���ڼ���
	bool GetMotorSpeed(int nIndex,double &Speed);
	bool GetPulsePerBigCircle(int& nPulsePerBigCircle);  //ȡ�ô�Ȧ��������
	bool GetOffset(double& dOffset);                    //ȡ����ͷ��࣬mm
	bool GetGirth(double& nGirth);						     //ȡԲ���ܳ�
	bool GetPulsePerUnit(double& dPulsePerUnit);         //ȡ�õ�λ���ȵ�������
	bool GetPulsePerMeter(unsigned int& nNumber);		//ȡ��ÿ�׵�������
	bool GetMotorOffset(int nIndex,double& nOffset);		// ȡ����ͷ�İ�װƫ��ֵ
	bool SetMotorOffset(int nIndex,double dOffset);		// ������ͷ�İ�װƫ��ֵ
	bool GetMotorInPosStatus(int nIndex,int& Status);    //ȡ����ͷ�Ƿ�λ��Ϣ
	bool GetMotor1Offset(double& Offset);               //ȡ����ͷ1�İ�װƫ��
	bool RememberPrintLoction(int nIndex);                          //��¼��ͷ�����λ�ã�ͨ����ȡ�˶���
	bool RestorePrintLocation(int nIndex);                          //�ָ���ͷ�����λ��
	bool SetPrintLocation(int nIndex,double location);           //������ͷ�����λ��


	//���Ժ���
	bool test(int nIndex, double CirNum, int nTA, int nTS);

	//����������������nIndex:��������� parIndex��������� parVal������ֵ
	bool SetDriverPar(int nIndex,int parIndex,int parVal);

	//��ȡ����������
	bool GetDriverPar(int nIndex,int parIndex,int &ParVal);

private:
	CMotor	*m_pMotor[32];
	int     m_nInstallOffset[32];       //��ͷ��װλ�õ�ƫ��
	//parameters from UserSetting
	double	m_dHMSpeed;					//�����ٶ�
	double	m_dHMSpeedHigh;				//���ٶ����ٶ�
	double	m_dJogSpeed;				//�Ի��ٶ�
	double	m_dJogSpeedHigh;			//���ٶԻ��ٶ�
	double	m_dCCWJogSpeed;				//��ת�ٶ�
	int		m_bPulseDirect;				//������������
	int		m_nPulserPerMeter;			//ĥ��ϵ��
	double	m_dMaxSpeed;				//��������ٶ�
	double	m_dOffset;					//԰�����
	int		m_nToothNum1;				//С���ֳ���
	int		m_nToothNum2;				//����ֳ���
	double		m_nGirth;					//԰���ܳ�
	int		m_nPulsePerCircle;			//����ֱ���
	double  m_nPulsePerBigCircle;       //Բ��תһȦ������

	double	m_dPulsePerUnit;			//ÿ����������

	double	m_dFollowScale;				//�������

	double	m_dDelay;					//�����ͺ����
	double  m_dServGearBoxScale;        //�ŷ��������ٱ�
	double  m_dMotor1Offset;            //��ͷ1�İ�װƫ��

	double     m_dPrintLocation[32];    //��ͷ�����λ��

	//��ǰ�Ĺ��ϵ���Ŀ
	int		m_nErrNum;
	struct ErrorInfo	m_stError[10];

	bool	SetErrorInfo(int nLocation, int nCode);
	bool	CalPulsePerUnit();
	bool	CalFollowScale();

	//���˶��������Ҫ��ռ
	QMutex Mutex;
};


#endif
