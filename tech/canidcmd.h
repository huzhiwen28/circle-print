/*
 * canidcmd.h
 *
 *  Created on: 2009-8-2
 *      Author: Owner
 */

#ifndef CANIDCMD_H_
#define CANIDCMD_H_
////////////////////////////////////
//CAN����ID
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
//����
////////////////////////////////////
const int ERRCMD = 0x00; //��������
const int COMTESTCMD = 0x01; //ͨѶ����
const int VERTESTCMD = 0x02; //�汾У��
const int SETBUMPNOCMD = 0x12; //���ý��ñ��
const int SETPRINTPARACMD = 0x13; //������ͷ����
const int SETBUMPIONOCMD = 0x14; //���ý���IO���
const int SETMAGICIONOCMD = 0x15; //���ô�̨IO���
const int SETMAGICANANOCMD = 0x16; //���ô�̨ģ�������
const int SETPRINTMACHPARACMD = 0x17; //������ͷ��������

const int MAGICRSPCMD = 0x21;//��̨״̬�ϱ��������Ӧ��
const int BUMPRSPCMD = 0x22;//����״̬�ϱ��������Ӧ��
const int ZEROLOCCMD = 0x23;//��λ���ϱ�

const int CHGSPEEDCMD = 0x31; //������������Ӧ��
const int ZEROCMD = 0x32; //��������Ӧ��
const int RESERVECMD = 0x33; //��ת����Ӧ��
const int YMOVECMD = 0x34; //����Ի�����Ӧ��

const int PRINTWORKSTATUSCMD = 0x41; //��ͷ����״̬����
const int PRINTSWITCHSTATUSCMD = 0x42; //��ͷ����״̬����
const int PRINTXSTATUSCMD = 0x43; //��ͷ����λ������
const int PRINTYSTATUSCMD = 0x44; //��ͷб��λ������
const int PRINTZSTATUSCMD = 0x45; //��ͷ�߶�λ������

const int PRINTOPCMD = 0x51; //��ͷ��������
const int STARTWORKCMD = 0x52; //������������
const int REQZEROCMD = 0x53;//����������ͷ��
const int REQCIRCLECMD = 0x54;//����������ͷ��ת
const int SETMACHSTATUSCMD = 0x55;//������ͷ״̬
const int CHGMAGICCMD = 0x58;//�޸Ĵ�����С
const int CHGMAGICMODE = 0x18;//�޸Ĵ�̨ģʽ

const int QUERYSPEEDCMD = 0x61; //��ѯ�����ٶ�����
const int QUERYMACHSTATUSCMD = 0x62; //��ѯ����״̬����
const int QUERYXSTATUSCMD = 0x63; //��ѯ���ػ����ֵ�Xλ��
const int QUERYYSTATUSCMD = 0x64; //��ѯ���ػ����ֵ�Yλ��
const int QUERYZSTATUSCMD = 0x65; //��ѯ���ػ����ֵ�Zλ��

const int PRINTCAUTIONCMD = 0x71; //��ͷ��������Ӧ��
const int ZEROIOCAUTIONCMD = 0x72; //����IO���������Ӧ��
const int OUTIOCAUTIONCMD = 0x73; //����IO���������Ӧ��
const int BUMPIOCAUTIONCMD = 0x74; //����IO���������Ӧ��
const int MAGICIOCAUTIONCMD = 0x75; //��̨IO���������

const int ENMACHWORKCMD = 0x81; //���������������
const int CHGMACHSPEEDCMD = 0x82; //��������������
const int ERRSTOPCMD = 0x83; //����������ͣ������
const int CAUTIONCMD = 0x84; //�������澯
const int WORKFINISHEDCMD = 0x85;//�����ź�
const int HALTCMD = 0x86; //�������ϱ���ͣ
const int MAINMOTOR = 0x87; //�������ͣ

const int MACHWORKACKCMD = 0x91;//������������Ӧ��
const int MACHCAUTIONACKCMD = 0x92;//��������Ӧ��
const int MACHERRSTOPACKCMD = 0x93;//��������Ӧ��
const int MACHSTOPACKCMD = 0x94;//����ͣ��Ӧ��
const int MACHHALTCMD = 0x95;//�ⲿ��ͣ�ź�

const int MAGICFULLCMD = 0xa1;//��̨���������
const int NOCMD = 0xff;//������


//����id
const int PCCANBDID = 0x20;//PC��CAN������

//const int ZEROIOBDID = 0x10;//�������
const int BUMPIOBDID = 0x40;//���ð���
const int MAGICIOBDID = 0x50;//��̨����
//const int OUTIOBDID = 0x30;//�������

const int PRINT1BDID = 0x00;//��ͷ����1
const int PRINT2BDID = 0x01;//��ͷ����2
const int PRINT3BDID = 0x02;//��ͷ����3
const int PRINT4BDID = 0x03;//��ͷ����4
const int PRINT5BDID = 0x04;//��ͷ����5
const int PRINT6BDID = 0x05;//��ͷ����6
const int PRINT7BDID = 0x06;//��ͷ����7
const int PRINT8BDID = 0x07;//��ͷ����8
const int PRINT9BDID = 0x08;//��ͷ����9
const int PRINT10BDID = 0x09;//��ͷ����10
const int PRINT11BDID = 0x0a;//��ͷ����11
const int PRINT12BDID = 0x0b;//��ͷ����12
const int PRINT13BDID = 0x0c;//��ͷ����13
const int PRINT14BDID = 0x0d;//��ͷ����14
const int PRINT15BDID = 0x0e;//��ͷ����15
const int PRINT16BDID = 0x0f;//��ͷ����16

//����λ�ö���
const int PCCAUTIONLOCATION = 0; //PC����λ��
const int A4NCAUTIONLOCATION = 1; //�˶�������λ��
const int CANCAUTIONLOCATION = 2; //CAN����λ��
const int PLCLOCATION = 3; //PLC����λ��
const int MAGICIOLOCATION = 5; //��̨IO�����λ��
const int BUMPIOLOCATION = 6; //����IO�����λ��
const int PRINT1LOCATION = 7; //��ͷ1����λ��
const int PRINT2LOCATION = 8; //��ͷ2����λ��
const int PRINT3LOCATION = 9; //��ͷ3����λ��
const int PRINT4LOCATION = 10; //��ͷ4����λ��
const int PRINT5LOCATION = 11; //��ͷ5����λ��
const int PRINT6LOCATION = 12; //��ͷ6����λ��
const int PRINT7LOCATION = 13; //��ͷ7����λ��
const int PRINT8LOCATION = 14; //��ͷ8����λ��
const int PRINT9LOCATION = 15; //��ͷ9����λ��
const int PRINT10LOCATION = 16; //��ͷ10����λ��
const int PRINT11LOCATION = 17; //��ͷ11����λ��
const int PRINT12LOCATION = 18; //��ͷ12����λ��
const int PRINT13LOCATION = 19; //��ͷ13����λ��
const int PRINT14LOCATION = 20; //��ͷ14����λ��
const int PRINT15LOCATION = 21; //��ͷ15����λ��
const int PRINT16LOCATION = 22; //��ͷ16����λ��
//const int A4N = 24; //A4Nϵͳλ��
//const int CANNETWORK = 25; //CAN����λ��
const int SERIAL = 4; //����

//���ϱ��붨��
const int CAUTION = 0; //Ĭ�Ϲ���
const int COMCAUTION = 1; //ͨѶ����
const int VERCAUTION = 2; //�汾����
const int PARCAUTION = 3; //��������
const int ZEROCAUTION = 4; //�������
const int PRINTBUMPCAUTION = 5; //��ͷ���ù���
const int PRINTMOTORCAUTION = 6; //��ͷ�ŷ�����

const int PLCCAUTION = 7; //��������
const int PLCHALTCATION = 8; //������ͣ����

const int PCNOA4NCAUTION = 9; //PC�������˶�������
const int PCNOCANCATION = 10; //PC������CAN������
const int PCSYSCATION = 11; //PC��ϵͳ����
const int PCERRCANCMD = 12;//�����CAN����

const int ZEROANSOUTTIME = 13;//����IO��Ӧ��ʱ
const int MAGICANSOUTTIME = 14;//��̨���ư�Ӧ��ʱ
const int BUMPANSOUTTIME = 15;//���ÿ��ư�Ӧ��ʱ
const int ZEROSIGNOUTTIME = 16;//��λ�źų�ʱ
const int XMOVMAX = 17;//����Ի�����λ
const int XMOVMIN = 18;//����Ի�����λ
const int YMOVMAX = 19;//б��Ի�����λ
const int YMOVMIN = 20;//б��Ի�����λ
const int XMOTORBOOTERR = 21;//��������������
const int YMOTORBOOTERR = 22;//б������������
const int EPRROMERR = 23;//EEPROM��д����
const int A4NANSOUTTIME = 24;//�˶����ƿ�Ӧ��ʱ
const int REPIDCAUTION = 25; //�ظ��İ���CAN ID
const int SERIALNOEXIST = 26; //���ڲ�����
const int SERIALLOST = 27; //��������
const int CANNETCAUTION = 28; //CAN�������


//�¼����붨��
const int SENDCMDFAILEVENT = 1;//��������ʧ��
const int WAITANSOUTTIMEEVENT = 2;//���ܳ�ʱ
const int BDONLINEEVENT = 3;//�����¼�
const int BDOFFLINEEVENT = 4;//�����¼�
const int UNKNOWCMDEVENT = 5;//����ʶ����
const int TOOMUCHUNUSEDCMDEVENT = 6;//����δ���������

//���õĳ���
const int OUTTIME = 20; //CAN����ȴ���ʱʱ�� 20 * 100 ms
const int MOTORSTOPTIME = 600;//���ͣת�ȴ�ʱ�� 600 * 100 ms
const int HEARTBEATTIME = 100; //�������Լ��ʱ�� 100 * 100 ms
const int ZEROOUTTIME = 600; //����ͨѶ�ȴ�ʱ�� 600 * 100ms
const int UNKNOWCMDNUM = 1000;//���ջ���δ���������������ﵽ������Ҫ�澯
const int ENCODERMETER = 1; //��������������ϵ�������Խ׶�ʹ��
const int CANOUTTIME = 1000; //CAN����ʱ���������ʱ��,1000*100 ms 
const int ZEROWIDTH = 20;    //���㴫�����Ŀ��
const int MODBUSBITHOLDTIME = 20; //modbus����λά��ʱ�䳤�� 20*100ms

#endif /* CANIDCMD_H_ */
