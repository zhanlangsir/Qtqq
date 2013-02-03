#ifndef CHATLOGWIN_H
#define CHATLOGWIN_H

#include <QWidget>
#include <QMap>

#include "core/qqmsg.h"

namespace Ui
{
    class ChatLogWin;
}

class QQChatLog;

class ChatLogWin : public QWidget
{
    Q_OBJECT
public:
    explicit ChatLogWin(QMap<QString, QString> names, QWidget *parent = 0);
    ~ChatLogWin();

public:
    void setChatLog(QQChatLog *chat_log)
    { chat_log_ = chat_log; }

protected:
    void closeEvent(QCloseEvent *);

public slots:
    void getNextPage();
    void getPrePage();
    void getLastPage();
    void getFirstPage();

private:
    void insertNameLine(const QString &name, QColor color);
    void insertQQFace(const QString &face_id);
    void insertWord(const QString &text, QFont font, QColor color, int size);
    void insertImg(const QString &img_name);
    void showChatLog(QVector<ShareQQMsgPtr> &chat_logs);
    
private:
    Ui::ChatLogWin *ui_;
    QQChatLog *chat_log_;

    QMap<QString, QString> names_;
};

#endif //CHATLOGWIN_H
