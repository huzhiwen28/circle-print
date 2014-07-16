/****************************************************************************
 **
 ** Trolltech hereby grants a license to use the Qt/Eclipse Integration
 ** plug-in (the software contained herein), in binary form, solely for the
 ** purpose of creating code to be used with Trolltech's Qt software.
 **
 ** Qt Designer is licensed under the terms of the GNU General Public
 ** License versions 2.0 and 3.0 ("GPL License"). Trolltech offers users the
 ** right to use certain no GPL licensed software under the terms of its GPL
 ** Exception version 1.2 (http://trolltech.com/products/qt/gplexception).
 **
 ** THIS SOFTWARE IS PROVIDED BY TROLLTECH AND ITS CONTRIBUTORS (IF ANY) "AS
 ** IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 ** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 ** PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 ** OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 ** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 ** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 ** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 ** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 ** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 ** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** Since we now have the GPL exception I think that the "special exception
 ** is no longer needed. The license text proposed above (other than the
 ** special exception portion of it) is the BSD license and we have added
 ** the BSD license as a permissible license under the exception.
 **
 ****************************************************************************/
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <iostream>
#include "keyboard.h"
#include "password.h"

extern QString choosestyle;
extern QString unchoosestyle;
extern QSettings settings;

password::password(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);
	//工艺权限
	designright = settings.value("design/modright").toInt();

	//工厂权限
	factoryright = settings.value("factory/modright").toInt();

	//开发者权限
	developright = 0x000f;
}

password::~password()
{

}

//返回按钮
void password::on_pushButton_clicked()
{
	settings.sync();
	close();
}

//初始化密码设置
void password::initpass(char usertype)
{
	int xp = 80;
	int yp = 200;

	//标题显示
	if (usertype == 2 || usertype == 1)
	{
		title1 = new QLabel(this);
		title1->setGeometry(QRect(xp, yp, 181, 51));
		title1->setText("用户");
		title1->setStyleSheet(unchoosestyle);

		xp += 100;
		title2 = new QLabel(this);
		title2->setGeometry(QRect(xp, yp, 181, 51));
		title2->setText("密码设置");
		title2->setStyleSheet(unchoosestyle);
	}
	else if (usertype == 3)
	{
		title1 = new QLabel(this);
		title1->setGeometry(QRect(xp, yp, 181, 51));
		title1->setText("用户");
		title1->setStyleSheet(unchoosestyle);

		xp += 120;
		title2 = new QLabel(this);
		title2->setGeometry(QRect(xp, yp, 181, 51));
		title2->setText("密码设置");
		title2->setStyleSheet(unchoosestyle);

		xp += 200;
		title3 = new QLabel(this);
		title3->setGeometry(QRect(xp, yp, 181, 51));
		title3->setText("模块权限选择");
		title3->setStyleSheet(unchoosestyle);

	}

	yp += 50;
	xp = 80;

	if (usertype == 1)
	{
		design = new QLabel(this);
		design->setGeometry(QRect(xp, yp, 131, 51));
		design->setText("工艺：");
		design->setStyleSheet(unchoosestyle);

		eddesign = new QLineEdit(this);
		eddesign->setGeometry(QRect(xp + 130, yp + 10, 75, 20));
		eddesign->setEchoMode (QLineEdit::Password);
		eddesign->setText(settings.value("design/pass").toString());

		pbchgpass1 = new QPushButton(this);
		pbchgpass1->setGeometry(QRect(xp + 200, yp, 50, 40));
		pbchgpass1->setText("设置");
		pbchgpass1->setStyleSheet(unchoosestyle);
		connect(pbchgpass1, SIGNAL(clicked()), this, SLOT(pbchgpass1_clicked()));

	}
	else if (usertype == 2)
	{
		factory = new QLabel(this);
		factory->setGeometry(QRect(xp, yp, 131, 51));
		factory->setText("设备厂家：");
		factory->setStyleSheet(unchoosestyle);

		edfactory = new QLineEdit(this);
		edfactory->setGeometry(QRect(xp + 130, yp + 10, 75, 20));
		edfactory->setEchoMode (QLineEdit::Password);
		edfactory->setText(settings.value("factory/pass").toString());

		pbchgpass2 = new QPushButton(this);
		pbchgpass2->setGeometry(QRect(xp + 200, yp, 50, 40));
		pbchgpass2->setText("设置");
		pbchgpass2->setStyleSheet(unchoosestyle);
		connect(pbchgpass2, SIGNAL(clicked()), this, SLOT(pbchgpass2_clicked()));

	}
	else if (usertype == 3)
	{
		design = new QLabel(this);
		design->setGeometry(QRect(xp, yp, 131, 51));
		design->setText("工艺：");
		design->setStyleSheet(unchoosestyle);

		eddesign = new QLineEdit(this);
		eddesign->setGeometry(QRect(xp + 130, yp + 10, 75, 20));
		eddesign->setEchoMode (QLineEdit::Password);
		eddesign->setText(settings.value("design/pass").toString());

		pbchgpass1 = new QPushButton(this);
		pbchgpass1->setGeometry(QRect(xp + 200, yp, 50, 40));
		pbchgpass1->setText("设置");
		pbchgpass1->setStyleSheet(unchoosestyle);
		connect(pbchgpass1, SIGNAL(clicked()), this, SLOT(pbchgpass1_clicked()));

		pbdesign1 = new QPushButton(this);
		pbdesign1->setGeometry(QRect(xp + 330, yp + 10, 100, 40));
		pbdesign1->setText("网头设置");
		if ((designright & 0x01) == 0x01)
		{
			pbdesign1->setStyleSheet(choosestyle);
		}
		else
		{
			pbdesign1->setStyleSheet(unchoosestyle);
		}
		connect(pbdesign1, SIGNAL(clicked()), this, SLOT(pbdesign1_clicked()));

		pbdesign2 = new QPushButton(this);
		pbdesign2->setGeometry(QRect(xp + 440, yp + 10, 100, 40));
		pbdesign2->setText("网头操作");
		//pbdesign2->setStyleSheet(unchoosestyle);
		if (((designright >> 1) & 0x01) == 0x01)
		{
			pbdesign2->setStyleSheet(choosestyle);
		}
		else
		{
			pbdesign2->setStyleSheet(unchoosestyle);
		}
		connect(pbdesign2, SIGNAL(clicked()), this, SLOT(pbdesign2_clicked()));

		pbdesign3 = new QPushButton(this);
		pbdesign3->setGeometry(QRect(xp + 550, yp + 10, 100, 40));
		pbdesign3->setText("工艺参数");
		//pbdesign3->setStyleSheet(unchoosestyle);
		if (((designright >> 2) & 0x01) == 0x01)
		{
			pbdesign3->setStyleSheet(choosestyle);
		}
		else
		{
			pbdesign3->setStyleSheet(unchoosestyle);
		}
		connect(pbdesign3, SIGNAL(clicked()), this, SLOT(pbdesign3_clicked()));

		pbdesign4 = new QPushButton(this);
		pbdesign4->setGeometry(QRect(xp + 660, yp + 10, 100, 40));
		pbdesign4->setText("密码设置");
		//pbdesign4->setStyleSheet(unchoosestyle);
		if (((designright >> 3) & 0x01) == 0x01)
		{
			pbdesign4->setStyleSheet(choosestyle);
		}
		else
		{
			pbdesign4->setStyleSheet(unchoosestyle);
		}
		connect(pbdesign4, SIGNAL(clicked()), this, SLOT(pbdesign4_clicked()));
		yp += 50;

		factory = new QLabel(this);
		factory->setGeometry(QRect(xp, yp, 131, 51));
		factory->setText("设备厂家：");
		factory->setStyleSheet(unchoosestyle);

		edfactory = new QLineEdit(this);
		edfactory->setGeometry(QRect(xp + 130, yp + 10, 75, 20));
		edfactory->setEchoMode (QLineEdit::Password);
		edfactory->setText(settings.value("factory/pass").toString());

		pbchgpass2 = new QPushButton(this);
		pbchgpass2->setGeometry(QRect(xp + 200, yp, 50, 40));
		pbchgpass2->setText("设置");
		pbchgpass2->setStyleSheet(unchoosestyle);
		connect(pbchgpass2, SIGNAL(clicked()), this, SLOT(pbchgpass2_clicked()));

		pbfactory1 = new QPushButton(this);
		pbfactory1->setGeometry(QRect(xp + 330, yp + 10, 100, 40));
		pbfactory1->setText("网头设置");
		//pbfactory1->setStyleSheet(unchoosestyle);
		if ((factoryright & 0x01) == 0x01)
		{
			pbfactory1->setStyleSheet(choosestyle);
		}
		else
		{
			pbfactory1->setStyleSheet(unchoosestyle);
		}
		connect(pbfactory1, SIGNAL(clicked()), this, SLOT(pbfactory1_clicked()));

		pbfactory2 = new QPushButton(this);
		pbfactory2->setGeometry(QRect(xp + 440, yp + 10, 100, 40));
		pbfactory2->setText("网头操作");
		//pbfactory2->setStyleSheet(unchoosestyle);
		if (((factoryright >> 1) & 0x01) == 0x01)
		{
			pbfactory2->setStyleSheet(choosestyle);
		}
		else
		{
			pbfactory2->setStyleSheet(unchoosestyle);
		}
		connect(pbfactory2, SIGNAL(clicked()), this, SLOT(pbfactory2_clicked()));

		pbfactory3 = new QPushButton(this);
		pbfactory3->setGeometry(QRect(xp + 550, yp + 10, 100, 40));
		pbfactory3->setText("工艺参数");
		//pbfactory3->setStyleSheet(unchoosestyle);
		if (((factoryright >> 2) & 0x01) == 0x01)
		{
			pbfactory3->setStyleSheet(choosestyle);
		}
		else
		{
			pbfactory3->setStyleSheet(unchoosestyle);
		}
		connect(pbfactory3, SIGNAL(clicked()), this, SLOT(pbfactory3_clicked()));

		pbfactory4 = new QPushButton(this);
		pbfactory4->setGeometry(QRect(xp + 660, yp + 10, 100, 40));
		pbfactory4->setText("密码设置");
		//pbfactory4->setStyleSheet(unchoosestyle);
		if (((factoryright >> 3) & 0x01) == 0x01)
		{
			pbfactory4->setStyleSheet(choosestyle);
		}
		else
		{
			pbfactory4->setStyleSheet(unchoosestyle);
		}
		connect(pbfactory4, SIGNAL(clicked()), this, SLOT(pbfactory4_clicked()));
	}
	else
	{

	}
}

void password::pbdesign1_clicked()
{
	if ((designright & 0x01) == 0x01)
	{
		designright = designright & (~0x01);
		pbdesign1->setStyleSheet(unchoosestyle);
		settings.setValue("design/modright", designright);
	}
	else
	{
		designright = designright | 0x01;
		pbdesign1->setStyleSheet(choosestyle);
		settings.setValue("design/modright", designright);
	}
}
void password::pbdesign2_clicked()
{
	if (((designright >> 1) & 0x01) == 0x01)
	{
		designright = designright & (~(0x01 << 1));
		pbdesign2->setStyleSheet(unchoosestyle);
		settings.setValue("design/modright", designright);
	}
	else
	{
		designright = designright | (0x01 << 1);
		pbdesign2->setStyleSheet(choosestyle);
		settings.setValue("design/modright", designright);
	}
}
void password::pbdesign3_clicked()
{
	if (((designright >> 2) & 0x01) == 0x01)
	{
		designright = designright & (~(0x01 << 2));
		pbdesign3->setStyleSheet(unchoosestyle);
		settings.setValue("design/modright", designright);
	}
	else
	{
		designright = designright | (0x01 << 2);
		pbdesign3->setStyleSheet(choosestyle);
		settings.setValue("design/modright", designright);
	}
}
void password::pbdesign4_clicked()
{
	if (((designright >> 3) & 0x01) == 0x01)
	{
		designright = designright & (~(0x01 << 3));
		pbdesign4->setStyleSheet(unchoosestyle);
		settings.setValue("design/modright", designright);
	}
	else
	{
		designright = designright | (0x01 << 3);
		pbdesign4->setStyleSheet(choosestyle);
		settings.setValue("design/modright", designright);
	}
}
void password::pbfactory1_clicked()
{
	if (((factoryright >> 0) & 0x01) == 0x01)
	{
		factoryright = factoryright & (~(0x01 << 0));
		pbfactory1->setStyleSheet(unchoosestyle);
		settings.setValue("factory/modright", factoryright);
	}
	else
	{
		factoryright = factoryright | (0x01 << 0);
		pbfactory1->setStyleSheet(choosestyle);
		settings.setValue("factory/modright", factoryright);
	}
}
void password::pbfactory2_clicked()
{
	if (((factoryright >> 1) & 0x01) == 0x01)
	{
		factoryright = factoryright & (~(0x01 << 1));
		pbfactory2->setStyleSheet(unchoosestyle);
		settings.setValue("factory/modright", factoryright);
	}
	else
	{
		factoryright = factoryright | (0x01 << 1);
		pbfactory2->setStyleSheet(choosestyle);
		settings.setValue("factory/modright", factoryright);
	}
}
void password::pbfactory3_clicked()
{
	if (((factoryright >> 2) & 0x01) == 0x01)
	{
		factoryright = factoryright & (~(0x01 << 2));
		pbfactory3->setStyleSheet(unchoosestyle);
		settings.setValue("factory/modright", factoryright);
	}
	else
	{
		factoryright = factoryright | (0x01 << 2);
		pbfactory3->setStyleSheet(choosestyle);
		settings.setValue("factory/modright", factoryright);
	}

}
void password::pbfactory4_clicked()
{
	if (((factoryright >> 3) & 0x01) == 0x01)
	{
		factoryright = factoryright & (~(0x01 << 3));
		pbfactory4->setStyleSheet(unchoosestyle);
		settings.setValue("factory/modright", factoryright);
	}
	else
	{
		factoryright = factoryright | (0x01 << 3);
		pbfactory4->setStyleSheet(choosestyle);
		settings.setValue("factory/modright", factoryright);
	}

}

void password::pbdevelop1_clicked()
{
	;
}
void password::pbdevelop2_clicked()
{
	;
}
void password::pbdevelop3_clicked()
{
	;
}
void password::pbdevelop4_clicked()
{
	;
}

void password::pbchgpass1_clicked()
{
	keyboard keyboardinst;
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//取消canel
	if (keyboardinst.flag == 1)
	{
		;
	}//确认
	else if (keyboardinst.flag == 2)
	{
		settings.setValue("design/pass",keyboardinst.password);
		eddesign->setText(keyboardinst.password);

	}
}
void password::pbchgpass2_clicked()
{
	keyboard keyboardinst;
	keyboardinst.setWindowFlags(Qt::CustomizeWindowHint);
	keyboardinst.exec();

	//取消canel
	if (keyboardinst.flag == 1)
	{
		;
	}//确认
	else if (keyboardinst.flag == 2)
	{
		settings.setValue("factory/pass",keyboardinst.password);
		edfactory->setText(keyboardinst.password);
	}
}
