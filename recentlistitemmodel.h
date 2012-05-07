#ifndef QTQQ_RECENTLISTITEMMODEL_H
#define QTQQ_RECENTLISTITEMMODEL_H

#include <QVector>

#include "qqitemmodel.h"
#include "qqitem.h"

class FriendItemModel;
class GroupItemModel;

class RecentListItemModel : public QQItemModel
{
    Q_OBJECT
public:
    RecentListItemModel(FriendItemModel *friend_model, GroupItemModel *group_model, QObject *parent = 0) : QQItemModel(parent),
   friend_model_(friend_model),
   group_model_(group_model) {}

public:
    void parse(const QByteArray &array);

public slots:
    void improveItem(QString id);

protected:
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;

private:
    FriendItemModel *friend_model_;
    GroupItemModel *group_model_;
    QVector<QQItem*> items_;
};

#endif // QTQQ_RECENTLISTITEMMODEL_H
