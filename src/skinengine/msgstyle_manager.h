#ifndef MSGSTYLE_MANAGER_H
#define MSGSTYLE_MANAGER_H

#include <QtXml/QDomDocument>
#include <QFile>
#include <QString>

#include "qqglobal.h"

class MsgStyleManager
{
public:
	static MsgStyleManager *instance()
	{ 
		if ( !instance_ )
			instance_ = new MsgStyleManager();
		return instance_;
	}

	void setStyle(const QString &style)
	{ curr_style_ = style; }
	QString currStyle() const
	{ return curr_style_; }

	QString styleRes(const QString &res_name)
	{
        return currStyleDir() + '/' + res_.documentElement().attribute(res_name, "unset");
	}

	QString currStyleDir() const
	{
		return QQGlobal::messageStyleDir() + '/' + curr_style_;
	}

private:
	QDomDocument res_;
	QString curr_style_;

private:
	MsgStyleManager();
	MsgStyleManager(const MsgStyleManager &);
	MsgStyleManager &operator=(const MsgStyleManager &);

	static MsgStyleManager *instance_;
};

#endif //MSGSTYLE_MANAGER_H
