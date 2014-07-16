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

#include "keyboard.h"
#include "tech.h"
#include "caution.h"
#include <iostream>
#include <QDomNode>
#include <QMessageBox>
#include <QSettings>
#include <QPixmap>
#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QLinearGradient>

extern QSettings settings; 
const QDomNode GetParaByName(const QString& groupname, const QString& paraname);
const QDomNode GetParaByID(const QString& groupname, const QString& paraid);

//遍历XML树，取得某类中的一个参数
const QDomNode GetPara(const QString& groupname, const QString tag,
					   const QString& paraname);

QString passwordstyle = QString("background-color: #AA0000; font-size: 40px");
QString pddlgbkcolor = QString("background-color:rgb(25, 226, 200)");
QPicture topbarpic1;
QPainter pdpainter1;
QPainter pdpainter2;

keyboard::keyboard(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	ui.lineEdit->setEchoMode (QLineEdit::Password);

	usertitle = new QLabel(this);
	usertitle->setPixmap(QPixmap(":/images/usertitle.jpg"));//加载密码输入框胖的用户图片
	usertitle->setGeometry(QRect(5, 70, 50, 50));

	padtopbar1 = new QLabel(this);
	padtopbar1->setGeometry(QRect(0,0,240,25));
	padtopbar1->setPicture(topbarpic1);

	QLinearGradient topbarbrush1(QPointF(0, 0), QPointF(240, 25));
	topbarbrush1.setColorAt(0, QColor(68, 182, 202));
	topbarbrush1.setColorAt(1, QColor(25, 226, 200));

	pdpainter1.begin(&topbarpic1); // paint in picture
	pdpainter1.setBrush(topbarbrush1);
	pdpainter1.setPen(QPen(QColor(68, 182, 202), 3));
	pdpainter1.drawRect(-1, -2, 240, 25);
	pdpainter1.end();
    
	padtopbar2 = new QLabel(this);
	padtopbar2->setGeometry(QRect(240,0,374,25));
	padtopbar2->setPicture(topbarpic1);
    
	QLinearGradient topbarbrush2(QPointF(240, 25), QPointF(374,25));
	topbarbrush2.setColorAt(0, QColor(25, 226, 200));
	topbarbrush2.setColorAt(1, QColor(25, 226, 200));

	pdpainter2.begin(&topbarpic1); // paint in picture
	pdpainter2.setBrush(topbarbrush2);
	pdpainter2.setPen(QPen(QColor(68, 182, 202), 3));
	pdpainter2.drawRect(240, 0, 374, 25);
	pdpainter2.end();
    
	userpsw1 = new QLabel( padtopbar1 );
    userpsw1->setAlignment(Qt::AlignBottom|Qt::AlignRight);
	userpsw1->setMargin(6); 
	userpsw1->setText( tr("用户口令") );

	userpsw2 = new QLabel( padtopbar2 );
	userpsw2->setAlignment(Qt::AlignBottom|Qt::AlignRight);
	userpsw2->setMargin(6); 
	userpsw2->setText( tr("密码输入键盘") );

	ui.pb1->setStyleSheet( pddlgbkcolor );
	ui.pb2->setStyleSheet( pddlgbkcolor );
	ui.pb3->setStyleSheet( pddlgbkcolor );
	ui.pb4->setStyleSheet( pddlgbkcolor ); 
	ui.pb5->setStyleSheet( pddlgbkcolor );
	ui.pb6->setStyleSheet( pddlgbkcolor );
	ui.pb7->setStyleSheet( pddlgbkcolor );
	ui.pb8->setStyleSheet( pddlgbkcolor );
	ui.pb9->setStyleSheet( pddlgbkcolor );
	ui.pb0->setStyleSheet( pddlgbkcolor );
	ui.pbback->setStyleSheet( pddlgbkcolor );

}

keyboard::~keyboard()
{

}

void keyboard::on_pb1_clicked()
{
	ui.lineEdit->insert("1");
}
void keyboard::on_pb2_clicked()
{
	ui.lineEdit->insert("2");
	}
void keyboard::on_pb3_clicked()
{
	ui.lineEdit->insert("3");
	}
void keyboard::on_pb4_clicked()
{
	ui.lineEdit->insert("4");
	}
void keyboard::on_pb5_clicked()
{
	ui.lineEdit->insert("5");
	}
void keyboard::on_pb6_clicked()
{
	ui.lineEdit->insert("6");
	}
void keyboard::on_pb7_clicked()
{
	ui.lineEdit->insert("7");
	}
void keyboard::on_pb8_clicked()
{
	ui.lineEdit->insert("8");
	}
void keyboard::on_pb9_clicked()
{
	ui.lineEdit->insert("9");
	}
void keyboard::on_pb0_clicked()
{
	ui.lineEdit->insert("0");
}

void keyboard::on_pbback_clicked()
{
	ui.lineEdit->backspace();
}

void keyboard::on_pbcancel_clicked()
{
	flag = 1;
	QMessageBox msgBox;
	msgBox.setText("<font size = 4>您真的要取消操作吗?</font>");
	msgBox.setWindowTitle("操作取消");
	msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No );
	QAbstractButton* tb1 = msgBox.button(QMessageBox::Yes);
	tb1->setText("是");
	QAbstractButton* tb2 = msgBox.button(QMessageBox::No);
	tb2->setText("否");

	int ret = msgBox.exec();

	if ( ret == QMessageBox::Yes )
    {
		this->accept();
    }
	else
	{
         return;
	}  
}

void keyboard::on_pbok_clicked()
{
	flag = 2;
	password = ui.lineEdit->text();

    this->accept();
}

