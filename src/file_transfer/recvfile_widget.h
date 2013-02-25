#ifndef RECVFIEL_WIDGET_H
#define RECVFIEL_WIDGET_H

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QMap>
#include <QTimer>

#include "ui_recvfilewidget.h"

struct RecvFileItem
{
    int id;
    QString sender_name;
    QString file_name;
    int file_size;
    int dl_byte;
    long begin_time;

    int row;
};

class RecvFileWidget : public QDialog
{
    Q_OBJECT
public:
    RecvFileWidget(QWidget *parent = NULL);

    void append(RecvFileItem item);
    void pause(int session_id);

private slots:
    void onFileTransferProgress(int session_id, int done_byte, int total_byte);
    void on_pause_btn_clicked();
    void on_open_directory_btn_clicked();
    void onTick();

private:
    void createUiItem(RecvFileItem &item);

private:
    Ui::RecvFileWidget ui;
    QTimer tick_;

    QMap<int, RecvFileItem> transfer_items_;
};

#endif //RECVFILE_WIDGET_H
