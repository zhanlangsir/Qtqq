#ifndef CHATMSG_PROCESSOR_H
#define CHATMSG_PROCESSOR_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVector>

#include "core/qqmsg.h"

class QAction;

class QQMsgListener;
class QQChatDlg;

class ChatMsgProcessor : public QObject
{
	Q_OBJECT
public:
	~ChatMsgProcessor();
	static ChatMsgProcessor *instance()
	{
		if ( !instance_ )
			instance_ = new ChatMsgProcessor();

		return instance_;
	}

	void registerListener(QQMsgListener *listener);
	void removeListener(QQMsgListener *listener);

	void stop();

private slots:
	void onNewChatMsg(ShareQQMsgPtr msg);
    void onNewOffFileMsg(ShareQQMsgPtr msg);
	void onActivatedChatDlgChanged(QQChatDlg *before, QQChatDlg *after);
	void onActionTriggered();
    void onNewStrangerInfo(QString id, Contact *stranger);
    void onNewStrangerIcon(QString id, QPixmap pix);

private:
	void createTrayNotify(ShareQQMsgPtr msg);
    void getOldMsgs(const QString &id, QVector<ShareQQMsgPtr> &msgs);
	QQMsgListener *listenerById(const QString &id) const;
	QAction *actionById(const QString &id) const;
	void removeAction(QAction *act);

private:
    QList<ShareQQMsgPtr> old_msgs_;
    QList<QQMsgListener*> listener_;

	QList<QAction *> actions_;

private:
	ChatMsgProcessor();
	ChatMsgProcessor(const ChatMsgProcessor&);
	ChatMsgProcessor &operator=(const ChatMsgProcessor &);

	static ChatMsgProcessor *instance_;
};

#endif //CHATMSG_PROCESSOR_H
