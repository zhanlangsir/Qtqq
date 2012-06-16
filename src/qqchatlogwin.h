#ifndef QTQQ_QQCHATLOGWIN_H
#define QTQQ_QQCHATLOGWIN_H

#include <QWidget>
#include "core/qqmsg.h"

namespace Ui
{
class QQChatLogWin;
}

class QQChatLog;
class NameConvertor;

class QQChatLogWin : public QWidget
{
    Q_OBJECT
public:
    explicit QQChatLogWin(QWidget *parent = 0);
    ~QQChatLogWin();

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
    Ui::QQChatLogWin *ui_;
    const NameConvertor *convertor_;
    QQChatLog *chat_log_;
};

#endif // QTQQ_QQCHATLOGWIN_H
