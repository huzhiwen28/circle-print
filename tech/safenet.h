//���ܹ�����ӿ�
#ifndef SAFENET_H
#define SAFENET_H
#include    "SentinelKeys.h"  /*  �����֤��ͷ�ļ�  */

class safenet
{
public:
	safenet();
	~safenet();

	//�������
	int open();

	//�ر������
	int close();

	//������
	int readdata(char* data,int offset,int len);

	//д����
	int writedata(char* data,int offset,int len);

	//AES����-Ӧ��
	int AESreqack();

	//AES����
	int AESEncrypt(unsigned  char* plain,unsigned char* cipher,unsigned int len);

	//AES����
	int AESDecrypt(unsigned char* cipher,unsigned char* plain,unsigned int len);

	//ȡ������
	int GetDogDate(int* year,int* mouth,int* day);

	//ECCǩ������ʱ���ṩ
	
	//ECCУ�飬��ʱ���ṩ

protected:

private:

	/* ʥ�칷 API �ı��� */
	SP_DWORD     status;              /* ʥ�칷 API ������ */

	/* ʥ�칷��Ϣ */
	SP_DWORD    DeveloperID;          /* ʥ�칷������ ID */
	SP_DWORD    flags;                /* ���֤��־  */

	SP_HANDLE   licHandle;            /* ���֤���  */
};
#endif