#ifndef SENDFILE_WIDGET_H
#define SENDFILE_WIDGET_H 

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QMap>
#include <QTimer>

#include "ui_sendfilewidget.h"

struct SendFileItem
{
    int session_id;

    QString to_id;
    QString to_name;
    QString file_path;
    int file_size;
    int dl_byte;
    long begin_time;

    bool done;  //if send done
    int row;
};

class SendFileWidget : public QDialog
{
    Q_OBJECT
public:
    SendFileWidget(QWidget *parent = NULL);

    void append(SendFileItem item);
    void pause(int session_id);

    void setUploadDone(const QString &file, int session_id);
    void setSendDone(int session_id);
    void setRefuseRecvFile(int session_id);
    void onSendAck(int session_id);

private slots:
    void onSendFileProgress(QString file, int done_byte, int total_byte);
    void on_pause_btn_clicked();
    void onTick();

private:
    void createUiItem(SendFileItem &item);

private:
    Ui::SendFileWidget ui;
    QTimer tick_;

    QMap<QString, SendFileItem> transfer_items_;
    QMap<int, SendFileItem> session_ids_;
};

#endif //SENDFILE_WIDGET_H
