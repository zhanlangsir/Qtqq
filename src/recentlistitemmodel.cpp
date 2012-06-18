#include "recentlistitemmodel.h"

#include "include/json.h"

#include "frienditemmodel.h"
#include "groupitemmodel.h"

QModelIndex RecentListItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!root_ || row < 0 || column < 0)
        return QModelIndex();

    QQItem *parent_item = itemFromIndex(parent);
    if (parent_item != root_)
        return QModelIndex();

    if (items_.count() < row+1)
        return QModelIndex();

    return createIndex(row, column, items_[row]);
}


void RecentListItemModel::parse(const QByteArray &array)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(array).toStdString(), root, false))
    {
        return;
    }

    const Json::Value result = root["result"];

    for (unsigned int i = 0; i < result.size(); ++i)
    {
        QString id = QString::number(result[i]["uin"].asLargestInt());
        int type = result[i]["type"].asInt();

        QQItem *item = NULL;
        if (type == 1)
        {
            item = group_model_->find(id);
        }
        else if(type == 0)
        {
            item = friend_model_->find(id);
        }
        if (!item)
            continue;

        QQItem *recent_list_item =  item;

        items_.append(recent_list_item);
    }
}

int RecentListItemModel::rowCount(const QModelIndex &parent) const
{
    if (itemFromIndex(parent) != root_)
        return 0;

    return items_.count();
}


QModelIndex RecentListItemModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

void RecentListItemModel::improveItem(QString id)
{
    QQItem *item = find(id);
    if (!item)
    {
        item = friend_model_->find(id);
        if (!item)
        {
            item = group_model_->find(id);
        }
        if (!item)
            return;
        beginInsertRows(QModelIndex(), 0, 0);
        items_.push_front(item);
        endInsertRows();
    }
    else
    {
        int idx = items_.indexOf(item);
        beginInsertRows(QModelIndex(), 0, 0);
        items_.remove(idx); 
        items_.push_front(item);
        endInsertRows();
    }
}
