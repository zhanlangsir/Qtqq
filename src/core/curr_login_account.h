#ifndef CURR_LOGIN_ACCOUNT_H
#define CURR_LOGIN_ACCOUNT_H

#include <QVector>
#include <QString>

#include "utils/contact_status.h"

class QQLoginCore;
class MainWindow;

class CurrLoginAccount
{
public:
	static QString id() 
	{ return id_; }
	static QString name()
	{ return name_; }
	static QString avatarPath()
	{ return avatar_path_; }
	static ContactStatus status()
	{ return status_; }

private:
	static void setId(const QString &id)
	{ id_ = id; }
	
	static void setName(const QString &name)
	{ name_ = name; }

	static void setAvatarPath(const QString &path)
	{ avatar_path_ = path; }

	static void setStatus(ContactStatus status)
	{ status_ = status; }

private:
	static QString id_;
	static QString name_;
	static QString avatar_path_;
	static ContactStatus status_;

private:
	CurrLoginAccount() {}
	CurrLoginAccount(const CurrLoginAccount&);
	CurrLoginAccount& operator=(const CurrLoginAccount&);

	friend class QQLoginCore;
	friend class MainWindow;
};

#endif //CURR_LOGIN_ACCOUNT_H
