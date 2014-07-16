//取得pc机的主板序列号和硬盘序列号，即指纹
#ifndef PCFINGERPRINT_H
#define PCFINGERPRINT_H


class pcfingerprint
{
public:
	pcfingerprint();
	~pcfingerprint();

	//取得机器指纹,cpfingerprint:指纹字节 len:指纹长度
    int getpcfingerprint(char* cpfingerprint,int* len);
protected:

private:

};

#endif