#ifndef QTQQ_QTQQ_H
#define QTQQ_QTQQ_H

#include <QObject>

#include "core/qqlogincore.h"

class LoginWin;
class MainWindow;

class Qtqq : public QObject
{
    Q_OBJECT
public:
    Qtqq();
    ~Qtqq();

public:
    void start();

private slots:
    void slot_showMainPanel();
    void restart();

private:
    LoginWin *login_dlg_;
    MainWindow *mainpanel_;
};

#endif // QTQQ_QTQQ_H
