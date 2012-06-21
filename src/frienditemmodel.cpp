#include "frienditemmodel.h"

#include <QDateTime>
#include <QTcpSocket>

#include "include/json.h"
#include "core/nameconvertor.h"
#include "core/captchainfo.h"

void FriendItemModel::parse(const QByteArray &array, NameConvertor *convertor)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(array).toStdString(), root, false))
    {
        return;
    }

    const Json::Value category = root["result"]["categories"];

    QQItem *myfriends_cat = new QQItem(QQItem::kCategory, tr("My Friends"), QString::number(0), root_);
    root_->children_.append(myfriends_cat);
    items_.append(myfriends_cat);

    for (unsigned int i = 0; i < category.size(); ++i)
    {
        QString name =  QString::fromStdString(category[i]["name"].asString());
        QQItem *cat = new QQItem(QQItem::kCategory, name, QString::number(category[i]["index"].asInt()), root_);
        root_->children_.append(cat);
        items_.append(cat);
    }

    QQItem *cat = new QQItem(QQItem::kCategory, tr("Strangers"), QString::number(99), root_);
    root_->children_.append(cat);
    items_.append(cat);

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

        QQItem *item = new QQItem();
        item->set_type(QQItem::kFriend);
        item->set_name(name);
        item->set_id(uin);
        item->set_status(kOffline);

        if (!uin_markname[uin].isEmpty())
        {
            item->set_markName(uin_markname[uin]);
        }

        int category_index = friends[i]["categories"].asInt();
        QQItem* parent = find(QString::number(category_index));
        item->set_parent(parent);

        items_.append(item);
        convertor->addUinNameMap(uin, name);
        convertor_ = convertor;

        parent->children_.append(item);
    }
}

void FriendItemModel::setMarkName(QString mark_name, QString id)
{
    QQItem *item = find(id);

    if (item)
        item->set_markName(mark_name);
}

void FriendItemModel::addItem(QString id, QString mark_name, QString groupidx, FriendStatus status)
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

    QQItem *item = new QQItem();
    item->set_name(name);
    item->set_id(id);
    item->set_status(status);

    if (!mark_name.isEmpty())
    {
        item->set_markName(mark_name);
    }

    QQItem* parent = find(groupidx);
    item->set_parent(parent);
    parent->children_.append(item);

    if (status != kOffline)
        parent->setOnlineCount(parent->onlineCount() + 1);

    items_.append(item);
    convertor_->addUinNameMap(id, name);

    int parent_idx = root_->children_.indexOf(parent);
    QModelIndex parent_mdl_idx = index(parent_idx,0, QModelIndex());

    beginInsertRows(parent_mdl_idx, 0, 0);
    parent->children_.insert(getNewPosition(item), item);
    endInsertRows();
}

void FriendItemModel::changeFriendStatus(QString id, FriendStatus status, ClientType client_type)
{
    QQItem *item = find(id);
    if (!item)
    {
        return;
    }

    QQItem *category = item->parent();

    if ( item->status() == kOffline && status != kOffline )
        category->setOnlineCount(category->onlineCount() + 1);
    else if ( item->status() != kOffline && status == kOffline )
        category->setOnlineCount(category->onlineCount() - 1);

    item->set_clientType(client_type);
    item->set_status(status);

    int parent_idx = category->children_.indexOf(item);

    QModelIndex parent_mdl_idx = index(root_->children_.indexOf(category), 0, QModelIndex());
    beginRemoveRows(parent_mdl_idx, 0, 0);
    category->children_.remove(parent_idx);
    endRemoveRows();

    int new_idx = getNewPosition(item);
    beginInsertRows(parent_mdl_idx, 0, 0);
    category->children_.insert(new_idx, item);
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
    return find(idx);
}

QVector<QQItem *> FriendItemModel::categorys() const
{
    return root_->children_;
}
