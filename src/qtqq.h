#ifndef QTQQ_QTQQ_H
#define QTQQ_QTQQ_H

#include <QObject>

#include "core/qqlogincore.h"

class QQLoginWin;
class QQMainWindow;

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
    QQLoginWin *login_dlg_;
    QQLoginCore *login_core_;
    QQMainWindow *mainpanel_;
};

#endif // QTQQ_QTQQ_H
