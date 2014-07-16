//加密狗实现
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

//打开软件狗
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

//关闭软件狗
int safenet::close()
{
	if (licHandle != NULL)
	{
		/* 使用 SFNTReleaseLicense API 函数来释放获取的许可证。 */
		status = SFNTReleaseLicense(licHandle );
		if (status != SP_SUCCESS)
		{
			/*
			如果许可证已释放，此 API 函数将返回错误。 
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

//AES加密
int safenet::AESEncrypt(unsigned char* plain,unsigned char* cipher,unsigned int len)
{
	//非16的整数倍
	if ((len%16) != 0 )
	{
		return 1;
	} 

	for (unsigned int i = 0;i < len /16; i++)
	{
		/* SFNTEncrypt API 函数使用存储于指定 FeatureID 内的密钥对 16 字节数据进行加密 */
		status = SFNTEncrypt(licHandle, SP_AES_AES, &plain[i*16], &cipher[i*16] );
		if (status != SP_SUCCESS)
		{
			/* 如果加密请求被指定到其它 FeatureID，将返回错误。
			*/
			return 1;
		}
	}
	return 0;
}

//AES解密
int safenet::AESDecrypt(unsigned char* cipher,unsigned char* plain,unsigned int len)
{
	//非16的整数倍
	if ((len%16) != 0 )
	{
		return 1;
	} 

	for (unsigned int i = 0;i < len /16; i++)
	{
		/* SFNTDecrypt API 函数使用存储于指定 FeatureID 内的密钥对 16 字节数据进行解密
		*/
		status = SFNTDecrypt(licHandle, SP_AES_AES, &cipher[i*16], &plain[i*16]); 
		if (status != SP_SUCCESS)
		{
			/*
			如果解密请求被指定到其它 FeatureID，将返回错误。
			*/
			return 1;
		}
	}
	return 0;


}

//读数据
int safenet::readdata(char* data,int offset,int len)
{
	if (licHandle != NULL)
	{
		status = SFNTReadRawData(licHandle,SP_RAW_RAWDATA,data,offset,len );
		if (status != SP_SUCCESS)
		{
			/*
			如果分配的缓冲区太小或 FeatureID 无效，则系统将返回错误。

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

//写数据
int safenet::writedata(char* data,int offset,int len)
{
	if (licHandle != NULL)
	{
		//初始化密码
		DWORD password = 0x111111;

		status = SFNTWriteRawData(licHandle,SP_RAW_RAWDATA,data,offset,len ,password);
		if (status != SP_SUCCESS)
		{
			/*
			如果分配的缓冲区太小或 FeatureID 无效，则系统将返回错误。

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

//AES请求-应答
int safenet::AESreqack()
{
	//0-1000间随机数，0-499为需要正确对比的，大于等于500的就产生无意义的数字查询
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
			如果 FeatureID 无效，此 API 函数将返回错误。
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
			如果 FeatureID 无效，此 API 函数将返回错误。
			*/
			return 1;
		}
		else
		{
			//比对返回值
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

//取得日期
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

