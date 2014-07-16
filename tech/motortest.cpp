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
#include <QString>
#include <QPushbutton>
#include "motortest.h"
#include "CMotor.h"
#include "serialif.h"

extern QString choosestyle;
extern QString unchoosestyle;

//A4N电机
extern CKSMCA4 *ksmc;

motortest::motortest(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);

	//网头选择
	printchoose[0] = true;
	printchoose[1] = false;
	printchoose[2] = false;

	ui.pushButton->setStyleSheet(choosestyle);
	ui.pushButton_2->setStyleSheet(unchoosestyle);
	ui.pushButton_3->setStyleSheet(unchoosestyle);

	motorfollow = false;
	ui.pushButton_9->setStyleSheet(unchoosestyle);


	motorstatus = 0;

	roll1 = false;

	roll2 = false;

}

motortest::~motortest()
{

}
//网头1选择
void motortest::on_pushButton_clicked()
{
	if (printchoose[0] == true)
	{
		printchoose[0] = false;
		ui.pushButton->setStyleSheet(unchoosestyle);
	}
	else
	{
		printchoose[0] = true;
		ui.pushButton->setStyleSheet(choosestyle);
	}

}
//网头2选择
void motortest::on_pushButton_2_clicked()
{
	if (printchoose[1] == true)
	{
		printchoose[1] = false;
		ui.pushButton_2->setStyleSheet(unchoosestyle);
	}
	else
	{
		printchoose[1] = true;
		ui.pushButton_2->setStyleSheet(choosestyle);
	}
}
//网头3选择
void motortest::on_pushButton_3_clicked()
{
	if (printchoose[2] == true)
	{
		printchoose[2] = false;
		ui.pushButton_3->setStyleSheet(unchoosestyle);
	}
	else
	{
		printchoose[2] = true;
		ui.pushButton_3->setStyleSheet(choosestyle);
	}
}

//正转
void motortest::on_pushButton_4_clicked()
{
	if (motorstatus == 0)
	{
		motorstatus = 1;
		ui.pushButton_4->setStyleSheet(choosestyle);
		ui.pushButton_5->setEnabled(false);
		ui.pushButton_9->setEnabled(false);
		ui.pushButton_11->setEnabled(false);

		//执行命令
		if (printchoose[0] == true)
		{
			ksmc->MoveCCW(1);
		}

		if (printchoose[1] == true)
		{
			ksmc->MoveCCW(2);
		}

		if (printchoose[2] == true)
		{
			ksmc->MoveCCW(3);
		}
	}

}
//反转
void motortest::on_pushButton_5_clicked()
{
	if (motorstatus == 0)
	{
		motorstatus = 1;
		ui.pushButton_5->setStyleSheet(choosestyle);
		ui.pushButton_4->setEnabled(false);
		ui.pushButton_9->setEnabled(false);
		ui.pushButton_11->setEnabled(false);

		//执行命令
		if (printchoose[0] == true)
		{
			ksmc->MoveCCW(1);
		}

		if (printchoose[1] == true)
		{
			ksmc->MoveCCW(2);
		}

		if (printchoose[2] == true)
		{
			ksmc->MoveCCW(3);
		}
	}
}
//停止转动
void motortest::on_pushButton_6_clicked()
{
	if (motorstatus == 1)
	{
		motorstatus = 0;
		ui.pushButton_4->setStyleSheet(unchoosestyle);
		ui.pushButton_5->setStyleSheet(unchoosestyle);
		ui.pushButton_4->setEnabled(true);
		ui.pushButton_5->setEnabled(true);
		ui.pushButton_9->setEnabled(true);
		ui.pushButton_11->setEnabled(true);

		ksmc->MoveStop(1);
		ksmc->MoveStop(2);
		ksmc->MoveStop(3);
	}
}

//JOG+
void motortest::on_pushButton_7_clicked()
{
	//执行命令
	if (printchoose[0] == true)
	{
		ksmc->MoveIncPTP(1, 10);
	}

	if (printchoose[1] == true)
	{
		ksmc->MoveIncPTP(2, 10);
	}

	if (printchoose[2] == true)
	{
		ksmc->MoveIncPTP(3, 10);
	}
}
//JOG-
void motortest::on_pushButton_8_clicked()
{
	//执行命令
	if (printchoose[0] == true)
	{
		ksmc->MoveIncPTP(1, -10);
	}

	if (printchoose[1] == true)
	{
		ksmc->MoveIncPTP(2, -10);
	}

	if (printchoose[2] == true)
	{
		ksmc->MoveIncPTP(3, -10);
	}
}

//跟随
void motortest::on_pushButton_9_clicked()
{
	if (motorstatus == 0)
	{
		motorstatus = 2;

		ui.pushButton_9->setStyleSheet(choosestyle);

		ui.pushButton_4->setEnabled(false);
		ui.pushButton_5->setEnabled(false);
		ui.pushButton_6->setEnabled(false);

		//执行命令
		if (printchoose[0] == true)
		{
			ksmc->EnableMasterFollowing(0);
		}

		if (printchoose[1] == true)
		{
			ksmc->EnableMasterFollowing(1);
		}

		if (printchoose[2] == true)
		{
			ksmc->EnableMasterFollowing(2);
		}
	}
}

//停止跟随
void motortest::on_pushButton_11_clicked()
{
	if (motorstatus == 2)
	{
		motorstatus = 0;

		ui.pushButton_9->setStyleSheet(unchoosestyle);

		ui.pushButton_4->setEnabled(true);
		ui.pushButton_5->setEnabled(true);
		ui.pushButton_6->setEnabled(true);

		//执行命令
		if (printchoose[0] == true)
		{
			ksmc->DisableMasterFollowing(0);
		}

		if (printchoose[1] == true)
		{
			ksmc->DisableMasterFollowing(1);
		}

		if (printchoose[2] == true)
		{
			ksmc->DisableMasterFollowing(2);
		}
	}

}

//返回
void motortest::on_pushButton_10_clicked()
{
	close();
}


void motortest::on_pushButton_12_clicked()
{

}

void motortest::on_pushButton_13_clicked()
{

}

void motortest::on_pushButton_14_clicked()
{

}