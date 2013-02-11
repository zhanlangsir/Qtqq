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

void FileTransferDlg::onCurrentCellChanged(int current_row, int current_column, int previous_row, int previous_column)
{
    /*
    QTableWidgetSelectionRange range(current_row, 0, current_row, 5); 
    ui.tw_tasks->setRangeSelected(range, true);
    */
    QTableWidgetSelectionRange range(current_row, current_column, current_row, current_column); 
    ui.tw_tasks->setRangeSelected(range, false);
}

void FileTransferDlg::appendItem(TransferItem item)
{
    if ( !transfer_items_.contains(item.id) )
    {
        transfer_items_.insert(item.id, item);

        createUiItem(transfer_items_[item.id]);
        if ( !tick_.isActive() )
            tick_.start();
    }
}

void FileTransferDlg::createUiItem(TransferItem &item)
{
    int n_row = ui.tw_tasks->rowCount();
    ui.tw_tasks->insertRow(n_row);
    ui.tw_tasks->setCellWidget(n_row, SENDER_NAME_ROW, new QLabel(item.sender_name));
    ui.tw_tasks->setCellWidget(n_row, FILE_NAME_ROW, new QLabel(item.file_name));
    ui.tw_tasks->setCellWidget(n_row, FILE_SIZE_ROW, new QLabel(QString::number(item.file_size)));
    QProgressBar *progress_bar =  new QProgressBar(ui.tw_tasks);
    ui.tw_tasks->setCellWidget(n_row, PROGRESS_BAR_ROW, progress_bar);
    ui.tw_tasks->setCellWidget(n_row, DOWNLOADED_SPEED_ROW, new QLabel("0 b/s"));
    ui.tw_tasks->setCellWidget(n_row, DOWNLOAD_STATUS_ROW, new QLabel(tr("Downloading...")));

    item.row = n_row;
}

void FileTransferDlg::cancelItem(int session_id)
{
    TransferItem &item = transfer_items_[session_id];
    QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, DOWNLOAD_STATUS_ROW);
    download_status_label->setText(tr("Download failed! Peer cancel to send file!"));

    transfer_items_.remove(session_id);
}

void FileTransferDlg::onFileTransferProgress(int session_id, int done_byte, int total_byte)
{
    TransferItem &item = transfer_items_[session_id];

    if ( item.file_size != total_byte )
    {
        item.file_size = total_byte;

        QString unit;
        int size = unitTranslation(total_byte, unit);
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
        transfer_items_.remove(session_id);
        QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, DOWNLOAD_STATUS_ROW);
        download_status_label->setText(tr("Download finish!"));
    }
}

void FileTransferDlg::on_close_btn_clicked()
{
    hide();
}

void FileTransferDlg::on_open_directory_btn_clicked()
{
    QDesktopServices::openUrl(QUrl(QDir::homePath() + "/QtqqDownload", QUrl::TolerantMode));
}

void FileTransferDlg::on_pause_btn_clicked()
{
    if ( ui.tw_tasks->currentRow() == -1 )
        return;

    foreach ( const TransferItem &item, transfer_items_.values() )
    {
        if ( item.row == ui.tw_tasks->currentRow() )
        {
            QLabel *download_status_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, DOWNLOAD_STATUS_ROW);
            download_status_label->setText(tr("Download been Cancel!"));

            transfer_items_.remove(item.id);

            emit parseItem(item.id);
        }
    }
}

void FileTransferDlg::onTick()
{
    if ( transfer_items_.count() == 0 )
    {
        tick_.stop();
        return;
    }

    foreach ( const TransferItem &item, transfer_items_.values() )
    {
        int used_time = QDateTime::fromMSecsSinceEpoch(item.begin_time).secsTo(QDateTime::currentDateTime());
        int speed = (used_time == 0 ? 0 : item.dl_byte / used_time);
        QLabel *speed_label = (QLabel *)ui.tw_tasks->cellWidget(item.row, DOWNLOADED_SPEED_ROW);

        QString unit;
        speed = unitTranslation(speed, unit);
        speed_label->setText(QString::number(speed) + ' ' + unit + "/s");
    }
}

int FileTransferDlg::unitTranslation(int byte, QString &unit)
{
    if ( byte < KB )
    {
        unit = "b";
        return byte;
    }
    else if ( KB < byte && byte < M )
    {
        unit = "kb";
        return B2KB(byte);
    }
    else
    {
        unit = "m"; 
        return B2M(byte);
    }
}
