#ifndef FILE_TRANSFER_DLG_H
#define FILE_TRANSFER_DLG_H

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QMap>
#include <QTimer>

#include "ui_filetransferdlg.h"

struct TransferItem
{
    int id;
    QString sender_name;
    QString file_name;
    int file_size;
    int dl_byte;
    long begin_time;

    int row;
};

class FileTransferDlg : public QDialog
{
    Q_OBJECT
signals:
    void parseItem(int session_id);

public:
    FileTransferDlg(QWidget *parent = NULL);

    void appendItem(TransferItem item);
    void cancelItem(int session_id);

private slots:
    void onFileTransferProgress(int session_id, int done_byte, int total_byte);
    void onCurrentCellChanged(int current_row, int current_column, int previous_row, int previous_column);
    void on_pause_btn_clicked();
    void on_open_directory_btn_clicked();
    void on_close_btn_clicked();
    void onTick();

private:
    void createUiItem(TransferItem &item);
    int unitTranslation(int byte, QString &unit);

private:
    Ui::FileTransferDlg ui;
    QTimer tick_;

    QMap<int, TransferItem> transfer_items_;
};

#endif //FILE_TRANSFER_DLG_H
