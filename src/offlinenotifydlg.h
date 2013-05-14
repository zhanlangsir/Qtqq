#ifndef OFFLINENOTIFY_DLG_H
#define OFFLINENOTIFY_DLG_H

#include <QWidget>
#include <QDialog>

#include "ui_offlinenotifydlg.h"

class OfflineNotifyDlg : public QDialog, public Ui::OfflineNotifyDlg
{
    Q_OBJECT
public:
    OfflineNotifyDlg(QWidget *parent = 0) :
        QDialog(parent)
    {
       setupUi(this); 
    }

private slots:
    void on_relogin_btn_clicked()
    {
        accept();
    }
    void on_ok_btn_clicked()
    {
        reject();
    }
};

#endif //OFFLINENOTIFY_DLG_H
