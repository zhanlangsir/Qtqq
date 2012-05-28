#include "qqitemmodel.h"
#include "include/json/json.h"
#include <QPixmap>
#include <QIcon>
#include <assert.h>
#include <QFile>
#include <QPainter>

#include "core/qqitem.h"
#include "core/qqskinengine.h"
#include "core/qqsetting.h"

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

        QPixmap pix;
        if (item->avatarPath().isEmpty())
        {
            if (!avatar_requester_.isRequesting(item->id()))
                emit noAvatar(item);
            pix = getDefaultPixmap(item);
        }
        else
            pix = getPixmap(item);
      
        return pix;
    }

    if (role != Qt::DisplayRole)
    {

        return QVariant();
    }
    else
    {
        return item->markName();
    }
    return QVariant();
}

QQItem* QQItemModel::find(QString id)
{
    return id_item_hash_.value(id, NULL);
}

void QQItemModel::setPixmapDecoration(const QQItem *item, QPixmap &pixmap) const
{
    if (item->status() == kOnline || item->status() == kOffline)
        return;

    QPainter painter(&pixmap);
    QImage img;

    if (item->status() == kCallMe)
    {
        img.load(QQSettings::instance()->resourcePath() + "/avatar/callme.png");
    }
    else if (item->status() == kBusy)
    {
        img.load(QQSettings::instance()->resourcePath() + "/avatar/busy.png");
    }
    else if (item->status() == kAway)
    {
        img.load(QQSettings::instance()->resourcePath() + "/avatar/away.png");
    }
    else if (item->status() == kSilent)
    {
        img.load(QQSettings::instance()->resourcePath() + "/avatar/silent.png");
    }

    QSize avatar_size = pixmap.size();
    QSize decoration_size = img.size();
    QPointF draw_point(avatar_size.width() - decoration_size.width(), avatar_size.height() - decoration_size.height());
    painter.drawImage(draw_point, img);
}

QPixmap QQItemModel::getDefaultPixmap(const QQItem *item) const
{
    QPixmap pix;
    if (item->type() == QQItem::kFriend)
    {
        QIcon icon(QQSkinEngine::instance()->getSkinRes("default_friend_avatar"));

        if (item->status() == kOffline)
        {
            pix = icon.pixmap(QSize(60, 60), QIcon::Disabled, QIcon::On);
        }
        else
            pix = icon.pixmap(QSize(60,60));
    }

    if (item->type() == QQItem::kGroup)
    {
        QIcon icon(QQSkinEngine::instance()->getSkinRes("default_group_avatar"));
        pix = icon.pixmap(QSize(60,60));
    }
    return pix;
}

QPixmap QQItemModel::getPixmap(const QQItem *item) const
{
    QFile file(item->avatarPath());
    file.open(QIODevice::ReadOnly);

    QPixmap pix(60, 60);
    QByteArray file_data = file.readAll();
    pix.loadFromData(file_data);
    file.close();

    QIcon icon;
    icon.addPixmap(pix);

    if (item->status() == kOffline)
    {
        pix = icon.pixmap(QSize(60, 60), QIcon::Disabled, QIcon::On);
    }
    else
    {
        pix = icon.pixmap(QSize(60,60));
        setPixmapDecoration(item,pix);
    }

    return pix;
}

void QQItemModel::requestAvatar(QQItem *item)
{
    avatar_requester_.request(item);
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
        return root_;
}

QQItemModel::QQItemModel(QObject *parent) : QAbstractItemModel(parent), root_(new QQItem())
{
    ItemInfo *info = new ItemInfo();
    info->set_markName("root");
    root_->set_itemInfo(info);
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
