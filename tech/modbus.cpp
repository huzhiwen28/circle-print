#include <Windows.h>
#include <iostream>
#include <QStringList>

#include "modbus.h"
#include "tech.h"
#include "canif.h"
extern struct _regs regs;


//����1.5Tʱ�䶨��
void set_modbusRTU_slave_t15(struct _modbusRTU* modbus,unsigned int t)
{
	modbus->t15 = t;

}

//����3.5Tʱ�䶨��
void set_modbusRTU_slave_t35(struct _modbusRTU* modbus,unsigned int t)
{
	modbus->t35 = t;

}

//����Ӧ����ʱ��
void set_modbusRTU_slave_answaittime(struct _modbusRTU* modbus,unsigned int t)
{
	modbus->answaittime = t;
}

//����modbus id
void set_modbusRTU_slave_ModbusID(struct _modbusRTU* modbus,unsigned int t)
{
	modbus->modbusid = t;
}

//�򿪴����豸
bool modbusRTU_slave_SerialOpen(struct _modbusRTU* modbus)
{
	modbus->com.devmutex.lock();

	char serialdevice[10] = "com";
	serialdevice[3]  = '1' + modbus->com.Name;

	//���ò��򿪶˿�
	modbus->com.hCom = CreateFileA(serialdevice, GENERIC_READ
		| GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (modbus->com.hCom == (HANDLE) - 1)
	{
		modbus->com.devmutex.unlock();
		return false;
	}
	else
	{
		DCB wdcb;
		GetCommState(modbus->com.hCom, &wdcb);

		wdcb.BaudRate = modbus->com.BaudRate;
		wdcb.ByteSize = modbus->com.ByteSize;
		wdcb.Parity = modbus->com.Parity;
		wdcb.StopBits = modbus->com.StopBits;
		wdcb.fRtsControl = modbus->com.fRtsControl;

		SetCommState(modbus->com.hCom, &wdcb);
		PurgeComm(modbus->com.hCom, PURGE_TXCLEAR|PURGE_RXCLEAR);


		//�첽Ҫ���ó�ʱʱ��
		COMMTIMEOUTS m_timeout;
		m_timeout.ReadIntervalTimeout = 1000;
		m_timeout.ReadTotalTimeoutConstant = 1000;
		m_timeout.ReadTotalTimeoutMultiplier = 1000;
		m_timeout.WriteTotalTimeoutConstant = 1000;
		m_timeout.WriteTotalTimeoutMultiplier =1000;
		SetCommTimeouts(modbus->com.hCom,&m_timeout);

		ResetEvent(modbus->ov1.hEvent);
		ResetEvent(modbus->ov2.hEvent);
		ResetEvent(modbus->ov3.hEvent);
		modbus->com.deviceopen = true;
	}

	modbus->com.devmutex.unlock();
	return true;
}

//�رմ����豸
bool modbusRTU_slave_SerialClose(struct _modbusRTU* modbus)
{
	modbus->com.devmutex.lock();

	if (modbus->com.deviceopen == true)
	{
		modbus->com.deviceopen = false;

		//��ֹ�ȴ�
		SetEvent(modbus->ov1.hEvent);
		SetEvent(modbus->ov2.hEvent);
		SetEvent(modbus->ov3.hEvent);

		CloseHandle(modbus->com.hCom);
	}

	modbus->com.devmutex.unlock();
	return true;
}

//���ô�������
void modbusRTU_slave_SetSerialName(struct _modbusRTU* modbus,int Name)
{
	modbus->com.Name = Name;
}

//���ô��ڲ�����
void modbusRTU_slave_SetSerialBaudRate(struct _modbusRTU* modbus,int BaudRate)
{
	modbus->com.BaudRate = BaudRate;
}

//���ô����ֽ�λ��
void modbusRTU_slave_SetSerialByteSize(struct _modbusRTU* modbus,int ByteSize)
{
	modbus->com.ByteSize = ByteSize;
}


//���ô���У�鷽ʽ
void modbusRTU_slave_SetSerialParity(struct _modbusRTU* modbus,int Parity)
{
	modbus->com.Parity = Parity;
}

//���ô���ֹͣλ
void modbusRTU_slave_SetSerialStopBits(struct _modbusRTU* modbus,int StopBits)
{
	modbus->com.StopBits = StopBits;
}

//����RTS������
void modbusRTU_slave_SetSerialfRtsControl(struct _modbusRTU* modbus,int fRtsControl)
{
	modbus->com.fRtsControl = fRtsControl;
}


// CRC��λ�ֽ�ֵ��
static unsigned char auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
		0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
		0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
		0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
		0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
		0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
		0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
		0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
		0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
		0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
		0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
		0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
		0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
		0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
		0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
		0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
		0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
		0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
		0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
		0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
		0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
		0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
		0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
		0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
		0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
		0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;


// CRC ��λ�ֽ�ֵ��
static unsigned char auchCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
		0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
		0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
		0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
		0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
		0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
		0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
		0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
		0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
		0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;

//CRCУ��ĺ���
unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen)
{
	unsigned char uchCRCHi = 0xFF ; /* ��CRC�ֽڳ�ʼ�� */
	unsigned char uchCRCLo = 0xFF ; /* ��CRC �ֽڳ�ʼ�� */
	unsigned uIndex ; /* CRCѭ���е����� */
	while (usDataLen--) /* ������Ϣ������ */
	{
		uIndex = uchCRCHi ^ *puchMsg++ ; /* ����CRC */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	return (uchCRCHi << 8 | uchCRCLo) ;
}


modbusmain::modbusmain(struct _modbusRTU* pmod)
{
	pmodbusRTU = pmod;

}

modbusmain::~modbusmain()
{
	// TODO Auto-generated destructor stub
}

//�߳���ѭ��
void modbusmain::run()
{
	LARGE_INTEGER litmp; 
	double  dfFreq; 
	QueryPerformanceFrequency(&litmp);
	dfFreq = (double)litmp.QuadPart;// ��ü�������ʱ��Ƶ��

	while (pubcom.quitthread == false)
	{
		if( pmodbusRTU->com.deviceopen == true)
		{
			//��ʼ
			if (pmodbusRTU->Status == 0)
			{
				pmodbusRTU->Status = 1;
			}
			//����
			else if(pmodbusRTU->Status == 1)
			{
				DWORD wCount = 0;
				DWORD dwError = 0;
				COMSTAT Status;

				bool success = ClearCommError(pmodbusRTU->com.hCom, &dwError, &Status);
				if (success && (Status.cbInQue > 0))
				{
					memset(pmodbusRTU->inbuff,0,256);
					pmodbusRTU->inlen = 0;
					ReadFile(pmodbusRTU->com.hCom, pmodbusRTU->inbuff + pmodbusRTU->inlen, Status.cbInQue, &wCount, &(pmodbusRTU->ov1));
					pmodbusRTU->inlen = wCount;
					//��¼ʱ��
					QueryPerformanceCounter(&(pmodbusRTU->oldtick));
					pmodbusRTU->Status = 2;
				}

			}

			//����
			else if(pmodbusRTU->Status == 2)
			{
				DWORD wCount = 0;
				DWORD dwError = 0;
				COMSTAT Status;

				bool success = ClearCommError(pmodbusRTU->com.hCom, &dwError, &Status);
				if (success && (Status.cbInQue > 0))
				{
					ReadFile(pmodbusRTU->com.hCom, pmodbusRTU->inbuff + pmodbusRTU->inlen, Status.cbInQue, &wCount, &(pmodbusRTU->ov1));
					pmodbusRTU->inlen = pmodbusRTU->inlen + wCount;

					QueryPerformanceCounter(&(pmodbusRTU->oldtick));

				}

				//��������ʱ���Ƿ񳬹�3.5T
				QueryPerformanceCounter(&(pmodbusRTU->nowtick));

				//֡����
				if ((((double)pmodbusRTU->nowtick.QuadPart - (double)pmodbusRTU->oldtick.QuadPart)/(dfFreq))*1000 > pmodbusRTU->t35)
				{
#ifdef PRINTMODBUSIN
					Dbgout("Modbus In:");
					//��ӡ�յ�������
					for (int i = 0; i< pmodbusRTU->inlen;++i)
					{
						Dbgout("%x ",pmodbusRTU->inbuff[i]);
					}
					Dbgout("\n");
#endif					
					pmodbusRTU->Status = 3;

				}
			}
			else if(pmodbusRTU->Status == 3)
			{
				DWORD wCount = 0;

				//Ŀ��Ϊ�Լ�
				if(pmodbusRTU->inbuff[0] == (unsigned char)(pmodbusRTU->modbusid))
				{

					//�������Ź�
					regs.heartbeatcnt = 0;

					//CRCУ��
					unsigned short crcresult = CRC16(pmodbusRTU->inbuff,(unsigned short)pmodbusRTU->inlen -2);
					if (crcresult == (pmodbusRTU->inbuff[pmodbusRTU->inlen-2]<<8 | pmodbusRTU->inbuff[pmodbusRTU->inlen-1] ))
					{
						//��PC���Ĵ���
						if (pmodbusRTU->inbuff[1] == 0x03)
						{
							//��ʼ�Ĵ�����ַ
							unsigned short j = (((((short)pmodbusRTU->inbuff[2]) << 8) + pmodbusRTU->inbuff[3]) - 200) * 2;

							//ID
							pmodbusRTU->outbuff[0] = (unsigned char)(pmodbusRTU->modbusid);
							//������
							pmodbusRTU->outbuff[1] = 0x03;

							//Ӧ���ֽ�����mobusЭ���޸ģ��ֽ�����Ҳ��16λ������PLC����
							pmodbusRTU->outbuff[2] = 0;
							pmodbusRTU->outbuff[3] = pmodbusRTU->inbuff[5] *2;

							int i;
							//�������ַ�
							for (i = 0;i < pmodbusRTU->outbuff[3] ; ++i)
							{
								unsigned char ch = 0;
								ReadRegs(&regs,j + i,&ch);
								pmodbusRTU->outbuff[4+ i] = ch;
							}

							//���ݰ��ܳ�
							pmodbusRTU->outlen = 4 + pmodbusRTU->outbuff[3] + 2;

							//���CRC�ַ�
							crcresult = CRC16(pmodbusRTU->outbuff,(unsigned short)pmodbusRTU->outlen -2);
							pmodbusRTU->outbuff[pmodbusRTU->outlen-2] = (unsigned char)((crcresult >> 8) & 0xff);
							pmodbusRTU->outbuff[pmodbusRTU->outlen-1] = (unsigned char)((crcresult) & 0xff);

						}
						//дPC���Ĵ���
						else if(pmodbusRTU->inbuff[1] == 0x10)
						{

							//��ʼ�Ĵ�����ַ
							unsigned short j = (((((short)pmodbusRTU->inbuff[2]) << 8) + pmodbusRTU->inbuff[3]) - 100) * 2;

							int ii;


							//д�յ������ݵ��Ĵ���
							for (ii = 0; ii < pmodbusRTU->inbuff[6]; ++ii)
							{
								WriteRegs(&regs,j+ii,pmodbusRTU->inbuff[7+ii]);
							}

							//�������ַ�
							//ID
							pmodbusRTU->outbuff[0] = (unsigned char)(pmodbusRTU->modbusid);
							//������
							pmodbusRTU->outbuff[1] = 0x10;
							//��ʼ��ַ
							pmodbusRTU->outbuff[2] = pmodbusRTU->inbuff[2];
							pmodbusRTU->outbuff[3] = pmodbusRTU->inbuff[3];

							//�Ĵ�������
							pmodbusRTU->outbuff[4] = pmodbusRTU->inbuff[4];
							pmodbusRTU->outbuff[5] = pmodbusRTU->inbuff[5];

							//���ݰ��ܳ�
							pmodbusRTU->outlen = 8;

							//���CRC�ַ�
							crcresult = CRC16(pmodbusRTU->outbuff,(unsigned short)pmodbusRTU->outlen -2);
							pmodbusRTU->outbuff[pmodbusRTU->outlen-2] = (unsigned char)((crcresult >> 8) & 0xff);
							pmodbusRTU->outbuff[pmodbusRTU->outlen-1] = (unsigned char)((crcresult) & 0xff);

						}
						//��������,ԭ���ظ�
						else 
						{
							memcpy(pmodbusRTU->outbuff,pmodbusRTU->inbuff,pmodbusRTU->inlen);
							pmodbusRTU->outlen = pmodbusRTU->inlen;
						}


						if (WriteFile(pmodbusRTU->com.hCom, pmodbusRTU->outbuff, pmodbusRTU->outlen, &wCount, &(pmodbusRTU->ov2)))
						{
						}
						else
						{
							DWORD writenum = 0;

							if (GetLastError() == ERROR_IO_PENDING)
							{
								GetOverlappedResult(pmodbusRTU->com.hCom,&(pmodbusRTU->ov2),&writenum,true);
							}
							else
							{
								Dbgout("a");
							}
						}
#ifdef PRINTMODBUSOUT
						Dbgout("Modbus Out:");
						//��ӡ���͵�����
						for (int i = 0; i< pmodbusRTU->outlen;++i)
						{
							Dbgout("%x ",pmodbusRTU->outbuff[i]);
						}
						Dbgout("\n");
#endif					
					}
					//CRCУ�����Ҳ��Ӧ��
					else
					{
						Dbgout("CRC error\n");

						//ID
						pmodbusRTU->outbuff[0] = (unsigned char)(pmodbusRTU->modbusid);

						//������
						pmodbusRTU->outbuff[1] = pmodbusRTU->inbuff[1] | 0x80;

						//�쳣��
						pmodbusRTU->outbuff[2] = 0x04;

						//���ݰ��ܳ�
						pmodbusRTU->outlen = 5;

						//���CRC�ַ�
						crcresult = CRC16(pmodbusRTU->outbuff,(unsigned short)pmodbusRTU->outlen -2);
						pmodbusRTU->outbuff[pmodbusRTU->outlen-2] = (unsigned char)((crcresult >> 8) & 0xff);
						pmodbusRTU->outbuff[pmodbusRTU->outlen-1] = (unsigned char)((crcresult) & 0xff);


						if (WriteFile(pmodbusRTU->com.hCom, pmodbusRTU->outbuff, pmodbusRTU->outlen, &wCount, &(pmodbusRTU->ov2)))
						{
						}
						else
						{
							DWORD writenum = 0;

							if (GetLastError() == ERROR_IO_PENDING)
							{
								GetOverlappedResult(pmodbusRTU->com.hCom,&(pmodbusRTU->ov2),&writenum,true);
							}
							else
							{
								Dbgout("a");
							}
						}
#ifdef PRINTMODBUSOUT
						Dbgout("Modbus Out:");
						//��ӡ���͵�����
						for (int i = 0; i< pmodbusRTU->outlen;++i)
						{
							Dbgout("%x ",pmodbusRTU->outbuff[i]);
						}
						Dbgout("\n");
#endif					
					}

					//msleep(pmodbusRTU->t35);
				}
				//Ŀ����Լ����򵥷���
				else
				{

				}

				//�������
				PurgeComm(pmodbusRTU->com.hCom, PURGE_RXCLEAR);
				memset(pmodbusRTU->inbuff,0,256);
				pmodbusRTU->Status = 1;

			}
		}
		msleep(1);
	}
}


