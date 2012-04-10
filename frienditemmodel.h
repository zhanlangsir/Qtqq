#ifndef QTQQ_FRIENDITEMMODEL_H
#define QTQQ_FRIENDITEMMODEL_H

#include "qqitemmodel.h"

class NameConvertor;

class FriendItemModel : public QQItemModel
{
public:
    FriendItemModel(QObject *parent = 0) : QQItemModel(parent) {}
public:
    void parse(const QByteArray &array, NameConvertor *convertor);
    void setMarkName(QString mark_name, QString id);
    void addFriend(QString id, QString mark_name, QString groupidx, FriendStatus status);
    QQItem *category(QString idx) const;
    QVector<QQItem*> categorys() const;
    int getNewPosition(const QQItem *item) const;

private:
    QQItem* createItem(QQItem::ItemType type, QString name, QString id, QQItem *parent);
    NameConvertor *convertor_;
};

#endif // QTQQ_FRIENDITEMMODEL_H
