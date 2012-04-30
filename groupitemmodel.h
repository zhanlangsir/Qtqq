#ifndef QTQQ_GROUPITEMMODEL_H
#define QTQQ_GROUPITEMMODEL_H

#include "qqitemmodel.h"

class NameConvertor;

class GroupItemModel : public QQItemModel
{
public:
    GroupItemModel(QObject *parent = 0) : QQItemModel(parent)
    {
    }
public:
    void parse(const QByteArray &array, NameConvertor *convertor);

private:
    //QQItem* createItem(QQItem::ItemType type, QString name, QString id, QQItem *parent);
};

#endif // QTQQ_GROUPITEMMODEL_H
