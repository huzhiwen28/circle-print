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

#ifndef PASSWORD_H
#define PASSWORD_H

#include <QtGui/QDialog>
#include <QLabel>
#include <QLineEdit>
#include "ui_password.h"

class password : public QDialog
{
    Q_OBJECT

public:
    password(QWidget *parent = 0);
    ~password();
    void initpass(char usertype);

    //标签
    QLabel* design;
    QLabel* factory;
    QLabel* develop;

    QLabel* title1;
    QLabel* title2;
    QLabel* title3;

    //密码输入
    QLineEdit* eddesign;
    QLineEdit* edfactory;

    //模块选择按钮
    QPushButton* pbdesign1;
    QPushButton* pbdesign2;
    QPushButton* pbdesign3;
    QPushButton* pbdesign4;

    QPushButton* pbfactory1;
    QPushButton* pbfactory2;
    QPushButton* pbfactory3;
    QPushButton* pbfactory4;

    QPushButton* pbdevelop1;
    QPushButton* pbdevelop2;
    QPushButton* pbdevelop3;
    QPushButton* pbdevelop4;

    //模块选择标识
    int designright;
    int factoryright;
    int developright;

    //密码修改按钮
    QPushButton* pbchgpass1;
    QPushButton* pbchgpass2;

public slots:

//按钮
	void on_pushButton_clicked();
	void pbdesign1_clicked();
	void pbdesign2_clicked();
	void pbdesign3_clicked();
	void pbdesign4_clicked();
	void pbfactory1_clicked();
	void pbfactory2_clicked();
	void pbfactory3_clicked();
	void pbfactory4_clicked();
	void pbdevelop1_clicked();
	void pbdevelop2_clicked();
	void pbdevelop3_clicked();
	void pbdevelop4_clicked();
	void pbchgpass1_clicked();
	void pbchgpass2_clicked();

private:
    Ui::passwordClass ui;
};

#endif // PASSWORD_H
