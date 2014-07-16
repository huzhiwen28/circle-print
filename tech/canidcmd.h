/*
 * canidcmd.h
 *
 *  Created on: 2009-8-2
 *      Author: Owner
 */

#ifndef CANIDCMD_H_
#define CANIDCMD_H_
////////////////////////////////////
//CAN对象ID
////////////////////////////////////
const int ALLPRINTID = 0x280;
const int ALLID = 0x300;

const int PCNODEID = 0x120;
const int PCRECVMASK = 0xffffffff;

//const int ZEROIONODEID = 0x110;
const int BUMPIONODEID = 0x140;
const int MAGICIONODEID = 0x150;
//const int OUTIONODEID = 0x130;

const int PRINT1NODEID = 0x200;
const int PRINT2NODEID = 0x201;
const int PRINT3NODEID = 0x202;
const int PRINT4NODEID = 0x203;
const int PRINT5NODEID = 0x204;
const int PRINT6NODEID = 0x205;
const int PRINT7NODEID = 0x206;
const int PRINT8NODEID = 0x207;
const int PRINT9NODEID = 0x208;
const int PRINT10NODEID = 0x209;
const int PRINT11NODEID = 0x20a;
const int PRINT12NODEID = 0x20b;
const int PRINT13NODEID = 0x20c;
const int PRINT14NODEID = 0x20d;
const int PRINT15NODEID = 0x20e;
const int PRINT16NODEID = 0x20f;

////////////////////////////////////
//命令
////////////////////////////////////
const int ERRCMD = 0x00; //错误命令
const int COMTESTCMD = 0x01; //通讯测试
const int VERTESTCMD = 0x02; //版本校验
const int SETBUMPNOCMD = 0x12; //设置浆泵编号
const int SETPRINTPARACMD = 0x13; //设置网头参数
const int SETBUMPIONOCMD = 0x14; //设置浆泵IO编号
const int SETMAGICIONOCMD = 0x15; //设置磁台IO编号
const int SETMAGICANANOCMD = 0x16; //设置磁台模拟量编号
const int SETPRINTMACHPARACMD = 0x17; //设置网头机器参数

const int MAGICRSPCMD = 0x21;//磁台状态上报命令，无需应答
const int BUMPRSPCMD = 0x22;//浆泵状态上报命令，无需应答
const int ZEROLOCCMD = 0x23;//零位置上报

const int CHGSPEEDCMD = 0x31; //主机调速命令应答
const int ZEROCMD = 0x32; //找零命令应答
const int RESERVECMD = 0x33; //反转命令应答
const int YMOVECMD = 0x34; //纵向对花命令应答

const int PRINTWORKSTATUSCMD = 0x41; //网头工作状态命令
const int PRINTSWITCHSTATUSCMD = 0x42; //网头开关状态命令
const int PRINTXSTATUSCMD = 0x43; //网头横向位置命令
const int PRINTYSTATUSCMD = 0x44; //网头斜向位置命令
const int PRINTZSTATUSCMD = 0x45; //网头高度位置命令

const int PRINTOPCMD = 0x51; //网头动作命令
const int STARTWORKCMD = 0x52; //主机启动命令
const int REQZEROCMD = 0x53;//界面请求网头对
const int REQCIRCLECMD = 0x54;//界面请求网头反转
const int SETMACHSTATUSCMD = 0x55;//设置网头状态
const int CHGMAGICCMD = 0x58;//修改磁力大小
const int CHGMAGICMODE = 0x18;//修改磁台模式

const int QUERYSPEEDCMD = 0x61; //查询主机速度命令
const int QUERYMACHSTATUSCMD = 0x62; //查询机器状态命令
const int QUERYXSTATUSCMD = 0x63; //查询工控机保持的X位置
const int QUERYYSTATUSCMD = 0x64; //查询工控机保持的Y位置
const int QUERYZSTATUSCMD = 0x65; //查询工控机保持的Z位置

const int PRINTCAUTIONCMD = 0x71; //网头故障命令应答
const int ZEROIOCAUTIONCMD = 0x72; //对零IO板故障命令应答
const int OUTIOCAUTIONCMD = 0x73; //对外IO板故障命令应答
const int BUMPIOCAUTIONCMD = 0x74; //浆泵IO板故障命令应答
const int MAGICIOCAUTIONCMD = 0x75; //磁台IO板故障命令

const int ENMACHWORKCMD = 0x81; //允许机器开机命令
const int CHGMACHSPEEDCMD = 0x82; //向主机调速命令
const int ERRSTOPCMD = 0x83; //向主机报错停机命令
const int CAUTIONCMD = 0x84; //向主机告警
const int WORKFINISHEDCMD = 0x85;//产量信号
const int HALTCMD = 0x86; //向主机上报急停
const int MAINMOTOR = 0x87; //主电机启停

const int MACHWORKACKCMD = 0x91;//主机开车请求应答
const int MACHCAUTIONACKCMD = 0x92;//主机报警应答
const int MACHERRSTOPACKCMD = 0x93;//主机报错应答
const int MACHSTOPACKCMD = 0x94;//主机停车应答
const int MACHHALTCMD = 0x95;//外部紧停信号

const int MAGICFULLCMD = 0xa1;//磁台满输出命令
const int NOCMD = 0xff;//无命令


//板子id
const int PCCANBDID = 0x20;//PC的CAN卡板子

//const int ZEROIOBDID = 0x10;//对零板子
const int BUMPIOBDID = 0x40;//浆泵板子
const int MAGICIOBDID = 0x50;//磁台板子
//const int OUTIOBDID = 0x30;//对外板子

const int PRINT1BDID = 0x00;//网头板子1
const int PRINT2BDID = 0x01;//网头板子2
const int PRINT3BDID = 0x02;//网头板子3
const int PRINT4BDID = 0x03;//网头板子4
const int PRINT5BDID = 0x04;//网头板子5
const int PRINT6BDID = 0x05;//网头板子6
const int PRINT7BDID = 0x06;//网头板子7
const int PRINT8BDID = 0x07;//网头板子8
const int PRINT9BDID = 0x08;//网头板子9
const int PRINT10BDID = 0x09;//网头板子10
const int PRINT11BDID = 0x0a;//网头板子11
const int PRINT12BDID = 0x0b;//网头板子12
const int PRINT13BDID = 0x0c;//网头板子13
const int PRINT14BDID = 0x0d;//网头板子14
const int PRINT15BDID = 0x0e;//网头板子15
const int PRINT16BDID = 0x0f;//网头板子16

//故障位置定义
const int PCCAUTIONLOCATION = 0; //PC故障位置
const int A4NCAUTIONLOCATION = 1; //运动卡故障位置
const int CANCAUTIONLOCATION = 2; //CAN故障位置
const int PLCLOCATION = 3; //PLC故障位置
const int MAGICIOLOCATION = 5; //磁台IO板故障位置
const int BUMPIOLOCATION = 6; //浆泵IO板故障位置
const int PRINT1LOCATION = 7; //网头1故障位置
const int PRINT2LOCATION = 8; //网头2故障位置
const int PRINT3LOCATION = 9; //网头3故障位置
const int PRINT4LOCATION = 10; //网头4故障位置
const int PRINT5LOCATION = 11; //网头5故障位置
const int PRINT6LOCATION = 12; //网头6故障位置
const int PRINT7LOCATION = 13; //网头7故障位置
const int PRINT8LOCATION = 14; //网头8故障位置
const int PRINT9LOCATION = 15; //网头9故障位置
const int PRINT10LOCATION = 16; //网头10故障位置
const int PRINT11LOCATION = 17; //网头11故障位置
const int PRINT12LOCATION = 18; //网头12故障位置
const int PRINT13LOCATION = 19; //网头13故障位置
const int PRINT14LOCATION = 20; //网头14故障位置
const int PRINT15LOCATION = 21; //网头15故障位置
const int PRINT16LOCATION = 22; //网头16故障位置
//const int A4N = 24; //A4N系统位置
//const int CANNETWORK = 25; //CAN网络位置
const int SERIAL = 4; //串口

//故障编码定义
const int CAUTION = 0; //默认故障
const int COMCAUTION = 1; //通讯故障
const int VERCAUTION = 2; //版本故障
const int PARCAUTION = 3; //参数故障
const int ZEROCAUTION = 4; //对零故障
const int PRINTBUMPCAUTION = 5; //网头浆泵故障
const int PRINTMOTORCAUTION = 6; //网头伺服故障

const int PLCCAUTION = 7; //主机故障
const int PLCHALTCATION = 8; //主机急停错误

const int PCNOA4NCAUTION = 9; //PC机上无运动卡故障
const int PCNOCANCATION = 10; //PC机上无CAN卡故障
const int PCSYSCATION = 11; //PC机系统故障
const int PCERRCANCMD = 12;//错误的CAN命令

const int ZEROANSOUTTIME = 13;//对零IO板应答超时
const int MAGICANSOUTTIME = 14;//磁台控制板应答超时
const int BUMPANSOUTTIME = 15;//浆泵控制板应答超时
const int ZEROSIGNOUTTIME = 16;//零位信号超时
const int XMOVMAX = 17;//横向对花上限位
const int XMOVMIN = 18;//横向对花下限位
const int YMOVMAX = 19;//斜向对花上限位
const int YMOVMIN = 20;//斜向对花下限位
const int XMOTORBOOTERR = 21;//横向电机启动错误
const int YMOTORBOOTERR = 22;//斜向电机启动错误
const int EPRROMERR = 23;//EEPROM读写错误
const int A4NANSOUTTIME = 24;//运动控制卡应答超时
const int REPIDCAUTION = 25; //重复的板子CAN ID
const int SERIALNOEXIST = 26; //串口不存在
const int SERIALLOST = 27; //串口离线
const int CANNETCAUTION = 28; //CAN网络故障


//事件编码定义
const int SENDCMDFAILEVENT = 1;//发送命令失败
const int WAITANSOUTTIMEEVENT = 2;//接受超时
const int BDONLINEEVENT = 3;//上线事件
const int BDOFFLINEEVENT = 4;//离线事件
const int UNKNOWCMDEVENT = 5;//不认识命令
const int TOOMUCHUNUSEDCMDEVENT = 6;//过多未处理的命令

//常用的常量
const int OUTTIME = 20; //CAN命令等待超时时间 20 * 100 ms
const int MOTORSTOPTIME = 600;//马达停转等待时间 600 * 100 ms
const int HEARTBEATTIME = 100; //心跳测试间隔时间 100 * 100 ms
const int ZEROOUTTIME = 600; //对零通讯等待时间 600 * 100ms
const int UNKNOWCMDNUM = 1000;//接收缓存未处理命令数量，达到此数量要告警
const int ENCODERMETER = 1; //编码器和米数的系数，测试阶段使用
const int CANOUTTIME = 1000; //CAN数据时间最大生存时间,1000*100 ms 
const int ZEROWIDTH = 20;    //对零传感器的宽度
const int MODBUSBITHOLDTIME = 20; //modbus设置位维持时间长度 20*100ms

#endif /* CANIDCMD_H_ */
