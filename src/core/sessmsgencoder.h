#ifndef QTQQ_CORE_SESSMSGENCODER_H
#define QTQQ_CORE_SESSMSGENCODER_H

#include "msgencoder.h"

class SessMsgEncoder : public MsgEncoder
{
public:
    SessMsgEncoder(QQChatDlg *dlg, QString code, QString sig);

    virtual QString encode(const QString &raw_msg);

private:
    QString code_;
    QString sig_;
};

#endif // QTQQ_CORE_SESSMSGENCODER_H
