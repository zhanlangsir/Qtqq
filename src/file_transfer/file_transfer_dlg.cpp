#include "file_transfer_dlg.h"

#include <QApplication>
#include <QUrl>
#include <QDir>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QLabel>
#include <QProgressBar>
#include <QDateTime>
#include <QTableWidgetSelectionRange>
#include <QDebug>

#define SENDER_NAME_ROW 0
#define FILE_NAME_ROW 1
#define FILE_SIZE_ROW 2
#define PROGRESS_BAR_ROW 3
#define DOWNLOADED_SPEED_ROW 4
#define DOWNLOAD_STATUS_ROW 5

#define KB 1024
#define M 1048576

#define B2KB(x) (x >> 10)
#define B2M(x) (x >> 20)

FileTransferDlg::FileTransferDlg(QWidget *parent) :
    QDialog(parent),
    send_widget_(this),
    recv_widget_(this)
{
    ui.setupUi(this);

    ui.main_tab->addTab(&send_widget_, tr("Sending"));
    ui.main_tab->addTab(&recv_widget_, tr("Recving"));

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    setWindowFlags(windowFlags() & Qt::Window);
}

void FileTransferDlg::appendRecvItem(RecvFileItem item)
{
    recv_widget_.append(item);
}

void FileTransferDlg::pauseRecving(int session_id)
{
    recv_widget_.pause(session_id);     
}

void FileTransferDlg::appendSendItem(const QString &to_id, const QString &to_name, const QString &file_path)
{
    SendFileItem item;
    item.to_id = to_id;
    item.to_name = to_name;
    item.file_path = file_path;
    item.begin_time = QDateTime::currentMSecsSinceEpoch();

    send_widget_.append(item);
}

void FileTransferDlg::pauseSending(int session_id)
{
    send_widget_.pause(session_id);     
}

void FileTransferDlg::on_close_btn_clicked()
{
    hide();
}

void FileTransferDlg::showRecvWidget()
{
    ui.main_tab->setCurrentIndex(ui.main_tab->indexOf(&recv_widget_));
    show();
    raise();
}

void FileTransferDlg::showSendWidget()
{
    ui.main_tab->setCurrentIndex(ui.main_tab->indexOf(&send_widget_));
    show();
    raise();
}

void FileTransferDlg::setUploadDone(const QString &file_name, int session_id)
{
    send_widget_.setUploadDone(file_name, session_id);
}
