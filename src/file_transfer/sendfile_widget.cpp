#include "sendfile_widget.h"

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

#include "file_transfer/transfer_util.h"
#include "protocol/qq_protocol.h"

#define RECV_NAME_ROW 0
#define FILE_NAME_ROW 1
#define FILE_SIZE_ROW 2
#define PROGRESS_BAR_ROW 3
#define UPLOAD_SPEED_ROW 4
#define SEND_STATUS_ROW 5

SendFileWidget::SendFileWidget(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    setWindowFlags(windowFlags() & Qt::Window);

    ui.tw_tasks->setSelectionBehavior(QTableWidget::SelectRows);
    ui.tw_tasks->setSelectionMode(QAbstractItemView::SingleSelection);

    tick_.setInterval(1000);
    connect(&tick_, SIGNAL(timeout()), this, SLOT(onTick()));
}

void SendFileWidget::append(SendFileItem item)
{
    if ( !transfer_items_.contains(item.file_path) )
    {
        transfer_items_.insert(item.file_path, item);

        createUiItem(transfer_items_[item.file_path]);
        if ( !tick_.isActive() )
            tick_.start();
    }
}

void SendFileWidget::pause(int session_id)
{
    SendFileItem &item = session_ids_[session_id];
    QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, SEND_STATUS_ROW);
    download_status_label->setText(tr("Peer pause recive file!"));

    session_ids_.remove(session_id);
}

void SendFileWidget::createUiItem(SendFileItem &item)
{
    int n_row = ui.tw_tasks->rowCount();

    ui.tw_tasks->insertRow(n_row);
    ui.tw_tasks->setCellWidget(n_row, RECV_NAME_ROW, new QLabel(item.to_name));
    ui.tw_tasks->setCellWidget(n_row, FILE_NAME_ROW, new QLabel(QFileInfo(item.file_path).fileName()));
    ui.tw_tasks->setCellWidget(n_row, FILE_SIZE_ROW, new QLabel(QString::number(item.file_size)));

    QProgressBar *progress_bar =  new QProgressBar(ui.tw_tasks);
    ui.tw_tasks->setCellWidget(n_row, PROGRESS_BAR_ROW, progress_bar);
    ui.tw_tasks->setCellWidget(n_row, UPLOAD_SPEED_ROW, new QLabel("0 b/s"));
    ui.tw_tasks->setCellWidget(n_row, SEND_STATUS_ROW, new QLabel(tr("Uploading...")));

    item.row = n_row;
}

void SendFileWidget::onSendFileProgress(QString file, int done_byte, int total_byte)
{
    SendFileItem &item = transfer_items_[file];

    if ( item.file_size != total_byte )
    {
        item.file_size = total_byte;

        QString unit;
        int size = TransferUtil::unitTranslation(total_byte, unit);
        QLabel *file_size_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, FILE_SIZE_ROW);
        file_size_label->setText(QString::number(size) + ' ' + unit);

        QProgressBar *progress_bar = (QProgressBar *)ui.tw_tasks->cellWidget(item.row, PROGRESS_BAR_ROW);
        progress_bar->setMaximum(total_byte);
    }

    QProgressBar *progress_bar = (QProgressBar *)ui.tw_tasks->cellWidget(item.row, PROGRESS_BAR_ROW);
    item.dl_byte = done_byte;
    progress_bar->setValue(done_byte);

    if ( done_byte == total_byte )
    {
        transfer_items_[file].done = true;
        QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, SEND_STATUS_ROW);
        download_status_label->setText(tr("Upload done!"));
    }
}

void SendFileWidget::on_pause_btn_clicked()
{
    if ( ui.tw_tasks->currentRow() == -1 )
        return;

    foreach ( const SendFileItem &item, transfer_items_.values() )
    {
        if ( item.row == ui.tw_tasks->currentRow() && !item.done )
        {
            QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, SEND_STATUS_ROW);
            download_status_label->setText(tr("Upload pause!"));

            transfer_items_.remove(item.file_path);

            Protocol::QQProtocol::instance()->pauseSendFile(item.file_path);
        }
    }
}

void SendFileWidget::onTick()
{
    if ( transfer_items_.count() == 0 )
    {
        tick_.stop();
        return;
    }

    foreach ( const SendFileItem &item, transfer_items_.values() )
    {
        int used_time = QDateTime::fromMSecsSinceEpoch(item.begin_time).secsTo(QDateTime::currentDateTime());
        int speed = (used_time == 0 ? 0 : item.dl_byte / used_time);

        QLabel *speed_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, UPLOAD_SPEED_ROW);

        QString unit;
        speed = TransferUtil::unitTranslation(speed, unit);
        speed_label->setText(QString::number(speed) + ' ' + unit + "/s");
    }
}

void SendFileWidget::setUploadDone(const QString &file, int session_id)
{
    QMap<QString, SendFileItem>::iterator itor = transfer_items_.begin();
    while ( itor != transfer_items_.end() )
    {
        if ( itor.key().contains(file) && itor->done )
        {
            itor->session_id = session_id; 
            session_ids_.insert(session_id, itor.value());

            break;
        }
        ++itor;
    }

    if ( itor != transfer_items_.end() )
    {
        transfer_items_.erase(itor);
    }
}

void SendFileWidget::onSendAck(int session_id)
{
    SendFileItem &item = session_ids_[session_id];
    QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, SEND_STATUS_ROW);
    download_status_label->setText(tr("Peer begin recive file!"));
}

void SendFileWidget::setSendDone(int session_id)
{
    SendFileItem &item = session_ids_[session_id];
    QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, SEND_STATUS_ROW);
    download_status_label->setText(tr("File send Done!"));
}

void SendFileWidget::setRefuseRecvFile(int session_id)
{
    SendFileItem &item = session_ids_[session_id];
    QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, SEND_STATUS_ROW);
    download_status_label->setText(tr("Peer refuse to recv file!"));
}
