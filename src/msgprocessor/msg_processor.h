#ifndef MSG_PROCESSOR_H
#define MSG_PROCESSOR_H

#include <QList>
#include <QThread>
#include <QString>
#include <QQueue>
#include <QMutex>

#include "json/json.h"
#include "core/qqmsg.h"

class MsgProcessor : public QThread
{
    Q_OBJECT
signals:
    void contactStatusChanged(QString uin, ContactStatus state, ContactClientType client_type);

    void newGroupChatMsg(ShareQQMsgPtr msg);
    void newFriendChatMsg(ShareQQMsgPtr msg);
    void newSessChatMsg(ShareQQMsgPtr msg);
    void newChatMsg(ShareQQMsgPtr msg);
    void newSystemMsg(ShareQQMsgPtr msg);
    void newSystemGMsg(ShareQQMsgPtr msg);
    void newFileMsg(ShareQQMsgPtr msg);
    void newOffFileMsg(ShareQQMsgPtr msg);
    void newFilesrvTransferMsg(ShareQQMsgPtr msg);

public:
    ~MsgProcessor();
    static MsgProcessor *instance()
    {
      if ( !instance_ )
	instance_ = new MsgProcessor();

      return instance_;
    }

    void run();
    void stop();

public slots:
    void pushRawMsg(QByteArray msg);

private:
    void dispatchMsg(QVector<QQMsg *> &msgs);

    QQMsg *createBuddiesStatusChangeMsg(const Json::Value &result) const;
    QQMsg *createFileMsg(const Json::Value &result) const;
    QQMsg *createFriendMsg(const Json::Value &result) const;
    QQMsg *createGroupMsg(const Json::Value &result) const;
    QQMsg *createMsg(QString type, const Json::Value result);
    QQMsg *createOffFileMsg(const Json::Value &result) const;
    QQMsg *createSessMsg(const Json::Value &result) const;
    QQMsg *createSystemGroupMsg(const Json::Value &result) const;
    QQMsg *createSystemMsg(const Json::Value &result) const;
    QQMsg *createFilesrvTransferMsg(const Json::Value &result) const;
  
	bool isChatContentEmpty(const QQChatMsg *msg, const QString &content) const;
	bool isMsgRepeat(const Json::Value &msg);

    void sortByTime(QVector<QQMsg*> &be_sorting_msg) const;

private:
   QQueue<QByteArray> message_queue_;
   QMutex lock_;

   Json::Value pre_msg_;

private:
    MsgProcessor();
    MsgProcessor(const MsgProcessor &);
    MsgProcessor &operator=(const MsgProcessor &);

	static MsgProcessor *instance_;
};

#endif //MSG_PROCESSOR_H
