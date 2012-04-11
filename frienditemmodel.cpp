#include "frienditemmodel.h"

#include <QDateTime>

#include "include/json/json.h"
#include "nameconvertor.h"

void FriendItemModel::parse(const QByteArray &array, NameConvertor *convertor)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(array).toStdString(), root, false))
    {
        return;
    }

    const Json::Value category = root["result"]["categories"];

    createItem(QQItem::kCategory, tr("My Friends"), QString::number(0), root_);

    for (unsigned int i = 0; i < category.size(); ++i)
    {
        QString name =  QString::fromStdString(category[i]["name"].asString());
        createItem(QQItem::kCategory, name, QString::number(category[i]["index"].asInt()), root_);
    }

    createItem(QQItem::kCategory, tr("Strangers"), QString::number(99), root_);

    //mark name
    const Json::Value mark_names = root["result"]["marknames"];

    QHash <QString, QString> uin_markname;
    for (unsigned int i = 0; i < mark_names.size(); ++i)
    {
        QString uin = QString::number(mark_names[i]["uin"].asLargestInt());
        QString mark_name = QString::fromStdString(mark_names[i]["markname"].asString());

        uin_markname.insert(uin, mark_name);
    }

    //set friends
    const Json::Value friends = root["result"]["friends"];
    const Json::Value info = root["result"]["info"];

    for (unsigned int i = 0; i < friends.size(); ++i)
    {
        QString name = QString::fromStdString(info[i]["nick"].asString());
        QString uin = QString::number(info[i]["uin"].asLargestInt());

        FriendInfo *info = new FriendInfo;
        info->set_name(name);
        info->set_id(uin);
        info->set_status(kOffline);

        if (!uin_markname[uin].isEmpty())
        {
            info->set_markName(uin_markname[uin]);
        }

        int category_index = friends[i]["categories"].asInt();
        QQItem* parent = id_item_hash_[QString::number(category_index)];
        QQItem *myfriend = new QQItem(QQItem::kFriend, info, parent);

        id_item_hash_[uin] = myfriend;
        convertor->addUinNameMap(uin, name);
        convertor_ = convertor;

        parent->append(myfriend);
    }
}

void FriendItemModel::setMarkName(QString mark_name, QString id)
{
    QQItem *item = id_item_hash_.value(id, NULL);
    if (item)
        item->itemInfo()->set_markName(mark_name);
}

void FriendItemModel::addFriend(QString id, QString mark_name, QString groupidx, FriendStatus status)
{
    QString get_friend_info_url = "/api/get_friend_info2?tuin="+ id +"&verifysession=&code=&vfwebqq=" + CaptchaInfo::singleton()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    Request req;
    req.create(kGet, get_friend_info_url);
    req.addHeaderItem("Host", "s.web2.qq.com");
    req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    req.addHeaderItem("Connection", "keep-live");
    req.addHeaderItem("Content-Type","utf-8");
    req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());

    QTcpSocket fd;
    fd.connectToHost("s.web2.qq.com", 80);

    fd.write(req.toByteArray());
    fd.waitForReadyRead();
    QString result = fd.readAll();
    result = result.mid(result.indexOf("\r\n\r\n")+4);

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(result).toStdString(), root, false))
    {
        return;
    }

    QString name = QString::fromStdString(root["result"]["nick"].asString());

    FriendInfo *info = new FriendInfo;
    info->set_name(name);
    info->set_id(id);
    info->set_status(status);

    if (!mark_name.isEmpty())
    {
        info->set_markName(mark_name);
    }

    QQItem* parent = id_item_hash_[groupidx];
    QQItem *myfriend = new QQItem(QQItem::kFriend, info, parent);

    id_item_hash_[id] = myfriend;
    convertor_->addUinNameMap(id, name);

    int parent_idx = root_->indexOf(parent);
    QModelIndex parent_mdl_idx = index(parent_idx,0, QModelIndex());

    beginInsertRows(parent_mdl_idx, 0, 0);
    parent->insert(getNewPosition(myfriend), myfriend);
    endInsertRows();
}

int FriendItemModel::getNewPosition(const QQItem *item) const
{
    QQItem *category = item->parent();
    for ( int i = 0; i < category->children_.count(); ++i )
    {
        if (category->children_[i]->status() >= item->status())
            return i;
    }
    return category->children_.count();
}

QQItem *FriendItemModel::category(QString idx) const
{
    return id_item_hash_.value(idx, NULL);
}

QVector<QQItem *> FriendItemModel::categorys() const
{
    return root_->children_;
}

QQItem *FriendItemModel::createItem(QQItem::ItemType type, QString name, QString id, QQItem* parent)
{
    ItemInfo *item = new ItemInfo;
    item->set_name(name);
    item->set_id(id);
    QQItem *cat = new QQItem(type, item, parent);
    parent->append(cat);

    id_item_hash_[id] = cat;

    return NULL;
}
