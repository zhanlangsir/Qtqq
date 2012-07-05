#ifndef QTQQ_CORE_FRIENDMSGENCODER_H
#define QTQQ_CORE_FRIENDMSGENCODER_H

#include "msgencoder.h"

class FriendMsgEncoder : public MsgEncoder
{
public:
    FriendMsgEncoder(QQChatDlg *dlg);

    virtual QString encode(const QString &raw_msg);
};

#endif // QTQQ_CORE_FRIENDMSGENCODER_H
