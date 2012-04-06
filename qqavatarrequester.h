#pragma once

#include <QHash>
#include <QList>
#include <QQueue>
#include <QTcpSocket>
#include <QThread>

#include "request.h"
#include "types.h"
#include "qqitem.h"

class QQAvatarRequester : public QThread
{
public:
    QQAvatarRequester() : finish_(false) {}

public:
    bool isRequesting(QString id) const; 
    void request(QQItem *info);
    void finishRequest();

protected:
    void run();

private:
    int getTypeNumber(const QQItem *info) const;
    QString getFileFormat(const QByteArray &array) const;

private:
    QList<QString> requesting_list_;
    QQueue<QQItem*> to_request_;

    bool finish_;
};
