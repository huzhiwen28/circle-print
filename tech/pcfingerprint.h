//ȡ��pc�����������кź�Ӳ�����кţ���ָ��
#ifndef PCFINGERPRINT_H
#define PCFINGERPRINT_H


class pcfingerprint
{
public:
	pcfingerprint();
	~pcfingerprint();

	//ȡ�û���ָ��,cpfingerprint:ָ���ֽ� len:ָ�Ƴ���
    int getpcfingerprint(char* cpfingerprint,int* len);
protected:

private:

};

#endif