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

	//界面上的网头状态
	unsigned char UIprintstatus[16];


	//网头当前状态
	QLabel* lbprintstatus[16];

	//网头的修改按钮
	pb* sleeppb[16];
	pb* workpb[16];
	pb* preparepb[16];

	//边框,放置网头选择的说明文字
	QFrame* textframe;
	QLabel* opprinttext;

	//表格
	QTableWidget* table[1];

	//表格单元
	QTableWidgetItem * tableitem[16][4];

	//表格列标题栏
	QTableWidgetItem * ctablehead[4];
    
	//表格行标题栏
	QTableWidgetItem * rtablehead[16];

private:
	//命令状态，想要设置的状态
	unsigned char cmdstatus;

	//命令网头编号
	int cmdprintnum;

	//改变标识
	bool printstatuschgflag;

public slots:

	void init();//界面初始化

	void on_cancelButton_clicked();

	//网头休眠按钮
	void sleep_clicked(int num);

	//网头工作按钮
	void work_clicked(int num);
 
	//网头准备按钮
	void prepare_clicked(int num);

protected:
	void customEvent(QEvent *e);

private:
	Ui::selectprintClass ui;


private slots:
	void on_cancelButton_2_clicked();
};

#endif // SELECTPRINT_H