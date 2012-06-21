#ifndef QTQQ_CHATLOGWIN_H
#define QTQQ_CHATLOGWIN_H

#include <QWidget>
#include "core/qqmsg.h"

namespace Ui
{
class ChatLogWin;
}

class QQChatLog;
class NameConvertor;

class ChatLogWin : public QWidget
{
    Q_OBJECT
public:
    explicit ChatLogWin(QWidget *parent = 0);
    ~ChatLogWin();

public:
    void setChatLog(QQChatLog *chat_log)
    { chat_log_ = chat_log; }
    void setNameConvertor(const NameConvertor *convertor)
    { convertor_ = convertor; }

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
    const NameConvertor *convertor_;
    QQChatLog *chat_log_;
};

#endif // QTQQ_CHATLOGWIN_H
