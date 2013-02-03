#include "frienditemmodel.h"

#include <QDateTime>
#include <QTcpSocket>
#include <QThreadPool>
#include <QPixmap>

#include "json/json.h"
#include "core/nameconvertor.h"
#include "core/captchainfo.h"
#include "core/tasks.h"
#include "core/curr_login_account.h"

void FriendItemModel::parse(const QByteArray &array, NameConvertor *convertor)
{
	/*
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(array).toStdString(), root, false))
    {
        return;
    }

    const Json::Value category = root["result"]["categories"];

	CurrLoginAccount *cla = CurrLoginAccount::instance();
	Category *myfriend = new Category(0, tr("My Friend"));
	cla->addCategory(myfriend);


    for (unsigned int i = 0; i < category.size(); ++i)
    {
        QString name =  QString::fromStdString(category[i]["name"].asString());
		int index = category[i]["index"].asInt();
		Category *cat = new Category(index, name);
		cla->addCategory(cat);
    }

	Category *stranger = new Category(99, tr("Strangers"));
	cla->addCategory(stranger);



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
		QString id = QString::number(info[i]["uin"].asLargestInt());
		Contact *contact = new Contact(id, name);
		contact->setStatus(CS_Offline);
		//contact->setMark(uin_markname.value(id, ""));

        int category_index = friends[i]["categories"].asInt();
		cla->addContact(contact, category_index);


        if ( contact->type() != Talkable::kCategory )
        {
            GetAvatarTask *task = new GetAvatarTask(contact, this);
            QThreadPool::globalInstance()->start(task);
        }
    }
	categorys_ = cla->categorys();
	*/
}

void FriendItemModel::setMarkName(QString mark_name, QString id)
{
    QQItem *item = find(id);

    if (item)
        item->set_markName(mark_name);
}

void FriendItemModel::addItem(QString id, QString mark_name, QString groupidx, ContactStatus status)
{
    QString get_friend_info_url = "/api/get_friend_info2?tuin="+ id +"&verifysession=&code=&vfwebqq=" + CaptchaInfo::instance()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    Request req;
    req.create(kGet, get_friend_info_url);
    req.addHeaderItem("Host", "s.web2.qq.com");
    req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    req.addHeaderItem("Connection", "keep-live");
    req.addHeaderItem("Content-Type","utf-8");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

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

    //if (status != kOffline)
     //   parent->setOnlineCount(parent->onlineCount() + 1);

    items_.append(item);
    convertor_->addUinNameMap(id, name);

    int parent_idx = root_->children_.indexOf(parent);
    QModelIndex parent_mdl_idx = index(parent_idx,0, QModelIndex());

    beginInsertRows(parent_mdl_idx, 0, 0);
    parent->children_.insert(getNewPosition(item), item);
    endInsertRows();
}

void FriendItemModel::changeFriendStatus(QString id, ContactStatus status, ContactClientType client_type)
{
    QQItem *item = find(id);
    if (!item)
    {
        return;
    }

    QQItem *category = item->parent();

    if ( item->status() == CS_Offline && status != CS_Offline )
        category->setOnlineCount(category->onlineCount() + 1);
    else if ( item->status() != CS_Offline && status == CS_Offline )
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


QModelIndex FriendItemModel::index(int row, int column, const QModelIndex &parent) const
{
	/*
	if ( row < 0 || column < 0 )
		return QModelIndex();

	if ( !parent.isValid() )	
		return createIndex(row, column, categorys_.at(row));
	else
	{
		Talkable *talkable = static_cast<Talkable*>(parent.internalPointer());
		if ( talkable->type() == Talkable::kCategory )
		{
			Category *cat = static_cast<Category*>(talkable);
			Contact *contact = cat->contacts.at(row);
			if ( contact )
				return createIndex(row, column, contact);
		}

		return QModelIndex();
	}
	*/
}

QVariant FriendItemModel::data(const QModelIndex &index, int role) const
{
	if ( !index.isValid() )
		return QVariant();

	Talkable *talkable = static_cast<Talkable*>(index.internalPointer());
	if (!talkable)
	{
		return QVariant();
	}

	if (role == Qt::DecorationRole)
	{
		if (talkable->type() == Talkable::kCategory)
			return QVariant();

		QPixmap pix(icon_size_);
		if (talkable->avatarPath().isEmpty())
		{
			//if (!avatar_requester_.isRequesting(talkable->id()))
				//emit noAvatar(item);
			//getDefaultPixmap(item, pix);
		}
		else
			;
			//getPixmap(item, pix);

		return pix;
	}

	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}
	else
	{
		if ( talkable->type() == Talkable::kCategory )
		{
			return "test";
			Category *cat = NULL;//static_cast<Category*>(talkable);
			return cat->name() + " ( " + QString::number(getOnlineContactCount(cat)) + "/" + QString::number(cat->contacts.count()) + " )";
		}

		return talkable->name();
	}
	return QVariant();
}

int FriendItemModel::getOnlineContactCount(Category *cat) const
{
	int n_online = 0;
	foreach ( Contact *contact, cat->contacts )
	{
		if ( contact->status() == CS_Online )
		{
			++n_online;
		}
	}

	return n_online;
}

QModelIndex FriendItemModel::parent(const QModelIndex &child) const
{
	if ( !child.isValid() )
		return QModelIndex();

	Talkable *talkable = static_cast<Talkable*>(child.internalPointer());
    if (!talkable)
        return QModelIndex();

	if ( talkable->type() == Talkable::kCategory )
        return QModelIndex();
	else if ( talkable->type() == Talkable::kContact )
	{
		Contact *contact = static_cast<Contact*>(talkable);
		Category *cat = contact->category();
		int row = categorys_.indexOf(cat);
		return createIndex(row, 0, cat);
	}

    return QModelIndex();
}

int FriendItemModel::rowCount(const QModelIndex &parent) const
{
	if ( !parent.isValid() )
		return categorys_.count();

	Talkable *talkable = static_cast<Talkable*>(parent.internalPointer());
    if (!talkable)
        return 0;

	if ( talkable->type() == Talkable::kCategory )
	{
		Category *cat = static_cast<Category*>(parent.internalPointer());
        return cat->contacts.count();
	}

	return 0;
}

int FriendItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}
