#include "file_ask_dlg.h"

#include <QApplication>
#include <QDesktopWidget>

FileAskDlg::FileAskDlg(QString sender, QString file, QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    ui.message_label->setVisible(false);
    ui.close_btn->setVisible(false);

    ui.sender_name_label->setText(sender);
    ui.file_name_label->setText(file);

    connect(ui.accept_btn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui.refuse_btn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui.close_btn, SIGNAL(clicked()), this, SLOT(reject()));
}

void FileAskDlg::notifyRefuseMessage()
{
    ui.message_label->setText("Peer cancel to send file");

    ui.message_label->setVisible(true);
    ui.close_btn->setVisible(true);
    ui.accept_btn->setVisible(false);
    ui.refuse_btn->setVisible(false);
}
