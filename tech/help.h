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

#ifndef HELP_H
#define HELP_H

#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QPen>
#include <QTime>
#include <QFile>
#include <QLineEdit>
#include "ui_help.h"
#include <QLabel>
class pdfshow: public QWidget
{
	Q_OBJECT

public:
	pdfshow(QWidget *parent = 0)		;
	~	pdfshow();
	void addBeginy();
	void decBeginy();

	int getiBeginy();
	int getiCurrentPage();
	bool getbCrossPage();
	void setiBeginy(int y);
	void setiCurrentPage(int i);
	void setbCrossPage(bool b);

protected:
	void paintEvent(QPaintEvent *);
private:
	int iViewWidth,iViewHeight;
	int iBeginy;
	int iPageNum;
	int iCurrentPage;//当前页号 0起始
	bool bCrossPage; //是否跨页，在目前情况只可能有跨一页的情况，不会跨多页
	double page_width, page_height;
	int logpixelsx, logpixelsy, size_x, size_y;

};

class help : public QDialog
{
    Q_OBJECT

public:
    help(QWidget *parent = 0);
    ~help();

	void init(int modright);

	QTimer mytimer;
	QTimer* timer;

	int timecount;

	QImage* image2;

	//绘制网头编号字体
	QFont f;

	//最上面的bar，显示故障等信息
	QLabel* topbar;

	//日期时间
	QLabel* datetime;

	//故障字符显示
	QLabel* Cautiondisplay;
	pdfshow* ppdfshow;

private:
    Ui::helpClass ui;
	bool downflag;
	bool upflag;
	int count;

public slots:

	//按钮
	void on_pushButton_6_clicked();

	void timeupdate();//定时处理的任务

protected:
	void mousePressEvent ( QMouseEvent * event );
	void paintEvent(QPaintEvent *);



private slots:
	void on_pushButton_7_pressed();
	void on_pushButton_7_released();

	void on_pushButton_8_pressed();
	void on_pushButton_8_released();
};

#endif // HELP_H
