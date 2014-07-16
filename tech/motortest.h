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

#ifndef MOTORTEST_H
#define MOTORTEST_H

#include <QtGui/QDialog>
#include "ui_motortest.h"

class motortest : public QDialog
{
    Q_OBJECT

public:
    motortest(QWidget *parent = 0);
    ~motortest();

private:
    Ui::motortestClass ui;

    bool printchoose[3];
    bool motorfollow;
    //0 就绪 1转动 2跟随
    unsigned char motorstatus;

    //正转
    bool roll1;

    //反转
    bool roll2;



public slots:

//网头1选择
void on_pushButton_clicked();
//网头2选择
void on_pushButton_2_clicked();
//网头3选择
void on_pushButton_3_clicked();

//正转
void on_pushButton_4_clicked();
//反转
void on_pushButton_5_clicked();
//停止转动
void on_pushButton_6_clicked();

//JOG+
void on_pushButton_7_clicked();
//JOG-
void on_pushButton_8_clicked();

//跟随
void on_pushButton_9_clicked();

//返回
void on_pushButton_10_clicked();

//停止跟随
void on_pushButton_11_clicked();


private slots:
	void on_pushButton_14_clicked();
	void on_pushButton_13_clicked();
	void on_pushButton_12_clicked();
};

#endif // MOTORTEST_H
