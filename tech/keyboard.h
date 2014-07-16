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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QtGui/QDialog>
#include "ui_keyboard.h"
#include "QString"
#include <QPainter>

class keyboard : public QDialog
{
    Q_OBJECT

public:
    keyboard(QWidget *parent = 0);
    ~keyboard();
    unsigned char flag;//1 cancel 2 OK
    QString password;

	//±³¾°Í¼Æ¬
    QLabel* padtopbar1;
    QLabel* padtopbar2;
	QLabel* usertitle;
	QLabel* userpsw1;
	QLabel* userpsw2;

public slots:
     void on_pb1_clicked();
     void on_pb2_clicked();
     void on_pb3_clicked();
     void on_pb4_clicked();
     void on_pb5_clicked();
     void on_pb6_clicked();
     void on_pb7_clicked();
     void on_pb8_clicked();
     void on_pb9_clicked();
     void on_pb0_clicked();
     void on_pbback_clicked();
     void on_pbcancel_clicked();
     void on_pbok_clicked();
   //void on_pbclear_clicked();

private:
    Ui::keyboardClass ui;
};

#endif // KEYBOARD_H
