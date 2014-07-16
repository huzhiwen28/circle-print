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



//状态机内部事件
typedef struct _intevent{
	int nodeid;//板子ID
	int eventid;//事件ID 01：板子上线  02：板子离线
} tintevent;


//状态机内部事件
class ta4ncaution{
public:
	int location;//位置
	int errcode;//故障码
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
	unsigned char cmd; //命令
	unsigned char id; //发送板子的id
	unsigned char para1; //参数1
	unsigned char para2; //参数2
	unsigned int  timecnt;//时间戳
} tcmd;


//板子上线参数设置发送命令
typedef struct
{
	//网络ID
	int canid;

	//待发送命令
	tcmd cmd;

	//是否发送成功
	bool sendsucceed;
} tbdonlineparasendcmd;


//板子上线参数设置接收命令
typedef struct
{
	//待接收的命令
	tcmd cmd;

	//是否接收成功
	bool recvsucceed;
} tbdonlinepararecvcmd;

//板子上线参数设置命令对
typedef struct  
{
	tbdonlineparasendcmd sendcmd;
	tbdonlinepararecvcmd recvcmd;
}tbdonlineparacmdpair;

//恢复圆网位置任务
struct restore{
	int id;//网头id
	int timer;//等待时间200ms，等电机动作做完
	int step;//任务接口 1开始 0任务结束
};

//保存圆网位置任务
struct saveloc{
	int id;//网头id
	int timer;//等待时间200ms，等电机动作做完
	int step;//任务接口 1开始 0任务结束
};

//CAN接口卡
//CAN设备的使用逻辑采用“生产者-消费者”模式，而不是采用CAN命令触发模式
class canif
{

public:
	canif();
	virtual ~canif();

	//成功打开和初始化
	bool succeedopenandinit;

	//打开并使用
	bool open();

	//关闭设备
	bool close();

	//从设备内存取数据到缓存
	bool refresh();

	//刷新缓存中的时间戳，并且命令如果超时的话，就删除命令
	void refreshtime();


	//发送一个命令,发送的对象是fid
	bool sendcmd(unsigned long fid,tcmd* cmd);

	//查看缓存中是否有来自id板子命令,如果有的话，就把命令取到cmd中去
	bool getidcmd(unsigned char id,tcmd* cmd);

	//查看缓存中是否有板子id的 cmdid命令，如果有的话，就取到cmd中去
	bool getcmd(unsigned char id,unsigned char cmdid,tcmd* cmd);

	//查看缓存中是否有板子id的 cmdid命令，参数为para1,如果有的话，就取到cmd中去
	bool getcmd(unsigned char id,unsigned char cmdid,unsigned char para1,tcmd* cmd);

	//查看缓存中是否有板子id的 cmdid命令，参数为para1,para2,如果有的话，就取到cmd中去
	bool getcmd(unsigned char id,unsigned char cmdid,unsigned char para1,unsigned char para2,tcmd* cmd);

	//查看缓存中是否有板子id的 cmdid命令，参数为para1,如果有的话，就取到cmdlist中去,返回是数量
	int getcmd(unsigned char id,unsigned char cmdid,unsigned char para1,QList<tcmd>& cmdlist);

	//查看缓存中是否有板子id的 cmdid命令，参数为para1,para2 如果有的话，就取到cmdlist中去,返回是数量
	int getcmd(unsigned char id,unsigned char cmdid,unsigned char para1,unsigned char para2,QList<tcmd>& cmdlist);

	//查看缓存中是否有板子id的 cmdid命令,如果有的话，就取到cmdlist中去,返回是数量
	int getcmd(unsigned char id,unsigned char cmdid,QList<tcmd>& cmdlist);

	int getcmdcount();

	//清除命令缓存
	int clearcmdbuff();



private:

	int nDeviceType ; //设备类型
	int nDeviceInd ; //设备索引，安装有多个can卡时的识别只用
	int nReserved ; //对于PCI9820保留，无意义
	int nCANInd ; //can卡上面的端口号

	int port;  //端口
	int speed; //速率

	//命令缓存
	QLinkedList<tcmd> RecvBuff;

	//命令缓存锁
	QMutex mutex;

	//CAN设备锁
	QMutex devmutex;
};

//共享变量类，作为线程间以及线程内部通讯之用
class com
{
public:
	com();
	~com();

	int version;
	int anscmdcount;

	//网头的工作状态,此由网头上报更新
	//0X01表示休眠状态，0X02表示设置状态，0X03表示手动状态，0X04表示在准备状态，0X05表示在印花状态
	unsigned char printstatus[16];

	//预设的网头状态，这个是根据配置文件得到的，0X01表示休眠状态 0X03表示工作状态 0X04表示在准备状态
	unsigned char presetprintstatus[16];

	//张网和松网状态0:  松网状态           1：张网状态
	unsigned char lockreleasestatus[16];

	//反转状态0:  无反转           1：反转状态
	unsigned char circlestatus[16];

	//1:  抬网状态           0：下网状态
	unsigned char updownstatus[16];

	//0:  网架放下           1：网架抬起
	unsigned char platupdownstatus[16];

	//0:  料杆锁定           1：料杆松下
	unsigned char colorpipestatus[16];

	//0:  磁台关闭           1：磁台加磁
	unsigned char magicstatus[16];

	//00: 浆泵停     01：浆泵正转  10浆泵反转
	unsigned char bumpstatus[16];

	//磁台磁力百分比
	unsigned char magicpercent[16];

	//对零状态,1 已经对零 0 没有对零
	unsigned char zerostatus[16];

	//横向位置x
	short xstatus[16];
	//斜向位置y
	short ystatus[16];
	//高度位置z
	short zstatus[16];

	//主机，运动卡采样取得
	double motorspeed;//m/min

	//速度是否改变
	bool MotorSpeedChgFlag;

	//bdid集合，作为遍历之用，当前的板子的集合
	QList<int> bdlist;

	//bdid和canid映射集合
	QMap<int,int> bdnodemap;

	//界面是否有向网头bdid发起命令请求
	QMap<int,unsigned char> uicmdtoprintmap;

	//界面向网头发起命令的步骤
	QMap<int,int> uicmdtoprintstepmap;

	//界面向网头发起命令的计时器
	QMap<int,int> uicmdtoprinttimermap;


	//界面综合命令步骤
	int mainuicmdstep;
	int mainuicmdcount;
	int mainuidcmdtimer;
	unsigned char mainuidcmd;

	//是否有主界面操作操作命令
	bool ismainuicmd;

	QMap<int,bool> uicmdtoprintansmap;
	QMap<int,bool> senduicmdtoprintmap;

	//界面命令部分成功
	bool somemainuicmdfailed;

	//PLC综合命令步骤
	int plccmdstep;
	int plccmdcount;
	int plcdcmdtimer;
	unsigned char plcdcmd;

	//是否有主界面操作操作命令
	bool isplccmd;

	//界面命令部分成功
	bool someplccmdfailed;

	//操作界面命令
	bool opuicmdflag;
	int opuicmdstep;
	int opuicmdtimer;
	int opuicmdprintno;
	int opuicmdcmd;
	int opuicmdcount;


	//加减速度命令来自哪儿：0-15 对应网头板 16界面
	unsigned char speedcmdplace;

	//加减速命令步骤
	int speedcmdstep;

	//计数器
	int speedcmdcount;

	//加减速定时器
	int speedcmdtimer;

	//加减速小命令
	int speedcmdlittlecmd;

	//参数应用是否在进行
	bool isapply;

	//参数应用的步骤
	unsigned char applystep;

	//设置界面按键标识
	bool applyflag ; //是否按确定或者应用按钮了

	//参数的应用的定时器
	int applytimer;

	//是否刷新应用，全部参数下发
	bool refreshapply;

	//应用计数
	int applycount;

	//待发送和接受的命令对，参数应用使用，key是板子的id，value是命令对
	QMap<int,QList<tbdonlineparacmdpair> > applyparacmdpair;

	//板子上线处理步骤
	QMap<int,int> bdonlinestep;

	//板子上线定时器
	QMap<int,int> bdonlinetimer;

	//板子上线计数器
	QMap<int,int> bdonlinecount;

	//板子上线命令处理对
	QMap<int,QList<tbdonlineparacmdpair> > bdonlineparacmdpair;

	//参数等待返回的map，板子id和参数的映射
	QMap<int,QMap<tablekey,tablenode>::iterator> applybidparamap;

	//参数设置有失败
	bool hasparafailed;
	//参数设置有成功
	bool hasparasucceed;

	//应用应答返回的地方，1 set界面 2 主界面
	unsigned char applyansplace;

	//网头调速CAN等待应答标识
	bool printchgspeedansflag;

	//网头调速的步骤
	int printchgspeedstep;
	int printchgspeedcount;

	//发送的记录
	QMap<int,bool> sendprintchgspeedmap;

	//返回的记录
	QMap<int,bool> printchgspeedansmap;

	//网头调速定时器
	int printchgspeedtimer;

	//网头调速等待返回的list
	QList<int> printchgspeedmap;

	//单网头调速的步骤
	int singleprintchgspeedstep;

	//单网头调速定时器
	int singleprintchgspeedtimer;

	//单网头调速id
	int singleprintchgspeedid;

	QMap<int,int> actionzerostep; //对零动作步骤 0无动作 1开始
	QMap<int,int> actionzerotimer; //对零动作定时器

	QMap<int,double> actionzeroloc1; //对零第一个位置
	QMap<int,double> actionzeroloc2; //对零第二个位置
	QMap<int,double> actionzeroloc3; //对零第三个位置
	QMap<int,double> actionzeroloc4; //对零第四个位置

	int ZeroDetectWidth; //对零传感器信号宽度
	int ZeroHighSpeedWidth; //对零高速段宽度
	int ZeroLowSpeedWidth; //对零低速段宽度



	//外部请求启动信号标识
	int plcreqstartstep;
	int plcreqstarttimer;
	int plcreqstartcount;
	bool printok;


	//网头板回复是否可以启动
	QMap<int,bool> startbdansmap;

	//发送给网头的启动，错误，进入工作，停止工作等命令的应答情况
	QMap<int,bool> printcmdansmap;

	//是否已经发送了命令给网头
	QMap<int,bool> sendprintcmdmap;

	//板子启动准备好
	QMap<int,bool> printreadymap;

	//当前激活的网头
	QList<int> activeprintlist;

	//网头是否处于准备状态
	bool preprintlist[16];

	//activeprintlist线程锁
	QReadWriteLock activeprintlistmutex;


	//外部停机
	int plcreqstopstep;
	int plcreqstopcount;
	int plcreqstoptimer;

	bool printchgspeedfailed;
	bool printchgspeedsucceed;

	//网头状态修改标识
	bool printactiveflag;
	int printactivetimer;
	int printactivestep;
	int printactivenum;
	int printactivecount;
	//网头状态命令
	int printactivecmd;

	//恢复网头位置任务
	struct restore restoreTask[16];
	//保存网头当前位置任务
	struct saveloc savelocTask[16];

	bool delcauflag;

	//CAN命令集合
	QList<unsigned char> cmdlist;


	//当前通讯状态，各个板子是否在线
	QMap<int,bool> nodeonlinemap;

	//通讯失败的次数计数，达到3次（含发送和接受，含初始化）才算是真正的失败，才置nodeonlinemap为false
	//每达到3次就将数值复0
	QMap<int,int> nodeonlinecountmap;

	//板子重复ID，3次都检测到才算是，这个最大化的排查网头软件问题
	QMap<int,int> nodedupidmap;

	//历史通讯状态，各个板子是否在线
	QMap<int,bool> oldnodeonlinemap;

	//状态机内部事件
	QList<tintevent> eventqueue;


	//心跳测试标识
	int heartbeatstep;

	//心跳测试定时器
	int heartbeattimer;

	//心跳测试计数器
	int heartbeatcount;

	//心跳测试启动计数
	int heartbeatbegincnt;

	//网络失败计数器
	int networkfailcount;

	//当前网头的数量
	char PrintNum;

	//板子id 故障位置id映射表
	QMap<int,int> bdcautionmap;

	//故障位置id和字符间的map
	QMap<unsigned int,QString> cautionlocationidstrmap;

	//故障id和字符的map
	QMap<unsigned int,QString> cautionidstrmap;

	//事件id和字符的map
	QMap<unsigned int,QString> eventidstrmap;

	//运动卡故障管理状态机
	QList<ta4ncaution> historya4ncaution;
	QList<ta4ncaution> currenta4ncaution;

	//圆网整机状态
	int machprintstatus; //机器状态 0初始化 1准备就绪 2运行

	//对外IO板故障上报处理标识
	unsigned char iocautonflag;
	unsigned int iocautiontimer;

	//故障消息过程处理
	unsigned char nocautionflag;

	//界面网头调试按钮是否有用
	bool enuichgspdbt;

	//状态改变标识
	int statuschgstep;
	int statuschgcount;
	int statuschgtimer;

	//IO端口上报故障状态，false:没有故障 true:有故障
	bool IOCautionStatus;
	int IOCautionStatusstep;
	int IOCautionStatuscount;
	int IOCautionStatustimer;

	//IO端口上报错误状态，false:没有错误 true:有错误
	bool IOErrStatus;
	int IOErrStatusstep;
	int IOErrStatuscount;
	int IOErrStatustimer;

	//IO端口上报急停状态，false:没有急停 true:有急停
	bool IOHaltStatus;
	int IOHaltStatusstep;
	int IOHaltStatuscount;
	int IOHaltStatustimer;

	//参数刷新请求
	int FlushDataReqStep;
	int FlushDataReqTimer;

	//关机标识
	bool PowerOff;

	//后台线程离开循环，本来用事件好点，但是这个比较简单实用
	bool quitthread;

	//低速到高速切换时间
	unsigned long LowToHighSpeedtime;

	//纵向对花计时器
	QMap<int,int> movedirecttimer;

	//纵向对花状态 0 默认 1正慢 2反慢 3正快 4反快
	QMap<int,int> movedirectstatus;

	//是否已经发送了状态命令给网头
	QMap<int,bool> statuschgsendcmdmap;

	//是否收到网头状态命令应答
	QMap<int,bool> statuschganscmdmap;

	//welcom界面是否启动完毕
	unsigned char initflag;

	//是否工作到位停车
	bool NeedStopAtWorkfinished;

	bool chkver(unsigned char para1,unsigned char para2);

	//是否是命令
	bool iscmd(unsigned char cmd);

	//设置界面是否有向网头发起的命令请求
	int setcmdtoprint(int bdid,bool flag);

	//为了减少CPU，当前界面显示才刷新界面
	unsigned char CurrentWnd; //0：主界面 1：网头操作 2：设置 3：故障 4：生产管理 5：帮助
	QMutex  CurrentWndMutex;

	//是否要记录操作
	bool bloprecflag;

	//是否要记录异常记录
	bool blexceptrecflag;

	//是否记录调试信息
	bool bldbginforecflag;

	//是否可以设置运动卡
	bool blmotorcardsetflag;

	//摩擦系数测试标识
	int measurepulsestartstep;
	int measurepulsestartcount;
	int measurepulsestarttimer;

	int measurepulsestopstep;
	int measurepulsestopcount;
	int measurepulsestoptimer;

	//PLC是否离线
	bool OfflineFlag;

	//IO标识，true 有效 flase 无效
	//启动上升沿
	bool PlcStartUpFlag;

	//停止上升沿
	bool PlcStopUpFlag;

	//错误上升沿
	bool PlcErrUpFlag;

	//错误下降沿
	bool PlcErrDownFlag;

	//告警上升沿
	bool PlcCauUpFlag;

	//告警下降沿
	bool PlcCauDownFlag;

	//进布运行上升沿
	bool PlcJinbuyunxinUpFlag;

	//进布运行下降沿
	bool PlcJinbuyunxinDownFlag;

	//印花导带运行上升沿
	bool PlcYinhuadaodaiyunxinUpFlag;

	//印花导带运行下降沿
	bool PlcYinhuadaodaiyunxinDownFlag;

	//烘房网带1运行上升沿
	bool PlcHongfangwangdai1yunxinUpFlag;

	//烘房网带1运行下降沿
	bool PlcHongfangwangdai1yunxinDownFlag;

	//烘房网带2运行上升沿
	bool PlcHongfangwangdai2yunxinUpFlag;

	//烘房网带2运行下降沿
	bool PlcHongfangwangdai2yunxinDownFlag;

	//落布运行上升沿
	bool PlcLuobuyunxinUpFlag;

	//落布运行下降沿
	bool PlcLuobuyunxinDownFlag;

	//供浆磁力开关上升沿
	bool PlcGongjiangcilikaiguanUpFlag;

	//供浆磁力开关下降沿
	bool PlcGongjiangcilikaiguanDownFlag;

	//磁力大小改变标识
	bool PlcGongjiangcilidaxiaogaibianFlag;
	int PlcGongjiangcilidaxiao;

	//上浆磁台编号
	int Shangjiangcitaibianhao;

	//上浆磁台修改任务标识
	int Shangjiangcitaistep;
	int Shangjiangcitaicount;
	int Shangjiangcitaitimer;
	bool Shangjiangcitaiflag;

	//帮助页面的记录
	int iHelpViewLocation;
	int iHelpPageLocation;
	bool bHelpCrossPage;

	//初始化
	void init();

	//是否有板子上线
	bool hasbdonline();

	//多少次CAN失败被认为是离线故障
	int canfailedcnt;

	//伺服电机正反转方向
	char ServoDir; //1正传 0反转

	bool cardok;//false卡不存在 true 卡存在

	//磁台上磁刷新步骤
	int MagicOnFlushStep[16];
	//磁台上磁刷新定时器
	int MagicOnFlushTimer[16];

	//跟随滞后量
	float Follow;
};

//寄存器组
struct _regs{

	//读区，master至slave
	unsigned char readarray[100];
	//历史值
	unsigned char hisreadarray[100];

	//写区，至master
	unsigned char writearray[100];

	//心跳计数
	unsigned char heartbeatcnt;

	//工控机开机允许位位置
	int PcEnableStartBitIndex;

	//工控机主机加速位位置
	int PcMotorAccBitIndex;

	//工控机主机减速位位置
	int PcMotorDecBitIndex;

	//工控机报错位位置
	int PcErrBitIndex;

	//工控机报警位位置
	int PcCauBitIndex;

	//工控机开橡毯启动位位置
	int PcMotorOnBitIndex;

	//工控机橡毯停止位位置
	int PcMotorOffBitIndex;

	//工控机工作量满位位置
	int PcWorkFinBitIndex;

	//工控机设置心跳位位置
	int PcSetHeartBeatBitIndex;

	//工控机请求刷新数据位位置
	int PcFlushDataReqBitIndex;

	//PLC主机开机请求位位置
	int PlcStartReqBitIndex;

	//PLC主机停止位位置
	int PlcStopBitIndex;

	//PLC报错位位置
	int PlcErrBitIndex;

	//PLC报警位位置
	int PlcCauBitIndex;

	//PLC进布运行位位置
	int PlcJinbuyunxinBitIndex;

	//PLC印花导带运行位位置
	int PlcYinhuadaodaiyunxinBitIndex;

	//PLC烘房网带1运行位位置
	int PlcHongfangwangdai1yunxinBitIndex;

	//PLC烘房网带2运行位位置
	int PlcHongfangwangdai2yunxinBitIndex;

	//PLC落布运行位位置
	int PlcLuobuyunxinBitIndex;

	//PLC供浆磁力开关位位置
	int PlcGongjiangcilikaiguanBitIndex;

	//工控机输出IO字位置
	int PcOutIOWordIndex;

	//工控机计米双字位置
	int PcWorkRecDWordIndex;

	//开机请求处理函数指针
	void (*PlcStartReqFp)(bool);

	//停机请求处理函数指针
	void (*PlcStopFp)(bool);

	//报错处理函数指针
	void (*PlcErrFp)(bool);

	//报警处理函数指针
	void (*PlcCauFp)(bool);

	//进布运行处理函数指针
	void (*PlcJinbuyunxinFp)(bool);

	//印花导带运行处理函数指针
	void (*PlcYinhuadaodaiyunxinFp)(bool);

	//烘房网带1运行处理函数指针
	void (*PlcHongfangwangdai1yunxinFp)(bool);

	//烘房网带2运行处理函数指针
	void (*PlcHongfangwangdai2yunxinFp)(bool);

	//落布运行处理函数指针
	void (*PlcLuobuyunxinFp)(bool);

	//供浆磁力开关处理函数指针
	void (*PlcGongjiangcilikaiguanFp)(bool);

	//磁力大小改变处理函数指针
	void (*PlcGongjiangcilidaxiaogaibianFp)(bool,unsigned char);

};

//寄存器初始化接口
void initRegs(struct _regs* regs);

//regs针对modbus的接口
void ReadRegs(struct _regs* regs,unsigned int index,unsigned char* ch);
void WriteRegs(struct _regs* regs,unsigned int index,unsigned char ch);

//初始化
//工控机开机允许位位置
void SetPcEnableStartBitIndex(struct _regs* regs,int index);

//工控机主机加速位位置
void SetPcMotorAccBitIndex(struct _regs* regs,int index);

//工控机主机减速位位置
void SetPcMotorDecBitIndex(struct _regs* regs,int index);

//工控机报错位位置
void SetPcErrBitIndex(struct _regs* regs,int index);

//工控机报警位位置
void SetPcCauBitIndex(struct _regs* regs,int index);

//工控机开橡毯启动位位置
void SetPcMotorOnBitIndex(struct _regs* regs,int index);

//工控机橡毯停止位位置
void SetPcMotorOffBitIndex(struct _regs* regs,int index);

//工控机工作量满位位置
void SetPcWorkFinBitIndex(struct _regs* regs,int index);

//工控机设置心跳位位置
void SetPcSetHeartBeatBitIndex(struct _regs* regs,int index);

//工控机请求刷新数据位位置
void SetPcFlushDataReqBitIndex(struct _regs* regs,int index);

//PLC主机开机请求位位置
void SetPlcStartReqBitIndex(struct _regs* regs,int index);

//PLC主机停止位位置
void SetPlcStopBitIndex(struct _regs* regs,int index);

//PLC报错位位置
void SetPlcErrBitIndex(struct _regs* regs,int index);

//PLC报警位位置
void SetPlcCauBitIndex(struct _regs* regs,int index);



//PLC进布运行位位置
void SetPlcJinbuyunxinBitIndex(struct _regs* regs,int index);


//PLC印花导带运行位位置
void SetPlcYinhuadaodaiyunxinBitIndex(struct _regs* regs,int index);


//PLC烘房网带1运行位位置
void SetPlcHongfangwangdai1yunxinBitIndex(struct _regs* regs,int index);


//PLC烘房网带2运行位位置
void SetPlcHongfangwangdai2yunxinBitIndex(struct _regs* regs,int index);


//PLC落布运行位位置
void SetPlcLuobuyunxinBitIndex(struct _regs* regs,int index);

//PLC供浆磁力开关位位置
void SetPlcGongjiangcilikaiguanBitIndex(struct _regs* regs,int index);

//工控机输出IO字位置
void SetPcOutIOWordIndex(struct _regs* regs,int index);

//工控机计米双字位置
void SetPcWorkRecDWordIndex(struct _regs* regs,int index);

//零速参数字位置
void SetZeroSpeedWordIndex(struct _regs* regs,int index);

//导布速参数字位置
void SetBootSpeedWordIndex(struct _regs* regs,int index);

//最高速参数字位置
void SetMaxSpeedWordIndex(struct _regs* regs,int index);

//升速时间参数字位置
void SetAccSpeedTimeWordIndex(struct _regs* regs,int index);

//降速时间参数字位置
void SetDecSpeedTimeWordIndex(struct _regs* regs,int index);

//网带最小值字位置
void SetNetBeltMinWordIndex(struct _regs* regs,int index);

//网带最大值字位置
void SetNetBeltMaxWordIndex(struct _regs* regs,int index);

//网带增加小值字位置
void SetNetBeltMinStepWordIndex(struct _regs* regs,int index);

//网带增加大值字位置
void SetNetBeltMaxStepWordIndex(struct _regs* regs,int index);

//排风最低频率字位置
void SetDischargeMinFreqWordIndex(struct _regs* regs,int index);

//排风最高频率字位置
void SetDischargeMaxFreqWordIndex(struct _regs* regs,int index);

//风机最低频率字位置
void SetFanMinFreqWordIndex(struct _regs* regs,int index);

//风机最高频率字位置
void SetFanMaxFreqWordIndex(struct _regs* regs,int index);

//纠偏工作时间字位置
void SetCorrectTimeWordIndex(struct _regs* regs,int index);

//纠偏工作周期字位置
void SetCorrectCycleWordIndex(struct _regs* regs,int index);

//弧形板最低温度字位置
void SetBoardLowTempWordIndex(struct _regs* regs,int index);

//弧形板最高温度字位置
void SetBoardHighTempWordIndex(struct _regs* regs,int index);

//烘房最低温度字位置
void SetDryboxLowTempWordIndex(struct _regs* regs,int index);

//烘房最高温度字位置
void SetDryboxHighTempWordIndex(struct _regs* regs,int index);

//设置输入IO处理函数指针（回调接口）
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

//开机允许
void WritePcEnableStartBit(struct _regs* regs,bool bit);

//主机加速
void WritePcMotorAccBit(struct _regs* regs,bool bit);

//主机减速
void WritePcMotorDecBit(struct _regs* regs,bool bit);

//工控机报错
void WritePcErrBit(struct _regs* regs,bool bit);

//工控机报警
void WritePcCauBit(struct _regs* regs,bool bit);

//马达启动
void WritePcMotorOnBit(struct _regs* regs,bool bit);

//马达停止
void WritePcMotorOffBit(struct _regs* regs,bool bit);

//工作量满
void WritePcWorkFinBit(struct _regs* regs,bool bit);

//写心跳位
void WritePcSetHeartBeatBit(struct _regs* regs,bool bit);

//写刷新数据请求
void WritePcFlushDataReqBit(struct _regs* regs,bool bit);

//写工作时磁台未开启告警
void WriteMagicCauBit(struct _regs* regs,bool bit);

//写工作量
void WritePcWorkRecDWord(struct _regs* regs,int word);

//写零速
void WriteZeroSpeedWord(struct _regs* regs,int word);

//写导布速
void WriteBootSpeedWord(struct _regs* regs,int word);

//写最高速
void WriteMaxSpeedWord(struct _regs* regs,int word);

//写升速时间
void WriteAccSpeedTimeWord(struct _regs* regs,int word);

//写降速时间
void WriteDecSpeedTimeWord(struct _regs* regs,int word);

//写网带下限值
void WriteNetBeltMinWord(struct _regs* regs,int word);

//写网带上限值
void WriteNetBeltMaxWord(struct _regs* regs,int word);

//写网增加最小值
void WriteNetBeltMinStepWord(struct _regs* regs,int word);

//写网带增加最大值
void WriteNetBeltMaxStepWord(struct _regs* regs,int word);

//写排风最低频率
void WriteDischargeMinFreqWord(struct _regs* regs,int word);

//写排风最高频率
void WriteDischargeMaxFreqWord(struct _regs* regs,int word);

//写风机最低频率
void WriteFanMinFreqWord(struct _regs* regs,int word);

//写风机最高频率
void WriteFanMaxFreqWord(struct _regs* regs,int word);

//写纠偏工作时间
void WriteCorrectTimeWord(struct _regs* regs,int word);

//写纠偏工作周期
void WriteCorrectCycleWord(struct _regs* regs,int word);

//写弧形板最低温度字
void WriteBoardLowTempWord(struct _regs* regs,int word);

//写弧形板最高温度字
void WriteBoardHighTempWord(struct _regs* regs,int word);

//写烘房最低温度字
void WriteDryboxLowTempWord(struct _regs* regs,int word);

//写烘房最高温度字
void WriteDryboxHighTempWord(struct _regs* regs,int word);
   
//刷新读区的心跳寄存器
void FlushRegsReadHeatbeat(struct _regs* regs);

//读读区的心跳计数值
void ReadRegsReadHeatbeat(struct _regs* regs,unsigned char* pch);

//输入IO处理函数
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
