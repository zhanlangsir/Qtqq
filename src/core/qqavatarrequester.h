#ifndef QTQQ_CORE_QQAVATARREQUESTER_H
#define QTQQ_CORE_QQAVATARREQUESTER_H

#include <QHash>
#include <QList>
#include <QQueue>
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
    static QString requestOne(int type, QString id, QString save_path);
    static int getTypeNumber(QQItem::ItemType type);

protected:
    void run();

private:
    QList<QString> requesting_list_;
    QQueue<QQItem*> to_request_;

    bool finish_;
};

#endif //QTQQ_CORE_QQAVATARREQUESTER_H
