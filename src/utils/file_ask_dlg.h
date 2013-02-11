#ifndef FILE_ASK_DLG_H
#define FILE_ASK_DLG_H

#include <QWidget>
#include <QDialog>

#include "ui_fileaskdlg.h"

namespace Ui
{
    class FileAskDlg;
};

class FileAskDlg : public QDialog
{
    Q_OBJECT
public:
    FileAskDlg(QString sender, QString file, QWidget *parent = NULL);

    void notifyRefuseMessage();

private:
    Ui::FileAskDlg ui;
};

#endif //FILE_ASK_DLG_H
