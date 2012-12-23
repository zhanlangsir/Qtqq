#ifndef QQSKINENGINE_H
#define QQSKINENGINE_H

#include <QtXml/QDomDocument>
#include <QVector>
#include <QApplication>
#include <QFile>

#include "core/qqskinable.h"
#include "qqglobal.h"

class QQSkinEngine : public QObject
{
    Q_OBJECT
signals:
    void skinChanged();

public:
    static QQSkinEngine *instance()
    {
        if ( !instance_ )
            instance_ = new QQSkinEngine();

        return instance_;
    }

	void setSkin(QString skin_name)
	{
		curr_skin_ = skin_name;

		QFile file(currSkinDir() + "/style.css");
        file.open(QIODevice::ReadOnly);
        QString style = file.readAll();
        style.replace("%skin_path%", currSkinDir());
        qApp->setStyleSheet(style);

        emit skinChanged();
	}

    void addSkinableWidget(QQSkinable *skinable_wid)
    {
        skinable_widgets_.append(skinable_wid);
    }

    QString skinRes(QString res_name)
    {
        return currSkinDir() + '/' + res_.documentElement().attribute(res_name, "unset");
    }

	QString currSkinDir() const
	{ return QQGlobal::skinsDir()+'/'+curr_skin_; }

private:
    QVector<QQSkinable*> skinable_widgets_;
    QDomDocument res_;
    QString curr_skin_;

private:
    QQSkinEngine(QObject *parent = NULL) : QObject(parent),
        res_("xml"),
		curr_skin_("default")
    {
        QFile file(currSkinDir() + "/res.xml");
        file.open(QIODevice::ReadOnly);

        res_.setContent(&file);
        file.close();

		setSkin(curr_skin_);
    }
	QQSkinEngine(const QQSkinEngine &);
	QQSkinEngine &operator=(const QQSkinEngine &);

    static QQSkinEngine *instance_;
};

#endif //QQSKINENGINE_H
