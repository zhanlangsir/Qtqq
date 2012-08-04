#ifndef QTQQ_CORE_GROUPMSGENCODER_H
#define QTQQ_CORE_GROUPMSGENCODER_H

#include "msgencoder.h"

class GroupMsgEncoder : public MsgEncoder
{
public:
    GroupMsgEncoder(QQChatDlg *dlg);
    
    virtual QString encode(const QString &raw_msg);
};

#endif // QTQQ_CORE_GROUPMSGENCODER_H
