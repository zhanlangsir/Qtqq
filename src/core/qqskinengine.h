#ifndef QTQQ_CORE_QQSKINENGINE_H
#define QTQQ_CORE_QQSKINENGINE_H

#include <QtXml/QDomDocument>
#include <QVector>
#include <QApplication>
#include <QFile>
#include <QDebug>

#include "qqskinable.h"
#include "qqsetting.h"

class QQSkinEngine : public QObject
{
    Q_OBJECT
public:
    static QQSkinEngine *instance()
    {
        if (!qqskin_engine_)
            qqskin_engine_ = new QQSkinEngine();

        return qqskin_engine_;
    }

    void updateSkin(QString skin_name)
    {
        curr_skin_ = skin_name;
        QQSettings::instance()->setSkin(skin_name);

        QFile file(QQSettings::instance()->currSkinPath() + "/style.css");
        file.open(QIODevice::ReadOnly);
        QString style = file.readAll();
        style.replace("%skin_path%", QQSettings::instance()->currSkinPath());
        qApp->setStyleSheet(style);

        emit skinChanged();
    }

    void addSkinableWid(QQSkinable *skinable_wid)
    {
        skinable_wids_.append(skinable_wid);
    }

    QString getSkinRes(QString res_name)
    {
        return QQSettings::instance()->currSkinPath() + "/" + doc_.documentElement().attribute(res_name, "unset");
    }

signals:
    void skinChanged();

private:
    QQSkinEngine(QObject *parent = NULL) : QObject(parent),
        doc_("xml")
    {
        QFile file(QQSettings::instance()->currSkinPath() + "/res.xml");
        file.open(QIODevice::ReadOnly);

        doc_.setContent(&file);
        file.close();
    }

private:
    static QQSkinEngine *qqskin_engine_;
    QVector<QQSkinable*> skinable_wids_;
    QDomDocument doc_;
    QString curr_skin_;
};

#endif // QTQQ_CORE_QQSKINENGINE_H
