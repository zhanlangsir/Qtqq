#ifndef QTQQ_CORE_MSGENCODER_H
#define QTQQ_CORE_MSGENCODER_H

#include <QString>

class QQChatDlg;

class MsgEncoder
{
public:
    MsgEncoder(QQChatDlg *dlg);
    virtual ~MsgEncoder() {}

    virtual QString encode(const QString &raw_msg) = 0;

    void jsonEncoding(QString &escasing) const
    {
        escasing.replace("&lt;", "%3C").replace("&gt;", "%3E").replace("&amp;", "%26").replace('+', "%2B").replace(';', "%3B");
    }


protected:
    QQChatDlg *dlg_;
    int msg_id_;
};

#endif // QTQQ_CORE_MSGENCODER_H
