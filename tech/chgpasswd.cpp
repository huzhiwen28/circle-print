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
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <iostream>
#include "chgpasswd.h"

chgpasswd::chgpasswd(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	ui.lineEdit->setEchoMode (QLineEdit::Password);
	ui.lineEdit_2->setEchoMode (QLineEdit::Password);
	ui.lineEdit_3->setEchoMode (QLineEdit::Password);

	//事件过滤器
	ui.lineEdit->installEventFilter(this);
	ui.lineEdit_2->installEventFilter(this);
	ui.lineEdit_3->installEventFilter(this);
	oldnewflag = 0;
}

chgpasswd::~chgpasswd()
{

}

bool chgpasswd::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == ui.lineEdit) 
	{
		if (ev->type() == QEvent::FocusIn) 
		{
           oldnewflag = 0;
		}
	}
	else if (obj == ui.lineEdit_2) 
	{
		if (ev->type() == QEvent::FocusIn) 
		{
           oldnewflag = 1;
		}
	}
	else if (obj == ui.lineEdit_3)
	{
		if (ev->type() == QEvent::FocusIn) 
		{
           oldnewflag = 2;
		}
	}

	return QDialog::eventFilter(obj,ev);

}


void chgpasswd::on_pb1_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("1");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("1");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("1");
	}
}
void chgpasswd::on_pb2_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("2");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("2");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("2");
	}
}
void chgpasswd::on_pb3_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("3");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("3");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("3");
	}
}
void chgpasswd::on_pb4_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("4");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("4");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("4");
	}
}

void chgpasswd::on_pb5_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("5");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("5");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("5");
	}
}
void chgpasswd::on_pb6_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("6");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("6");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("6");
	}
}
void chgpasswd::on_pb7_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("7");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("7");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("7");
	}
}

void chgpasswd::on_pb8_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("8");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("8");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("8");
	}
}
void chgpasswd::on_pb9_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("9");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("9");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("9");
	}
}
void chgpasswd::on_pb0_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->insert("0");
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->insert("0");
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->insert("0");
	}
}

void chgpasswd::on_pbback_clicked()
{
	if(oldnewflag == 0)
{
	ui.lineEdit->backspace();
}
else if(oldnewflag == 1)
{
	ui.lineEdit_2->backspace();
}
else if(oldnewflag == 2)
{
	ui.lineEdit_3->backspace();
}
	}

void chgpasswd::on_pbclear_clicked()
{
	if(oldnewflag == 0)
	{
		ui.lineEdit->clear();
	}
	else if(oldnewflag == 1)
	{
		ui.lineEdit_2->clear();
	}
	else if(oldnewflag == 2)
	{
		ui.lineEdit_3->clear();
	}
}

void chgpasswd::on_pbcancel_clicked()
{
	flag = 1;
	this->accept();
}

void chgpasswd::on_pbok_clicked()
{
	flag = 2;

	oldpassword = ui.lineEdit->text();
	newpassword = ui.lineEdit_2->text();
	newpassword2 = ui.lineEdit_3->text();

	this->accept();
}

void chgpasswd::setRole(int Role)
{
	if(Role == 1)
	{
		ui.label_5->setText("6位密码格式 1*****");
	}
	else if(Role == 2)
	{
		ui.label_5->setText("6位密码格式 2*****");
	}
	else if(Role == 3)
	{
		ui.label_5->setText("6位密码格式 3*****");
	}
}









