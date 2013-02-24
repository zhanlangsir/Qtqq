#ifndef FILE_TRANSFER_DLG_H
#define FILE_TRANSFER_DLG_H

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QMap>
#include <QTimer>

#include "ui_filetransferdlg.h"

#include "file_transfer/sendfile_widget.h"
#include "file_transfer/recvfile_widget.h"

class FileTransferDlg : public QDialog
{
    Q_OBJECT
signals:
    void parseItem(int session_id);

public:
    FileTransferDlg(QWidget *parent = NULL);

    SendFileWidget *sendWidget()
    { return &send_widget_; }
    RecvFileWidget *recvWidget() 
    { return &recv_widget_; }

    void setUploadDone(const QString &file_name, int session_id);

    void showRecvWidget();
    void showSendWidget();

    void appendRecvItem(RecvFileItem item);
    void pauseRecving(int session_id);

    void appendSendItem(const QString &to_id, const QString &to_name, const QString &file_path);
    void pauseSending(int session_id);

private slots:
    void on_close_btn_clicked();

private:
    Ui::FileTransferDlg ui;

    SendFileWidget send_widget_;
    RecvFileWidget recv_widget_;
};

#endif //FILE_TRANSFER_DLG_H
