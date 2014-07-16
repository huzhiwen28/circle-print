/*
* canif.h
*
*  Created on: 2009-8-3
*      Author: Owner
*/

#ifndef CANIF_H_
#define CANIF_H_

#include "canidcmd.h"
#include <Windows.h>
#include "ControlCan.h"
#include <QList>
#include <QLinkedList>
#include <QMap>
#include "set.h"
#include <QReadWriteLock>



//״̬���ڲ��¼�
typedef struct _intevent{
	int nodeid;//����ID
	int eventid;//�¼�ID 01����������  02����������
} tintevent;


//״̬���ڲ��¼�
class ta4ncaution{
public:
	int location;//λ��
	int errcode;//������
} ;

inline bool operator==(const ta4ncaution &e1, const ta4ncaution &e2)
{
	if(e1.errcode == e2.errcode
		&& e1.location == e2.location)
	{
		return true;
	}
	else
	{
		return false;
	}
}



typedef struct
{
	unsigned char cmd; //����
	unsigned char id; //���Ͱ��ӵ�id
	unsigned char para1; //����1
	unsigned char para2; //����2
	unsigned int  timecnt;//ʱ���
} tcmd;


//�������߲������÷�������
typedef struct
{
	//����ID
	int canid;

	//����������
	tcmd cmd;

	//�Ƿ��ͳɹ�
	bool sendsucceed;
} tbdonlineparasendcmd;


//�������߲������ý�������
typedef struct
{
	//�����յ�����
	tcmd cmd;

	//�Ƿ���ճɹ�
	bool recvsucceed;
} tbdonlinepararecvcmd;

//�������߲������������
typedef struct  
{
	tbdonlineparasendcmd sendcmd;
	tbdonlinepararecvcmd recvcmd;
}tbdonlineparacmdpair;

//�ָ�Բ��λ������
struct restore{
	int id;//��ͷid
	int timer;//�ȴ�ʱ��200ms���ȵ����������
	int step;//����ӿ� 1��ʼ 0�������
};

//����Բ��λ������
struct saveloc{
	int id;//��ͷid
	int timer;//�ȴ�ʱ��200ms���ȵ����������
	int step;//����ӿ� 1��ʼ 0�������
};

//CAN�ӿڿ�
//CAN�豸��ʹ���߼����á�������-�����ߡ�ģʽ�������ǲ���CAN�����ģʽ
class canif
{

public:
	canif();
	virtual ~canif();

	//�ɹ��򿪺ͳ�ʼ��
	bool succeedopenandinit;

	//�򿪲�ʹ��
	bool open();

	//�ر��豸
	bool close();

	//���豸�ڴ�ȡ���ݵ�����
	bool refresh();

	//ˢ�»����е�ʱ������������������ʱ�Ļ�����ɾ������
	void refreshtime();


	//����һ������,���͵Ķ�����fid
	bool sendcmd(unsigned long fid,tcmd* cmd);

	//�鿴�������Ƿ�������id��������,����еĻ����Ͱ�����ȡ��cmd��ȥ
	bool getidcmd(unsigned char id,tcmd* cmd);

	//�鿴�������Ƿ��а���id�� cmdid�������еĻ�����ȡ��cmd��ȥ
	bool getcmd(unsigned char id,unsigned char cmdid,tcmd* cmd);

	//�鿴�������Ƿ��а���id�� cmdid�������Ϊpara1,����еĻ�����ȡ��cmd��ȥ
	bool getcmd(unsigned char id,unsigned char cmdid,unsigned char para1,tcmd* cmd);

	//�鿴�������Ƿ��а���id�� cmdid�������Ϊpara1,para2,����еĻ�����ȡ��cmd��ȥ
	bool getcmd(unsigned char id,unsigned char cmdid,unsigned char para1,unsigned char para2,tcmd* cmd);

	//�鿴�������Ƿ��а���id�� cmdid�������Ϊpara1,����еĻ�����ȡ��cmdlist��ȥ,����������
	int getcmd(unsigned char id,unsigned char cmdid,unsigned char para1,QList<tcmd>& cmdlist);

	//�鿴�������Ƿ��а���id�� cmdid�������Ϊpara1,para2 ����еĻ�����ȡ��cmdlist��ȥ,����������
	int getcmd(unsigned char id,unsigned char cmdid,unsigned char para1,unsigned char para2,QList<tcmd>& cmdlist);

	//�鿴�������Ƿ��а���id�� cmdid����,����еĻ�����ȡ��cmdlist��ȥ,����������
	int getcmd(unsigned char id,unsigned char cmdid,QList<tcmd>& cmdlist);

	int getcmdcount();

	//��������
	int clearcmdbuff();



private:

	int nDeviceType ; //�豸����
	int nDeviceInd ; //�豸��������װ�ж��can��ʱ��ʶ��ֻ��
	int nReserved ; //����PCI9820������������
	int nCANInd ; //can������Ķ˿ں�

	int port;  //�˿�
	int speed; //����

	//�����
	QLinkedList<tcmd> RecvBuff;

	//�������
	QMutex mutex;

	//CAN�豸��
	QMutex devmutex;
};

//��������࣬��Ϊ�̼߳��Լ��߳��ڲ�ͨѶ֮��
class com
{
public:
	com();
	~com();

	int version;
	int anscmdcount;

	//��ͷ�Ĺ���״̬,������ͷ�ϱ�����
	//0X01��ʾ����״̬��0X02��ʾ����״̬��0X03��ʾ�ֶ�״̬��0X04��ʾ��׼��״̬��0X05��ʾ��ӡ��״̬
	unsigned char printstatus[16];

	//Ԥ�����ͷ״̬������Ǹ��������ļ��õ��ģ�0X01��ʾ����״̬ 0X03��ʾ����״̬ 0X04��ʾ��׼��״̬
	unsigned char presetprintstatus[16];

	//����������״̬0:  ����״̬           1������״̬
	unsigned char lockreleasestatus[16];

	//��ת״̬0:  �޷�ת           1����ת״̬
	unsigned char circlestatus[16];

	//1:  ̧��״̬           0������״̬
	unsigned char updownstatus[16];

	//0:  ���ܷ���           1������̧��
	unsigned char platupdownstatus[16];

	//0:  �ϸ�����           1���ϸ�����
	unsigned char colorpipestatus[16];

	//0:  ��̨�ر�           1����̨�Ӵ�
	unsigned char magicstatus[16];

	//00: ����ͣ     01��������ת  10���÷�ת
	unsigned char bumpstatus[16];

	//��̨�����ٷֱ�
	unsigned char magicpercent[16];

	//����״̬,1 �Ѿ����� 0 û�ж���
	unsigned char zerostatus[16];

	//����λ��x
	short xstatus[16];
	//б��λ��y
	short ystatus[16];
	//�߶�λ��z
	short zstatus[16];

	//�������˶�������ȡ��
	double motorspeed;//m/min

	//�ٶ��Ƿ�ı�
	bool MotorSpeedChgFlag;

	//bdid���ϣ���Ϊ����֮�ã���ǰ�İ��ӵļ���
	QList<int> bdlist;

	//bdid��canidӳ�伯��
	QMap<int,int> bdnodemap;

	//�����Ƿ�������ͷbdid������������
	QMap<int,unsigned char> uicmdtoprintmap;

	//��������ͷ��������Ĳ���
	QMap<int,int> uicmdtoprintstepmap;

	//��������ͷ��������ļ�ʱ��
	QMap<int,int> uicmdtoprinttimermap;


	//�����ۺ������
	int mainuicmdstep;
	int mainuicmdcount;
	int mainuidcmdtimer;
	unsigned char mainuidcmd;

	//�Ƿ��������������������
	bool ismainuicmd;

	QMap<int,bool> uicmdtoprintansmap;
	QMap<int,bool> senduicmdtoprintmap;

	//��������ֳɹ�
	bool somemainuicmdfailed;

	//PLC�ۺ������
	int plccmdstep;
	int plccmdcount;
	int plcdcmdtimer;
	unsigned char plcdcmd;

	//�Ƿ��������������������
	bool isplccmd;

	//��������ֳɹ�
	bool someplccmdfailed;

	//������������
	bool opuicmdflag;
	int opuicmdstep;
	int opuicmdtimer;
	int opuicmdprintno;
	int opuicmdcmd;
	int opuicmdcount;


	//�Ӽ��ٶ����������Ķ���0-15 ��Ӧ��ͷ�� 16����
	unsigned char speedcmdplace;

	//�Ӽ��������
	int speedcmdstep;

	//������
	int speedcmdcount;

	//�Ӽ��ٶ�ʱ��
	int speedcmdtimer;

	//�Ӽ���С����
	int speedcmdlittlecmd;

	//����Ӧ���Ƿ��ڽ���
	bool isapply;

	//����Ӧ�õĲ���
	unsigned char applystep;

	//���ý��水����ʶ
	bool applyflag ; //�Ƿ�ȷ������Ӧ�ð�ť��

	//������Ӧ�õĶ�ʱ��
	int applytimer;

	//�Ƿ�ˢ��Ӧ�ã�ȫ�������·�
	bool refreshapply;

	//Ӧ�ü���
	int applycount;

	//�����ͺͽ��ܵ�����ԣ�����Ӧ��ʹ�ã�key�ǰ��ӵ�id��value�������
	QMap<int,QList<tbdonlineparacmdpair> > applyparacmdpair;

	//�������ߴ�����
	QMap<int,int> bdonlinestep;

	//�������߶�ʱ��
	QMap<int,int> bdonlinetimer;

	//�������߼�����
	QMap<int,int> bdonlinecount;

	//��������������
	QMap<int,QList<tbdonlineparacmdpair> > bdonlineparacmdpair;

	//�����ȴ����ص�map������id�Ͳ�����ӳ��
	QMap<int,QMap<tablekey,tablenode>::iterator> applybidparamap;

	//����������ʧ��
	bool hasparafailed;
	//���������гɹ�
	bool hasparasucceed;

	//Ӧ��Ӧ�𷵻صĵط���1 set���� 2 ������
	unsigned char applyansplace;

	//��ͷ����CAN�ȴ�Ӧ���ʶ
	bool printchgspeedansflag;

	//��ͷ���ٵĲ���
	int printchgspeedstep;
	int printchgspeedcount;

	//���͵ļ�¼
	QMap<int,bool> sendprintchgspeedmap;

	//���صļ�¼
	QMap<int,bool> printchgspeedansmap;

	//��ͷ���ٶ�ʱ��
	int printchgspeedtimer;

	//��ͷ���ٵȴ����ص�list
	QList<int> printchgspeedmap;

	//����ͷ���ٵĲ���
	int singleprintchgspeedstep;

	//����ͷ���ٶ�ʱ��
	int singleprintchgspeedtimer;

	//����ͷ����id
	int singleprintchgspeedid;

	QMap<int,int> actionzerostep; //���㶯������ 0�޶��� 1��ʼ
	QMap<int,int> actionzerotimer; //���㶯����ʱ��

	QMap<int,double> actionzeroloc1; //�����һ��λ��
	QMap<int,double> actionzeroloc2; //����ڶ���λ��
	QMap<int,double> actionzeroloc3; //���������λ��
	QMap<int,double> actionzeroloc4; //������ĸ�λ��

	int ZeroDetectWidth; //���㴫�����źſ��
	int ZeroHighSpeedWidth; //������ٶο��
	int ZeroLowSpeedWidth; //������ٶο��



	//�ⲿ���������źű�ʶ
	int plcreqstartstep;
	int plcreqstarttimer;
	int plcreqstartcount;
	bool printok;


	//��ͷ��ظ��Ƿ��������
	QMap<int,bool> startbdansmap;

	//���͸���ͷ�����������󣬽��빤����ֹͣ�����������Ӧ�����
	QMap<int,bool> printcmdansmap;

	//�Ƿ��Ѿ��������������ͷ
	QMap<int,bool> sendprintcmdmap;

	//��������׼����
	QMap<int,bool> printreadymap;

	//��ǰ�������ͷ
	QList<int> activeprintlist;

	//��ͷ�Ƿ���׼��״̬
	bool preprintlist[16];

	//activeprintlist�߳���
	QReadWriteLock activeprintlistmutex;


	//�ⲿͣ��
	int plcreqstopstep;
	int plcreqstopcount;
	int plcreqstoptimer;

	bool printchgspeedfailed;
	bool printchgspeedsucceed;

	//��ͷ״̬�޸ı�ʶ
	bool printactiveflag;
	int printactivetimer;
	int printactivestep;
	int printactivenum;
	int printactivecount;
	//��ͷ״̬����
	int printactivecmd;

	//�ָ���ͷλ������
	struct restore restoreTask[16];
	//������ͷ��ǰλ������
	struct saveloc savelocTask[16];

	bool delcauflag;

	//CAN�����
	QList<unsigned char> cmdlist;


	//��ǰͨѶ״̬�����������Ƿ�����
	QMap<int,bool> nodeonlinemap;

	//ͨѶʧ�ܵĴ����������ﵽ3�Σ������ͺͽ��ܣ�����ʼ����������������ʧ�ܣ�����nodeonlinemapΪfalse
	//ÿ�ﵽ3�ξͽ���ֵ��0
	QMap<int,int> nodeonlinecountmap;

	//�����ظ�ID��3�ζ���⵽�����ǣ������󻯵��Ų���ͷ�������
	QMap<int,int> nodedupidmap;

	//��ʷͨѶ״̬�����������Ƿ�����
	QMap<int,bool> oldnodeonlinemap;

	//״̬���ڲ��¼�
	QList<tintevent> eventqueue;


	//�������Ա�ʶ
	int heartbeatstep;

	//�������Զ�ʱ��
	int heartbeattimer;

	//�������Լ�����
	int heartbeatcount;

	//����������������
	int heartbeatbegincnt;

	//����ʧ�ܼ�����
	int networkfailcount;

	//��ǰ��ͷ������
	char PrintNum;

	//����id ����λ��idӳ���
	QMap<int,int> bdcautionmap;

	//����λ��id���ַ����map
	QMap<unsigned int,QString> cautionlocationidstrmap;

	//����id���ַ���map
	QMap<unsigned int,QString> cautionidstrmap;

	//�¼�id���ַ���map
	QMap<unsigned int,QString> eventidstrmap;

	//�˶������Ϲ���״̬��
	QList<ta4ncaution> historya4ncaution;
	QList<ta4ncaution> currenta4ncaution;

	//Բ������״̬
	int machprintstatus; //����״̬ 0��ʼ�� 1׼������ 2����

	//����IO������ϱ������ʶ
	unsigned char iocautonflag;
	unsigned int iocautiontimer;

	//������Ϣ���̴���
	unsigned char nocautionflag;

	//������ͷ���԰�ť�Ƿ�����
	bool enuichgspdbt;

	//״̬�ı��ʶ
	int statuschgstep;
	int statuschgcount;
	int statuschgtimer;

	//IO�˿��ϱ�����״̬��false:û�й��� true:�й���
	bool IOCautionStatus;
	int IOCautionStatusstep;
	int IOCautionStatuscount;
	int IOCautionStatustimer;

	//IO�˿��ϱ�����״̬��false:û�д��� true:�д���
	bool IOErrStatus;
	int IOErrStatusstep;
	int IOErrStatuscount;
	int IOErrStatustimer;

	//IO�˿��ϱ���ͣ״̬��false:û�м�ͣ true:�м�ͣ
	bool IOHaltStatus;
	int IOHaltStatusstep;
	int IOHaltStatuscount;
	int IOHaltStatustimer;

	//����ˢ������
	int FlushDataReqStep;
	int FlushDataReqTimer;

	//�ػ���ʶ
	bool PowerOff;

	//��̨�߳��뿪ѭ�����������¼��õ㣬��������Ƚϼ�ʵ��
	bool quitthread;

	//���ٵ������л�ʱ��
	unsigned long LowToHighSpeedtime;

	//����Ի���ʱ��
	QMap<int,int> movedirecttimer;

	//����Ի�״̬ 0 Ĭ�� 1���� 2���� 3���� 4����
	QMap<int,int> movedirectstatus;

	//�Ƿ��Ѿ�������״̬�������ͷ
	QMap<int,bool> statuschgsendcmdmap;

	//�Ƿ��յ���ͷ״̬����Ӧ��
	QMap<int,bool> statuschganscmdmap;

	//welcom�����Ƿ��������
	unsigned char initflag;

	//�Ƿ�����λͣ��
	bool NeedStopAtWorkfinished;

	bool chkver(unsigned char para1,unsigned char para2);

	//�Ƿ�������
	bool iscmd(unsigned char cmd);

	//���ý����Ƿ�������ͷ�������������
	int setcmdtoprint(int bdid,bool flag);

	//Ϊ�˼���CPU����ǰ������ʾ��ˢ�½���
	unsigned char CurrentWnd; //0�������� 1����ͷ���� 2������ 3������ 4���������� 5������
	QMutex  CurrentWndMutex;

	//�Ƿ�Ҫ��¼����
	bool bloprecflag;

	//�Ƿ�Ҫ��¼�쳣��¼
	bool blexceptrecflag;

	//�Ƿ��¼������Ϣ
	bool bldbginforecflag;

	//�Ƿ���������˶���
	bool blmotorcardsetflag;

	//Ħ��ϵ�����Ա�ʶ
	int measurepulsestartstep;
	int measurepulsestartcount;
	int measurepulsestarttimer;

	int measurepulsestopstep;
	int measurepulsestopcount;
	int measurepulsestoptimer;

	//PLC�Ƿ�����
	bool OfflineFlag;

	//IO��ʶ��true ��Ч flase ��Ч
	//����������
	bool PlcStartUpFlag;

	//ֹͣ������
	bool PlcStopUpFlag;

	//����������
	bool PlcErrUpFlag;

	//�����½���
	bool PlcErrDownFlag;

	//�澯������
	bool PlcCauUpFlag;

	//�澯�½���
	bool PlcCauDownFlag;

	//��������������
	bool PlcJinbuyunxinUpFlag;

	//���������½���
	bool PlcJinbuyunxinDownFlag;

	//ӡ����������������
	bool PlcYinhuadaodaiyunxinUpFlag;

	//ӡ�����������½���
	bool PlcYinhuadaodaiyunxinDownFlag;

	//�淿����1����������
	bool PlcHongfangwangdai1yunxinUpFlag;

	//�淿����1�����½���
	bool PlcHongfangwangdai1yunxinDownFlag;

	//�淿����2����������
	bool PlcHongfangwangdai2yunxinUpFlag;

	//�淿����2�����½���
	bool PlcHongfangwangdai2yunxinDownFlag;

	//�䲼����������
	bool PlcLuobuyunxinUpFlag;

	//�䲼�����½���
	bool PlcLuobuyunxinDownFlag;

	//������������������
	bool PlcGongjiangcilikaiguanUpFlag;

	//�������������½���
	bool PlcGongjiangcilikaiguanDownFlag;

	//������С�ı��ʶ
	bool PlcGongjiangcilidaxiaogaibianFlag;
	int PlcGongjiangcilidaxiao;

	//�Ͻ���̨���
	int Shangjiangcitaibianhao;

	//�Ͻ���̨�޸������ʶ
	int Shangjiangcitaistep;
	int Shangjiangcitaicount;
	int Shangjiangcitaitimer;
	bool Shangjiangcitaiflag;

	//����ҳ��ļ�¼
	int iHelpViewLocation;
	int iHelpPageLocation;
	bool bHelpCrossPage;

	//��ʼ��
	void init();

	//�Ƿ��а�������
	bool hasbdonline();

	//���ٴ�CANʧ�ܱ���Ϊ�����߹���
	int canfailedcnt;

	//�ŷ��������ת����
	char ServoDir; //1���� 0��ת

	bool cardok;//false�������� true ������

	//��̨�ϴ�ˢ�²���
	int MagicOnFlushStep[16];
	//��̨�ϴ�ˢ�¶�ʱ��
	int MagicOnFlushTimer[16];

	//�����ͺ���
	float Follow;
};

//�Ĵ�����
struct _regs{

	//������master��slave
	unsigned char readarray[100];
	//��ʷֵ
	unsigned char hisreadarray[100];

	//д������master
	unsigned char writearray[100];

	//��������
	unsigned char heartbeatcnt;

	//���ػ���������λλ��
	int PcEnableStartBitIndex;

	//���ػ���������λλ��
	int PcMotorAccBitIndex;

	//���ػ���������λλ��
	int PcMotorDecBitIndex;

	//���ػ�����λλ��
	int PcErrBitIndex;

	//���ػ�����λλ��
	int PcCauBitIndex;

	//���ػ�����̺����λλ��
	int PcMotorOnBitIndex;

	//���ػ���ֹ̺ͣλλ��
	int PcMotorOffBitIndex;

	//���ػ���������λλ��
	int PcWorkFinBitIndex;

	//���ػ���������λλ��
	int PcSetHeartBeatBitIndex;

	//���ػ�����ˢ������λλ��
	int PcFlushDataReqBitIndex;

	//PLC������������λλ��
	int PlcStartReqBitIndex;

	//PLC����ֹͣλλ��
	int PlcStopBitIndex;

	//PLC����λλ��
	int PlcErrBitIndex;

	//PLC����λλ��
	int PlcCauBitIndex;

	//PLC��������λλ��
	int PlcJinbuyunxinBitIndex;

	//PLCӡ����������λλ��
	int PlcYinhuadaodaiyunxinBitIndex;

	//PLC�淿����1����λλ��
	int PlcHongfangwangdai1yunxinBitIndex;

	//PLC�淿����2����λλ��
	int PlcHongfangwangdai2yunxinBitIndex;

	//PLC�䲼����λλ��
	int PlcLuobuyunxinBitIndex;

	//PLC������������λλ��
	int PlcGongjiangcilikaiguanBitIndex;

	//���ػ����IO��λ��
	int PcOutIOWordIndex;

	//���ػ�����˫��λ��
	int PcWorkRecDWordIndex;

	//������������ָ��
	void (*PlcStartReqFp)(bool);

	//ͣ����������ָ��
	void (*PlcStopFp)(bool);

	//��������ָ��
	void (*PlcErrFp)(bool);

	//����������ָ��
	void (*PlcCauFp)(bool);

	//�������д�����ָ��
	void (*PlcJinbuyunxinFp)(bool);

	//ӡ���������д�����ָ��
	void (*PlcYinhuadaodaiyunxinFp)(bool);

	//�淿����1���д�����ָ��
	void (*PlcHongfangwangdai1yunxinFp)(bool);

	//�淿����2���д�����ָ��
	void (*PlcHongfangwangdai2yunxinFp)(bool);

	//�䲼���д�����ָ��
	void (*PlcLuobuyunxinFp)(bool);

	//�����������ش�����ָ��
	void (*PlcGongjiangcilikaiguanFp)(bool);

	//������С�ı䴦����ָ��
	void (*PlcGongjiangcilidaxiaogaibianFp)(bool,unsigned char);

};

//�Ĵ�����ʼ���ӿ�
void initRegs(struct _regs* regs);

//regs���modbus�Ľӿ�
void ReadRegs(struct _regs* regs,unsigned int index,unsigned char* ch);
void WriteRegs(struct _regs* regs,unsigned int index,unsigned char ch);

//��ʼ��
//���ػ���������λλ��
void SetPcEnableStartBitIndex(struct _regs* regs,int index);

//���ػ���������λλ��
void SetPcMotorAccBitIndex(struct _regs* regs,int index);

//���ػ���������λλ��
void SetPcMotorDecBitIndex(struct _regs* regs,int index);

//���ػ�����λλ��
void SetPcErrBitIndex(struct _regs* regs,int index);

//���ػ�����λλ��
void SetPcCauBitIndex(struct _regs* regs,int index);

//���ػ�����̺����λλ��
void SetPcMotorOnBitIndex(struct _regs* regs,int index);

//���ػ���ֹ̺ͣλλ��
void SetPcMotorOffBitIndex(struct _regs* regs,int index);

//���ػ���������λλ��
void SetPcWorkFinBitIndex(struct _regs* regs,int index);

//���ػ���������λλ��
void SetPcSetHeartBeatBitIndex(struct _regs* regs,int index);

//���ػ�����ˢ������λλ��
void SetPcFlushDataReqBitIndex(struct _regs* regs,int index);

//PLC������������λλ��
void SetPlcStartReqBitIndex(struct _regs* regs,int index);

//PLC����ֹͣλλ��
void SetPlcStopBitIndex(struct _regs* regs,int index);

//PLC����λλ��
void SetPlcErrBitIndex(struct _regs* regs,int index);

//PLC����λλ��
void SetPlcCauBitIndex(struct _regs* regs,int index);



//PLC��������λλ��
void SetPlcJinbuyunxinBitIndex(struct _regs* regs,int index);


//PLCӡ����������λλ��
void SetPlcYinhuadaodaiyunxinBitIndex(struct _regs* regs,int index);


//PLC�淿����1����λλ��
void SetPlcHongfangwangdai1yunxinBitIndex(struct _regs* regs,int index);


//PLC�淿����2����λλ��
void SetPlcHongfangwangdai2yunxinBitIndex(struct _regs* regs,int index);


//PLC�䲼����λλ��
void SetPlcLuobuyunxinBitIndex(struct _regs* regs,int index);

//PLC������������λλ��
void SetPlcGongjiangcilikaiguanBitIndex(struct _regs* regs,int index);

//���ػ����IO��λ��
void SetPcOutIOWordIndex(struct _regs* regs,int index);

//���ػ�����˫��λ��
void SetPcWorkRecDWordIndex(struct _regs* regs,int index);

//���ٲ�����λ��
void SetZeroSpeedWordIndex(struct _regs* regs,int index);

//�����ٲ�����λ��
void SetBootSpeedWordIndex(struct _regs* regs,int index);

//����ٲ�����λ��
void SetMaxSpeedWordIndex(struct _regs* regs,int index);

//����ʱ�������λ��
void SetAccSpeedTimeWordIndex(struct _regs* regs,int index);

//����ʱ�������λ��
void SetDecSpeedTimeWordIndex(struct _regs* regs,int index);

//������Сֵ��λ��
void SetNetBeltMinWordIndex(struct _regs* regs,int index);

//�������ֵ��λ��
void SetNetBeltMaxWordIndex(struct _regs* regs,int index);

//��������Сֵ��λ��
void SetNetBeltMinStepWordIndex(struct _regs* regs,int index);

//�������Ӵ�ֵ��λ��
void SetNetBeltMaxStepWordIndex(struct _regs* regs,int index);

//�ŷ����Ƶ����λ��
void SetDischargeMinFreqWordIndex(struct _regs* regs,int index);

//�ŷ����Ƶ����λ��
void SetDischargeMaxFreqWordIndex(struct _regs* regs,int index);

//������Ƶ����λ��
void SetFanMinFreqWordIndex(struct _regs* regs,int index);

//������Ƶ����λ��
void SetFanMaxFreqWordIndex(struct _regs* regs,int index);

//��ƫ����ʱ����λ��
void SetCorrectTimeWordIndex(struct _regs* regs,int index);

//��ƫ����������λ��
void SetCorrectCycleWordIndex(struct _regs* regs,int index);

//���ΰ�����¶���λ��
void SetBoardLowTempWordIndex(struct _regs* regs,int index);

//���ΰ�����¶���λ��
void SetBoardHighTempWordIndex(struct _regs* regs,int index);

//�淿����¶���λ��
void SetDryboxLowTempWordIndex(struct _regs* regs,int index);

//�淿����¶���λ��
void SetDryboxHighTempWordIndex(struct _regs* regs,int index);

//��������IO������ָ�루�ص��ӿڣ�
void SetPlcStartReqFp(struct _regs* regs,void (*fp)(bool));
void SetPlcStopFp(struct _regs* regs,void (*fp)(bool));
void SetPlcErrFp(struct _regs* regs,void (*fp)(bool));
void SetPlcCauFp(struct _regs* regs,void (*fp)(bool));

void SetPlcJinbuyunxinFp(struct _regs* regs,void (*fp)(bool));
void SetPlcYinhuadaodaiyunxinFp(struct _regs* regs,void (*fp)(bool));
void SetPlcHongfangwangdai1yunxinFp(struct _regs* regs,void (*fp)(bool));
void SetPlcHongfangwangdai2yunxinFp(struct _regs* regs,void (*fp)(bool));
void SetPlcLuobuyunxinFp(struct _regs* regs,void (*fp)(bool));
void SetPlcGongjiangcilikaiguanFp(struct _regs* regs,void (*fp)(bool));
void SetPlcGongjiangcilidaxiaogaibianFp(struct _regs* regs,void (*fp)(bool,unsigned char));

//��������
void WritePcEnableStartBit(struct _regs* regs,bool bit);

//��������
void WritePcMotorAccBit(struct _regs* regs,bool bit);

//��������
void WritePcMotorDecBit(struct _regs* regs,bool bit);

//���ػ�����
void WritePcErrBit(struct _regs* regs,bool bit);

//���ػ�����
void WritePcCauBit(struct _regs* regs,bool bit);

//�������
void WritePcMotorOnBit(struct _regs* regs,bool bit);

//���ֹͣ
void WritePcMotorOffBit(struct _regs* regs,bool bit);

//��������
void WritePcWorkFinBit(struct _regs* regs,bool bit);

//д����λ
void WritePcSetHeartBeatBit(struct _regs* regs,bool bit);

//дˢ����������
void WritePcFlushDataReqBit(struct _regs* regs,bool bit);

//д����ʱ��̨δ�����澯
void WriteMagicCauBit(struct _regs* regs,bool bit);

//д������
void WritePcWorkRecDWord(struct _regs* regs,int word);

//д����
void WriteZeroSpeedWord(struct _regs* regs,int word);

//д������
void WriteBootSpeedWord(struct _regs* regs,int word);

//д�����
void WriteMaxSpeedWord(struct _regs* regs,int word);

//д����ʱ��
void WriteAccSpeedTimeWord(struct _regs* regs,int word);

//д����ʱ��
void WriteDecSpeedTimeWord(struct _regs* regs,int word);

//д��������ֵ
void WriteNetBeltMinWord(struct _regs* regs,int word);

//д��������ֵ
void WriteNetBeltMaxWord(struct _regs* regs,int word);

//д��������Сֵ
void WriteNetBeltMinStepWord(struct _regs* regs,int word);

//д�����������ֵ
void WriteNetBeltMaxStepWord(struct _regs* regs,int word);

//д�ŷ����Ƶ��
void WriteDischargeMinFreqWord(struct _regs* regs,int word);

//д�ŷ����Ƶ��
void WriteDischargeMaxFreqWord(struct _regs* regs,int word);

//д������Ƶ��
void WriteFanMinFreqWord(struct _regs* regs,int word);

//д������Ƶ��
void WriteFanMaxFreqWord(struct _regs* regs,int word);

//д��ƫ����ʱ��
void WriteCorrectTimeWord(struct _regs* regs,int word);

//д��ƫ��������
void WriteCorrectCycleWord(struct _regs* regs,int word);

//д���ΰ�����¶���
void WriteBoardLowTempWord(struct _regs* regs,int word);

//д���ΰ�����¶���
void WriteBoardHighTempWord(struct _regs* regs,int word);

//д�淿����¶���
void WriteDryboxLowTempWord(struct _regs* regs,int word);

//д�淿����¶���
void WriteDryboxHighTempWord(struct _regs* regs,int word);
   
//ˢ�¶����������Ĵ���
void FlushRegsReadHeatbeat(struct _regs* regs);

//����������������ֵ
void ReadRegsReadHeatbeat(struct _regs* regs,unsigned char* pch);

//����IO������
void PlcStartReqHook(bool flag);
void PlcStopHook(bool flag);
void PlcErrHook(bool flag);
void PlcCauHook(bool flag);

void PlcJinbuyunxinHook(bool flag);
void PlcYinhuadaodaiyunxinHook(bool flag);
void PlcHongfangwangdai1yunxinHook(bool flag);
void PlcHongfangwangdai2yunxinHook(bool flag);
void PlcLuobuyunxinHook(bool flag);
void PlcGongjiangcilikaiguanHook(bool flag);
void PlcGongjiangcilidaxiaogaibianHook(bool flag,unsigned char percent);

extern com pubcom;


#endif /* CANIF_H_ */
