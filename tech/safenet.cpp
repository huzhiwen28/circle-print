//���ܹ�ʵ��
#include <Windows.h>
#include <iostream>

#include "safenet.h"
#include "SentinelKeysLicense.h"

safenet::safenet()
{
	licHandle = NULL;
	DeveloperID = DEVELOPERID;
	flags = 0x20;
	status = 0;
}


safenet::~safenet()
{
	close();
}

//�������
int safenet::open()
{
	if (licHandle == NULL)
	{
		status = SFNTGetLicense(DeveloperID, SOFTWARE_KEY, LICENSEID, flags, &licHandle );
		if (status != SP_SUCCESS)
		{
			return 1;

		}
		else
		{
			return 0;
		}

	}

	return 0;
	
}

//�ر������
int safenet::close()
{
	if (licHandle != NULL)
	{
		/* ʹ�� SFNTReleaseLicense API �������ͷŻ�ȡ�����֤�� */
		status = SFNTReleaseLicense(licHandle );
		if (status != SP_SUCCESS)
		{
			/*
			������֤���ͷţ��� API ���������ش��� 
			*/
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

//AES����
int safenet::AESEncrypt(unsigned char* plain,unsigned char* cipher,unsigned int len)
{
	//��16��������
	if ((len%16) != 0 )
	{
		return 1;
	} 

	for (unsigned int i = 0;i < len /16; i++)
	{
		/* SFNTEncrypt API ����ʹ�ô洢��ָ�� FeatureID �ڵ���Կ�� 16 �ֽ����ݽ��м��� */
		status = SFNTEncrypt(licHandle, SP_AES_AES, &plain[i*16], &cipher[i*16] );
		if (status != SP_SUCCESS)
		{
			/* �����������ָ�������� FeatureID�������ش���
			*/
			return 1;
		}
	}
	return 0;
}

//AES����
int safenet::AESDecrypt(unsigned char* cipher,unsigned char* plain,unsigned int len)
{
	//��16��������
	if ((len%16) != 0 )
	{
		return 1;
	} 

	for (unsigned int i = 0;i < len /16; i++)
	{
		/* SFNTDecrypt API ����ʹ�ô洢��ָ�� FeatureID �ڵ���Կ�� 16 �ֽ����ݽ��н���
		*/
		status = SFNTDecrypt(licHandle, SP_AES_AES, &cipher[i*16], &plain[i*16]); 
		if (status != SP_SUCCESS)
		{
			/*
			�����������ָ�������� FeatureID�������ش���
			*/
			return 1;
		}
	}
	return 0;


}

//������
int safenet::readdata(char* data,int offset,int len)
{
	if (licHandle != NULL)
	{
		status = SFNTReadRawData(licHandle,SP_RAW_RAWDATA,data,offset,len );
		if (status != SP_SUCCESS)
		{
			/*
			�������Ļ�����̫С�� FeatureID ��Ч����ϵͳ�����ش���

			*/
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

//д����
int safenet::writedata(char* data,int offset,int len)
{
	if (licHandle != NULL)
	{
		//��ʼ������
		DWORD password = 0x111111;

		status = SFNTWriteRawData(licHandle,SP_RAW_RAWDATA,data,offset,len ,password);
		if (status != SP_SUCCESS)
		{
			/*
			�������Ļ�����̫С�� FeatureID ��Ч����ϵͳ�����ش���

			*/
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

//AES����-Ӧ��
int safenet::AESreqack()
{
	//0-1000���������0-499Ϊ��Ҫ��ȷ�Աȵģ����ڵ���500�ľͲ�������������ֲ�ѯ
	int index = 0;
	int multi = 32767 / 1000;

	index = rand() / multi;

	if (index >= 500)
	{
		unsigned char query[4] = {index,index,index,index};
		unsigned char response[4];
		status = SFNTQueryFeature(licHandle, SP_AES_AES, SP_SIMPLE_QUERY, query, SP_LEN_OF_QR, response, 4);
		if (status != SP_SUCCESS)
		{
			/*
			��� FeatureID ��Ч���� API ���������ش���
			*/
			return 1;
		}
		else
		{
			return 0;
		}
		
	} 
	else
	{
		unsigned char response[4];
		status = SFNTQueryFeature(licHandle, SP_AES_AES, SP_SIMPLE_QUERY, QUERY_SP_AES_AES_SP_SIMPLE_QUERY[index], SP_LEN_OF_QR, response, 4);
		if (status != SP_SUCCESS)
		{
			/*
			��� FeatureID ��Ч���� API ���������ش���
			*/
			return 1;
		}
		else
		{
			//�ȶԷ���ֵ
			if (response[0] == RESPONSE_SP_AES_AES_SP_SIMPLE_QUERY[index][0]
			&& response[1] == RESPONSE_SP_AES_AES_SP_SIMPLE_QUERY[index][1]
			&& response[2] == RESPONSE_SP_AES_AES_SP_SIMPLE_QUERY[index][2]
			&& response[3] == RESPONSE_SP_AES_AES_SP_SIMPLE_QUERY[index][3])
			{
				return 0;
			}
			else
			{
				return 1;

			}
		}
	}
}

//ȡ������
int safenet::GetDogDate(int* year,int* mouth,int* day)
{
	unsigned long int  status;
	SP_DEVICE_INFO  deviceInfo;

	status = SFNTGetDeviceInfo( licHandle,
		&deviceInfo );

	if (status != SP_SUCCESS) { 
		// If license is already released,
		// then this API will return error.
    	return 1;
	}

	(*year) = deviceInfo.timeValue.year;
	
	(*mouth) = deviceInfo.timeValue.month;
	
	(*day) = deviceInfo.timeValue.dayOfMonth;

	return 0;
}

