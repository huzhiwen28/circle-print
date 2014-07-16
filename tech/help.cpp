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
#include <QPainter>
#include "tech.h"
#include "help.h"
#include "caution.h"
#include "canif.h"
#include <iostream>
#include <QScrollBar>
#include <QPaintEngine>
#include "fpdfview.h"

QColor bgColorForName(const QString &name);
QColor fgColorForName(const QString &name);

extern QString dlgbkcolor;

extern char cautionbkflag ;
extern char cautionbkchgflag ;
extern char datetimechgflag ;
extern char cautionstrchgflag ;
//当前显示的故障id
extern char curcautionindex; 

extern QPicture topbarpic;
extern QPicture topbarredpic;
extern QPicture topbargraypic;

extern QString datetimestyle;
extern QString cautionstyle;

QString widgetbkcolor = QString("QWidget { background-color:rgb(255,255,255)}");

FPDF_DOCUMENT pdf_doc;
FPDF_PAGE pdf_page;

help::help(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	image2 = new QImage();
	image2->load(":/DSC_1422.PNG");

	f.setPointSize(30);

	ui.pushButton_6->setStyleSheet("QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	topbar = new QLabel(this);
	topbar->setGeometry(QRect(0,0,1024,50));
	topbar->setPicture(topbarpic);

	datetime = new QLabel(topbar);
	datetime->setGeometry(QRect(790,10,231,31));
	datetime->setStyleSheet(datetimestyle);

	Cautiondisplay = new QLabel(topbar);
	Cautiondisplay->setGeometry(QRect(400,0,400,50));
	Cautiondisplay->setStyleSheet(cautionstyle);

	ppdfshow = new pdfshow(this);
	ppdfshow->setGeometry(QRect(1, 60, 1022, 651));
	ppdfshow->setStyleSheet(widgetbkcolor);
	ppdfshow->setiBeginy(pubcom.iHelpViewLocation);
	ppdfshow->setiCurrentPage(pubcom.iHelpPageLocation);
	ppdfshow->setbCrossPage(pubcom.bHelpCrossPage);

	downflag = false;
	upflag = false;

	ui.pushButton_7->setStyleSheet("QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");
	ui.pushButton_8->setStyleSheet("QPushButton {padding: 3px ;border-width: 5px; border-top-color: rgb(232,232,232); border-left-color: rgb(232,232,232); border-right-color: black; border-bottom-color: black; border-style:outset ;background-color: rgb(178,178,178) ;font:20px 楷体_GB2312; color:rgb(11,6,231)} QPushButton:hover {background-color: rgb(208,208,208);  color:blue; border-width: 6px; } QPushButton:pressed {border-top-color: black; border-left-color:black ; border-right-color: black; border-bottom-color: black; background-color: rgb(178,178,178);} ");

	//启动定时器
	timer = &mytimer;
	count = 0;

	connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));
	ui.versionlabel->setText(QString(SOFTVERSTR));
}

help::~help()
{
}

pdfshow::pdfshow(QWidget *parent)
    : QWidget(parent)
{
	//get instance of current program (self)
    HINSTANCE hInst= GetModuleHandle (0);
	FPDF_InitLibrary(hInst);
	setAttribute(Qt::WA_PaintOnScreen,true);

	// 首先，加载文档（没有指定的密码）
	pdf_doc = FPDF_LoadDocument("./manual/usermanual.pdf", NULL);
	if (pdf_doc == NULL)// error handling
	{
		printf("加载pdf文档错误\n");
	}

	// 现在加载首页（页面索引为0）
	pdf_page = FPDF_LoadPage(pdf_doc, 0);
	iPageNum = FPDF_GetPageCount(pdf_doc);

	if (pdf_page == NULL)
	{
		printf("加载pdf页面错误\n");
	}

	page_width = FPDF_GetPageWidth(pdf_page);
	page_height = FPDF_GetPageHeight(pdf_page);
	FPDF_ClosePage(pdf_page);
}

void pdfshow::addBeginy()
{
	//防止出界
	if((iBeginy+ iViewHeight) <= (iPageNum*size_y+30))
	{
	    //如果原来是跨页的
		if(bCrossPage == true)
		{
			if((iCurrentPage +1)*size_y <= (iBeginy+30))
			{
				bCrossPage = false;
				iCurrentPage++;
			}
		}
		else
		{
			if((iCurrentPage +1)*size_y <= (iBeginy+iViewHeight+30))
			{
				bCrossPage = true;
			}
		}
		iBeginy +=30;
	}
}

void pdfshow::decBeginy()
{
	//防止出界
	if(iBeginy >= 30)
	{
	    //如果原来是跨页的
		if(bCrossPage == true)
		{
			if((iCurrentPage +1)*size_y >= (iBeginy+iViewHeight-30) )
			{
				bCrossPage = false;
			}
		}
		else
		{
			if(iCurrentPage*size_y >= (iBeginy-30))
			{
				iCurrentPage--;
				bCrossPage = true;
			}
		}

		iBeginy -=30;
	}
}

pdfshow::~pdfshow()
{
	FPDF_CloseDocument(pdf_doc);
	FPDF_DestroyLibrary();
}

void pdfshow::paintEvent(QPaintEvent *)
{
	iViewWidth = width();
	iViewHeight = height();

	QPainter painter(this);
	HDC hdc = painter.paintEngine()->getDC();   // THIS IS THE CRITICAL STEP! 
	//HDC hdc = getDC();   // THIS IS THE CRITICAL STEP! 

	// 将点数转换为像素
	size_x = iViewWidth;
	size_y = ((page_height*1000*iViewWidth)/page_width)/1000;

	HWND hwnd = winId();

	if(bCrossPage == false)
	{
		pdf_page = FPDF_LoadPage(pdf_doc, iCurrentPage);
		FPDF_RenderPage(hdc, pdf_page, 0, iCurrentPage*size_y-iBeginy,size_x ,size_y, 0, 0);
		FPDF_ClosePage(pdf_page);
	}
	else
	{
		pdf_page = FPDF_LoadPage(pdf_doc, iCurrentPage);
		FPDF_RenderPage(hdc, pdf_page, 0, iCurrentPage*size_y-iBeginy,size_x ,size_y, 0, 0);
		FPDF_ClosePage(pdf_page);
		pdf_page = FPDF_LoadPage(pdf_doc, iCurrentPage+1);
		FPDF_RenderPage(hdc, pdf_page, 0, (iCurrentPage+1)*size_y-iBeginy,size_x ,size_y, 0, 0);
		FPDF_ClosePage(pdf_page);

	}
	ReleaseDC(hwnd, hdc);
	//releaseDC(hdc);
}
int pdfshow::getiBeginy()
{
	return iBeginy;
}
int pdfshow::getiCurrentPage()
{
	return iCurrentPage;
}
bool pdfshow::getbCrossPage()
{
	return bCrossPage;
}

void pdfshow::setiBeginy(int y)
{
	iBeginy = y;
}
void pdfshow::setiCurrentPage(int i)
{
	iCurrentPage = i;
}
void pdfshow::setbCrossPage(bool b)
{
	bCrossPage = b;
}

void help::paintEvent(QPaintEvent *)
{
	/*
    //HDC hdc = GetDC(hwnd);

	// From this point on it is all regular GDI 
	QString text("Test GDI Paint");
	RECT rect;
	GetClientRect(hwnd, &rect);


	HBRUSH hbrRed = CreateSolidBrush(RGB(255,0,0));
	FillRect(hdc, &rect, hbrRed);
	HBRUSH hbrBlue = CreateSolidBrush(RGB(40,40,255));
	HPEN bpenGreen = CreatePen(PS_SOLID, 4, RGB(0,255,0));
	SelectObject(hdc,bpenGreen);
	SelectObject(hdc,hbrBlue);


	Ellipse(hdc,10,10,rect.right-20,rect.bottom-20);
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	TextOutW(hdc, width() / 2, height() / 2, text.utf16(), text.size());
	*/

}


void help::init(int modright)
{
	publiccaution.addevent("帮助页面","进入帮助页面","用户帮助页面",1);

	pubcom.CurrentWnd = 5;

	//定时器在最后才开始工作
	timer->start(100); //100ms定时
	return;
}

//设置
void help::on_pushButton_6_clicked()
{
	pubcom.iHelpViewLocation = ppdfshow->getiBeginy();
	pubcom.iHelpPageLocation = ppdfshow->getiCurrentPage();
	pubcom.bHelpCrossPage = ppdfshow->getbCrossPage();
	publiccaution.addevent("帮助页面","退出帮助页面","用户退出帮助页面",1);

	close();
}


//定时处理的任务
void help::timeupdate()
{
	
	//故障背景变化了
	if (publiccaution.count() >= 1
		&& pubcom.delcauflag == false)
	{
		if (cautionbkchgflag == 1)
		{
			if (cautionbkflag == 1)
			{
				topbar->setPicture(topbargraypic);
			}
			else
			{
				topbar->setPicture(topbarredpic);
			}
			cautionbkchgflag = 0;
		}
	}
	else
	{
		if (cautionbkchgflag == 1)
		{
			topbar->setPicture(topbarpic);
			cautionbkchgflag = 0;
		}
	}
	

	//故障字符变化了
	if (publiccaution.count() >= 1
		&& pubcom.delcauflag == false)
	{
		if (cautionstrchgflag == 1)
		{
			t_caution caution;
			publiccaution.getcautionbyindex(curcautionindex,caution);
			Cautiondisplay->setText(pubcom.cautionlocationidstrmap.value(caution.location) + pubcom.cautionidstrmap.value(caution.code));
			cautionstrchgflag = 0;
		}
	}
	else
	{
		if (cautionstrchgflag == 1)
		{
			Cautiondisplay->setText("");
			cautionstrchgflag = 0;
		}
	}

	//日期变化了
	if (datetimechgflag == 1)
	{
		datetimechgflag = 0;
		datetime->setText(QDateTime::currentDateTime().toString(
			"yyyy-MM-dd hh:mm:ss"));
	}

	if (downflag == true
		|| upflag == true)
	{
		count ++;
	}

	//向下按钮
	if (downflag == true)
	{
		//3秒钟切换成快速卷动
		if (count > 20)
		{
			ppdfshow->addBeginy();
			ppdfshow->addBeginy();
			ppdfshow->addBeginy();
			ppdfshow->addBeginy();
			ppdfshow->addBeginy();
			ppdfshow->update();
		}
		else
		{
			ppdfshow->addBeginy();
			ppdfshow->update();
		}
	}

	//向上按钮
	if (upflag == true)
	{
		//3秒钟切换成快速卷动
		if (count > 20)
		{
			ppdfshow->decBeginy();
			ppdfshow->decBeginy();
			ppdfshow->decBeginy();
			ppdfshow->decBeginy();
			ppdfshow->decBeginy();
			ppdfshow->update();

		}
		else
		{
			ppdfshow->decBeginy();
			ppdfshow->update();
		}

	}
	ppdfshow->update();

}


void help::mousePressEvent(QMouseEvent * event)
{
	if (event->x() < 1024 && event->x() > 0 && event->y() < 50 && event->y()
			> 0)
	{
		caution Dlg;
		Dlg.setStyleSheet(dlgbkcolor);
#ifdef ONIPC
		Dlg.setWindowState(Qt::WindowFullScreen);
#endif
		Dlg.setWindowFlags(Qt::FramelessWindowHint);
		Dlg.init(127);
		//这个必须设置否则不能转跳到新窗口
		setModal(false);
		Dlg.exec();

		pubcom.CurrentWnd = 5;

	}
	QDialog::mousePressEvent(event);
}


void help::on_pushButton_7_pressed()
{
	downflag = true;
}

void help::on_pushButton_7_released()
{
	downflag = false;
	count = 0;
}


void help::on_pushButton_8_pressed()
{
	upflag = true;
}

void help::on_pushButton_8_released()
{
	upflag = false;
	count = 0;
}

