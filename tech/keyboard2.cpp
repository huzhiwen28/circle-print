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

#include "keyboard2.h"
#include <iostream>

#include "tech.h"
#include "caution.h"
#include <QPixmap>
#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QLinearGradient>


extern QString pd2dlgbkcolor = QString("background-color:rgb(25, 226, 200)");
QPicture topbarpic2;
QPainter pdpainter3;
QPainter pdpainter4;

keyboard2::keyboard2(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	//ui.lineEdit->setEchoMode (QLineEdit::Password);

	usertitle = new QLabel(this);
	usertitle->setPixmap(QPixmap(":/images/usertitle.jpg"));//加载密码输入框胖的用户图片
	usertitle->setGeometry(QRect(5, 80, 50, 50));

	padtopbar3 = new QLabel(this);
	padtopbar3->setGeometry(QRect(0,0,240,27));
	padtopbar3->setPicture(topbarpic2);

	QLinearGradient topbarbrush1(QPointF(0, 0), QPointF(240, 27));
	topbarbrush1.setColorAt(0, QColor(68, 182, 202));
	topbarbrush1.setColorAt(1, QColor(25, 226, 200));

	pdpainter3.begin(&topbarpic2); // paint in picture
	pdpainter3.setBrush(topbarbrush1);
	pdpainter3.setPen(QPen(QColor(68, 182, 202), 3));
	pdpainter3.drawRect(-1, -2, 240, 27);
	pdpainter3.end();

	padtopbar4 = new QLabel(this);
	padtopbar4->setGeometry(QRect(240,0,443,27));
	padtopbar4->setPicture(topbarpic2);

	QLinearGradient topbarbrush2(QPointF(240, 0), QPointF(443,27));
	topbarbrush2.setColorAt(0, QColor(25, 226, 200));
	topbarbrush2.setColorAt(1, QColor(25, 226, 200));

	pdpainter4.begin(&topbarpic2); // paint in picture
	pdpainter4.setBrush(topbarbrush2);
	pdpainter4.setPen(QPen(QColor(68, 182, 202), 3));
	pdpainter4.drawRect(240, 0, 443, 27);
	pdpainter4.end();

	userpsw3 = new QLabel( padtopbar3 );
	userpsw3->setAlignment(Qt::AlignBottom|Qt::AlignRight);
	userpsw3->setMargin(8); 
	userpsw3->setText( tr("参数录入") );

	userpsw4 = new QLabel( padtopbar4 );
	userpsw4->setAlignment(Qt::AlignBottom|Qt::AlignRight);
	userpsw4->setMargin(8); 
	userpsw4->setText( tr("参数输入键盘") );
    //userpsw4->setFont(QFont("Times",18,QFont::Bold));

	ui.pb1->setStyleSheet( pd2dlgbkcolor );
	ui.pb2->setStyleSheet( pd2dlgbkcolor );
	ui.pb3->setStyleSheet( pd2dlgbkcolor );
	ui.pb4->setStyleSheet( pd2dlgbkcolor ); 
	ui.pb5->setStyleSheet( pd2dlgbkcolor );
	ui.pb6->setStyleSheet( pd2dlgbkcolor );
	ui.pb7->setStyleSheet( pd2dlgbkcolor );
	ui.pb8->setStyleSheet( pd2dlgbkcolor );
	ui.pb9->setStyleSheet( pd2dlgbkcolor );
	ui.pb0->setStyleSheet( pd2dlgbkcolor );
	ui.pbdot->setStyleSheet( pd2dlgbkcolor );
	ui.pbback->setStyleSheet( pd2dlgbkcolor );

}

keyboard2::~keyboard2()
{

}

void keyboard2::on_pb1_clicked()
{
	ui.lineEdit->insert("1");
}
void keyboard2::on_pb2_clicked()
{
	ui.lineEdit->insert("2");
	}
void keyboard2::on_pb3_clicked()
{
	ui.lineEdit->insert("3");
	}
void keyboard2::on_pb4_clicked()
{
	ui.lineEdit->insert("4");
	}
void keyboard2::on_pb5_clicked()
{
	ui.lineEdit->insert("5");
	}
void keyboard2::on_pb6_clicked()
{
	ui.lineEdit->insert("6");
	}
void keyboard2::on_pb7_clicked()
{
	ui.lineEdit->insert("7");
	}
void keyboard2::on_pb8_clicked()
{
	ui.lineEdit->insert("8");
	}
void keyboard2::on_pb9_clicked()
{
	ui.lineEdit->insert("9");
	}
void keyboard2::on_pb0_clicked()
{
	ui.lineEdit->insert("0");
	}

void keyboard2::on_pbdot_clicked()
{
	ui.lineEdit->insert(".");
}

void keyboard2::on_pbback_clicked()
{
	ui.lineEdit->backspace();
	}

void keyboard2::on_pbcancel_clicked()
{
	flag = 1;
	this->accept();
	}


void keyboard2::on_pbok_clicked()
{
	flag = 2;
	enter = ui.lineEdit->text();
	this->accept();
}

void keyboard2::setText(QString text)
{
	ui.lineEdit->setText(text);
}





void keyboard2::on_pb1_2_clicked()
{
	ui.lineEdit->insert("-");
}