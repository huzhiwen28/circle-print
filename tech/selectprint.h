#ifndef SELECTPRINT_H
#define SELECTPRINT_H

#include <QtGui/QDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include "ui_selectprint.h"
#include "QString"
#include "tech.h"

class selectprint : public QDialog
{
	Q_OBJECT

public:
	selectprint(QWidget *parent = 0);
	~selectprint();

	//�����ϵ���ͷ״̬
	unsigned char UIprintstatus[16];


	//��ͷ��ǰ״̬
	QLabel* lbprintstatus[16];

	//��ͷ���޸İ�ť
	pb* sleeppb[16];
	pb* workpb[16];
	pb* preparepb[16];

	//�߿�,������ͷѡ���˵������
	QFrame* textframe;
	QLabel* opprinttext;

	//���
	QTableWidget* table[1];

	//���Ԫ
	QTableWidgetItem * tableitem[16][4];

	//����б�����
	QTableWidgetItem * ctablehead[4];
    
	//����б�����
	QTableWidgetItem * rtablehead[16];

private:
	//����״̬����Ҫ���õ�״̬
	unsigned char cmdstatus;

	//������ͷ���
	int cmdprintnum;

	//�ı��ʶ
	bool printstatuschgflag;

public slots:

	void init();//�����ʼ��

	void on_cancelButton_clicked();

	//��ͷ���߰�ť
	void sleep_clicked(int num);

	//��ͷ������ť
	void work_clicked(int num);
 
	//��ͷ׼����ť
	void prepare_clicked(int num);

protected:
	void customEvent(QEvent *e);

private:
	Ui::selectprintClass ui;


private slots:
	void on_cancelButton_2_clicked();
};

#endif // SELECTPRINT_H