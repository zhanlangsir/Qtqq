#ifndef QTQQ_QTQQ_H
#define QTQQ_QTQQ_H

#include <QObject>

#include "QQLoginCore.h"

class QQLoginDlg;
class QQMainPanel;

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
    QQLoginDlg *login_dlg_;
    QQLoginCore *login_core_;
    QQMainPanel *mainpanel_; 
};

#endif // QTQQ_QTQQ_H
