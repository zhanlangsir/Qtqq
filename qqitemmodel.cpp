#include "qqitemmodel.h"
#include "include/json/json.h"
#include <QPixmap>
#include <QIcon>
#include <assert.h>

QVariant QQItemModel::data(const QModelIndex &index, int role) const
{
    QQItem *item = itemFromIndex(index);
    if (!item)
    {
        return QVariant();
    }
 
    if (role == Qt::DecorationRole)
    {
        if (item->type() == QQItem::kCategory)
            return QVariant();

        QIcon icon("1.bmp");
        QPixmap pix;
        if (item->type() == QQItem::kFriend)
        {
            if (item->state() == kLeave)
            {
                pix = icon.pixmap(QSize(60, 60), QIcon::Disabled, QIcon::On);
                return pix;
            }
        }

        pix = icon.pixmap(QSize(60,60));

        return pix;
    }

    if (role != Qt::DisplayRole)
    {

        return QVariant();
    }


   else
    {
        return item->name();
    }
    return QVariant();
}

void QQItemModel::setRoot(QQItem *root)
{
    root_ = root;
}

QModelIndex QQItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!root_ || row < 0 || column < 0)
        return QModelIndex();

    QQItem *parentItem = itemFromIndex(parent);
    QQItem *childItem = parentItem->value(row);


    if (!childItem)
        return QModelIndex();

    return createIndex(row, column, childItem);
}

QModelIndex QQItemModel::parent(const QModelIndex &child) const
{

    QQItem *item = itemFromIndex(child);
    if (!item)
        return QModelIndex();

    QQItem *parent = item->parent();
    if (!parent)
        return QModelIndex();

    QQItem *grandparent = parent->parent();
    if (!grandparent)
        return QModelIndex();

    int row = grandparent->indexOf(parent); 

    return createIndex(row, 0, parent); 
}

int QQItemModel::rowCount(const QModelIndex &parent) const
{
   QQItem *item = itemFromIndex(parent);
   if (!item)
       return 0;
   return item->count();
}

int QQItemModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QQItem* QQItemModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) 
    {
        return static_cast<QQItem*>(index.internalPointer());
    }
    else
        return root_;
}

QQItemModel::QQItemModel(QObject *parent) : QAbstractItemModel(parent)
{

}
