#ifndef REQUESTMSG_PROCESSOR_H
#define REQUESTMSG_PROCESSOR_H

#include <QObject>
#include <QList>
#include <QPixmap>

#include "core/qqmsg.h"

class QAction;

class RequestMsgProcessor : public QObject
{
	Q_OBJECT
public:
	~RequestMsgProcessor();
	static RequestMsgProcessor *instance()
	{
		if ( !instance_ )
			instance_ = new RequestMsgProcessor();
		
		return instance_;
	}

	void stop();

private slots:
	void onNewSystemMsg(ShareQQMsgPtr msg);
    void onNewSystemGMsg(ShareQQMsgPtr msg);

	void onActionTriggered();

	void onNewStrangerInfo(QString id, Contact *stranger);
	void onNewStrangerIcon(QString id, QPixmap pix);

private:
	void createTrayNotify(ShareQQMsgPtr msg, Contact *stranger);
	QAction *friendActionById(const QString &id) const;
	QAction *groupActionById(const QString &id) const;

    void showMessageBox(const QPixmap &pix, const QString &msg);
    Contact *getContactFromSystemMsg(const QString &id);

private:
	QList<QAction *> actions_;
    QMap<QString, QString> requesting_groups_;

private:
	RequestMsgProcessor();
	RequestMsgProcessor(const RequestMsgProcessor &);
	RequestMsgProcessor &operator=(const RequestMsgProcessor &);
	
	static RequestMsgProcessor *instance_;
};

#endif //REQUESTMSG_PROCESSOR_H
