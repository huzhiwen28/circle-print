#ifndef MEASUREPULSE_H
#define MEASUREPULSE_H

#include <QtGui/QDialog>
#include "ui_measurepulse.h"
#include "QString"
#include "tech.h"

class measurepulse : public QDialog
{
	Q_OBJECT

public:
	measurepulse(QWidget *parent = 0);
	~measurepulse();

public slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();
	void on_pushButton_4_clicked();
	void on_pushButton_5_clicked();
	void on_pushButton_6_clicked();
	void on_pushButton_7_clicked();
	void on_pushButton_8_clicked();
	void lineEdit_clicked(int);

private:
	Ui::measurepulseClass ui;


	le* lineEdit[3];

	//输入的长度
	double meaure[3];

	//编码器
	double encoder[3];

	//第几次测试
	int step;

protected:
	void customEvent(QEvent *e);

};

#endif // MEASUREPULSE_H