//加密狗对外接口
#ifndef SAFENET_H
#define SAFENET_H
#include    "SentinelKeys.h"  /*  此许可证的头文件  */

class safenet
{
public:
	safenet();
	~safenet();

	//打开软件狗
	int open();

	//关闭软件狗
	int close();

	//读数据
	int readdata(char* data,int offset,int len);

	//写数据
	int writedata(char* data,int offset,int len);

	//AES请求-应答
	int AESreqack();

	//AES加密
	int AESEncrypt(unsigned  char* plain,unsigned char* cipher,unsigned int len);

	//AES解密
	int AESDecrypt(unsigned char* cipher,unsigned char* plain,unsigned int len);

	//取得日期
	int GetDogDate(int* year,int* mouth,int* day);

	//ECC签名，暂时不提供
	
	//ECC校验，暂时不提供

protected:

private:

	/* 圣天狗 API 的变量 */
	SP_DWORD     status;              /* 圣天狗 API 返回码 */

	/* 圣天狗信息 */
	SP_DWORD    DeveloperID;          /* 圣天狗开发商 ID */
	SP_DWORD    flags;                /* 许可证标志  */

	SP_HANDLE   licHandle;            /* 许可证句柄  */
};
#endif