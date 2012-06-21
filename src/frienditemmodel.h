#ifndef QTQQ_FRIENDITEMMODEL_H
#define QTQQ_FRIENDITEMMODEL_H

#include "qqitemmodel.h"

class NameConvertor;

class FriendItemModel : public QQItemModel
{
    Q_OBJECT
public:
    FriendItemModel(QObject *parent = 0) : QQItemModel(parent)
    {
    }

public:
    void parse(const QByteArray &array, NameConvertor *convertor);
    void setMarkName(QString mark_name, QString id);
    void addItem(QString id, QString mark_name, QString groupidx, FriendStatus status);
    QQItem *category(QString idx) const;
    QVector<QQItem*> categorys() const;
    int getNewPosition(const QQItem *item) const;
    
public slots:
    void changeFriendStatus(QString id, FriendStatus status, ClientType client_type);

private:
    NameConvertor *convertor_;
};

#endif // QTQQ_FRIENDITEMMODEL_H
