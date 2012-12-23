#include "qqitemmodel.h"

#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <QPainter>
#include <QVector>

#include <assert.h>

#include "json/json.h"
#include "core/qqitem.h"
#include "skinengine/qqskinengine.h"

QQItemModel::QQItemModel(QObject *parent) :
    QAbstractItemModel(parent),
    icon_size_(40, 40),
    root_(new QQItem())
{
    connect(this, SIGNAL(noAvatar(QQItem*)), this, SLOT(requestAvatar(QQItem*)));
}

QQItemModel::~QQItemModel()
{
    avatar_requester_.finishRequest();
    avatar_requester_.wait();

    //root_会被递归删除子元素，在QQItem中可以看到
    if (root_)
    {
        delete root_;
        root_ = NULL;
    }
}

void QQItemModel::notifyItemDataChanged(QQItem * item)
{
	 QQItem *parent = item->parent();
     QModelIndex index = createIndex(parent->children_.indexOf(item), 0, item);
	 dataChanged(index, index);
}

void QQItemModel::insertItem(QQItem *item)
{
    assert(item);
    root_->children_.append(item);

    items_.append(item);
}

void QQItemModel::insertItem(QQItem *item, QQItem *parent)
{
    assert(item && parent);
    int parent_idx = items_.indexOf(parent);
    assert(parent_idx != -1);

    parent->children_.append(item);
    items_.append(item);
}

void QQItemModel::pushFront(QQItem *item)
{
    assert(item);
    root_->children_.push_front(item);

    items_.append(item);
}

void QQItemModel::pushBack(QQItem *item)
{
    assert(item);
    root_->children_.push_back(item);

    items_.append(item);
}

void QQItemModel::improveItem(QString id)
{
    QQItem *item = find(id);
    if (item)
    {
        int idx = item->parent()->children_.indexOf(item);
        beginInsertRows(QModelIndex(), 0, 0);
        item->parent()->children_.remove(idx);
        item->parent()->children_.push_front(item);
        endInsertRows();
    }
}

QQItem *QQItemModel::rootItem() const
{
    return root_;
}

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

        QPixmap pix(icon_size_);
        if (item->avatarPath().isEmpty())
        {
            if (!avatar_requester_.isRequesting(item->id()))
                emit noAvatar(item);
            getDefaultPixmap(item, pix);
        }
        else
            getPixmap(item, pix);
      
        return pix;
    }

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    else
    {
        if ( item->type() == QQItem::kCategory )
            return item->markName() + " ( " + QString::number(item->onlineCount()) + "/" + 
                QString::number(item->children_.count()) + " )";
        return item->markName();
    }
    return QVariant();
}

QQItem* QQItemModel::find(QString id) const
{
    QQItem *item = NULL;
    foreach (item, items_)
    {
        if (item->id() == id)
            return item;
    }

    return NULL;
}

void QQItemModel::setPixmapDecoration(const QQItem *item, QPixmap &pixmap) const
{
    if (item->status() == CS_Online || item->status() == CS_Offline)
        return;

    QPainter painter(&pixmap);
    QImage img;

    if (item->status() == CS_CallMe)
    {
        img.load(QQGlobal::resourceDir() + "/avatar/callme.png");
    }
    else if (item->status() == CS_Busy)
    {
        img.load(QQGlobal::resourceDir() + "/avatar/busy.png");
    }
    else if (item->status() == CS_Away)
    {
        img.load(QQGlobal::resourceDir() + "/avatar/away.png");
    }
    else if (item->status() == CS_Silent)
    {
        img.load(QQGlobal::resourceDir() + "/avatar/silent.png");
    }

    QSize avatar_size = pixmap.size();
    QSize decoration_size = img.size();
    QPointF draw_point(avatar_size.width() - decoration_size.width(), avatar_size.height() - decoration_size.height());
    painter.drawImage(draw_point, img);
}

void QQItemModel::getDefaultPixmap(const QQItem *item, QPixmap &pix) const
{
    if (item->type() == QQItem::kFriend)
    {
        QIcon icon(QQSkinEngine::instance()->skinRes("default_friend_avatar"));

        if (item->status() == CS_Offline)
        {
            pix = icon.pixmap(QSize(icon_size_), QIcon::Disabled, QIcon::On);
        }
        else
            pix = icon.pixmap(QSize(icon_size_));
    }

    if (item->type() == QQItem::kGroup)
    {
        QIcon icon(QQSkinEngine::instance()->skinRes("default_group_avatar"));
        pix = icon.pixmap(QSize(icon_size_));
    }
}

void QQItemModel::getPixmap(const QQItem *item, QPixmap &pix) const
{
    QFile file(item->avatarPath());
    file.open(QIODevice::ReadOnly);

    QByteArray file_data = file.readAll();
    pix.loadFromData(file_data);
    file.close();

    QIcon icon;
    icon.addPixmap(pix);

    if (item->status() == CS_Offline)
    {
        pix = icon.pixmap(QSize(icon_size_), QIcon::Disabled, QIcon::On);
    }
    else
    {
        pix = icon.pixmap(QSize(icon_size_));
        setPixmapDecoration(item,pix);
    }
}

void QQItemModel::requestAvatar(QQItem *item)
{
	avatar_requester_.request(item);
}

QModelIndex QQItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!root_ || row < 0 || column < 0)
        return QModelIndex();

    QQItem *parentItem = itemFromIndex(parent);
    QQItem *childItem = parentItem->children_.value(row);


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

    int row = grandparent->children_.indexOf(parent); 

    return createIndex(row, 0, parent); 
}

int QQItemModel::rowCount(const QModelIndex &parent) const
{
   QQItem *item = itemFromIndex(parent);
   if (!item)
       return 0;
   return item->children_.count();
}

int QQItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QQItem* QQItemModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) 
    {
        return static_cast<QQItem*>(index.internalPointer());
    }
    else
    {
        return root_;
    }
}
