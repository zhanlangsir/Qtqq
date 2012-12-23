#include "roster.h"

#include <assert.h>

#include <QDebug>

#include "json/json.h"

#include "core/qqutility.h"
#include "qq_protocol/qq_protocol.h"

Roster* Roster::instance_ = NULL;

Roster::Roster()
{
	connect(this, SIGNAL(sigContactDataChanged(QString, QVariant, TalkableDataRole)), this, SIGNAL(sigTalkableDataChanged(QString, QVariant, TalkableDataRole)));
	connect(this, SIGNAL(sigGroupDataChanged(QString, QVariant, TalkableDataRole)), this, SIGNAL(sigTalkableDataChanged(QString, QVariant, TalkableDataRole)));
}


Roster::~Roster()
{
	clean();
	instance_ = NULL;
}


void Roster::parseContact(const QByteArray &array)
{
	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(QString(array).toStdString(), root, false))
	{
		qDebug() << "parse friend error!" << endl;
		return;
	}

	const Json::Value category = root["result"]["categories"];

	Category *myfriend = new Category(0, tr("My Friend"));
	categorys_.push_back(myfriend);
	emit sigNewCategory(myfriend);


	for (unsigned int i = 0; i < category.size(); ++i)
	{
		QString name =  QString::fromStdString(category[i]["name"].asString());
		int index = category[i]["index"].asInt();
		Category *cat = new Category(index, name);

		categorys_.push_back(cat);
		emit sigNewCategory(cat);
	}

	Category *stranger = new Category(99, tr("Strangers"));
	categorys_.push_back(stranger);
	emit sigNewCategory(stranger);

	//mark name
	const Json::Value mark_names = root["result"]["marknames"];

	QHash <QString, QString> uin_markname;
	for (unsigned int i = 0; i < mark_names.size(); ++i)
	{
		QString uin = QString::number(mark_names[i]["uin"].asLargestInt());
		QString mark_name = QString::fromStdString(mark_names[i]["markname"].asString());

		uin_markname.insert(uin, mark_name);
	}

	//Protocol::QQProtocol *proto = Protocol::QQProtocol::instance();
	//set friends
	const Json::Value friends = root["result"]["friends"];
	const Json::Value info = root["result"]["info"];

	QVector<Category *> changed_cat;
	for (unsigned int i = 0; i < friends.size(); ++i)
	{
		QString name = QString::fromStdString(info[i]["nick"].asString());
		QString id = QString::number(info[i]["uin"].asLargestInt());
		Contact *contact = new Contact(id, name);
		contact->setStatus(CS_Offline);
		contact->setMarkname(uin_markname.value(id, ""));

		int category_index = friends[i]["categories"].asInt();
		Category *cat = this->category(category_index);
		addContact(contact, cat);

		changed_cat.push_back(cat);
	}
}

void Roster::addContact(Contact *contact, Category *cat)
{
	if ( contacts_.contains(contact->id()) )
		return;

	contact->setCategory(cat);
	cat->contacts.append(contact);

	contacts_.insert(contact->id(), contact);	
	emit sigNewContact(contact);


	Protocol::QQProtocol *proto = Protocol::QQProtocol::instance();
	if ( !proto->isRequesting(contact->id(), JT_Icon) )
	{
		IconRequestCallback *callback = new IconRequestCallback(contact->id());
		connect(callback, SIGNAL(sigRequestDone(QString, QByteArray)), this, SLOT(slotIconRequestDone(QString, QByteArray)));
		proto->requestIconFor(contact->id(), callback);
	}
}

void Roster::parseGroup(const QByteArray &array)
{
	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(QString(array).toStdString(), root, false))
	{
		return;
	}

	const Json::Value result = root["result"];
	const Json::Value gnamelist = result["gnamelist"];

	Protocol::QQProtocol *proto = Protocol::QQProtocol::instance();

	for (unsigned int i = 0; i < gnamelist.size(); ++i)
	{
		QString name = QString::fromStdString(gnamelist[i]["name"].asString());
		QString gid = QString::number(gnamelist[i]["gid"].asLargestInt());
		QString code = QString::number(gnamelist[i]["code"].asLargestInt());

		Group *group = new Group(gid, code, name);
		groups_.insert(gid, group);

		emit sigNewGroup(group);

		IconRequestCallback *callback = new IconRequestCallback(gid);
		connect(callback, SIGNAL(sigRequestDone(QString, QByteArray)), this, SLOT(slotIconRequestDone(QString, QByteArray)));
		proto->requestIconFor(gid, callback);
	}

}


void Roster::parseContactStatus(const QByteArray &array)
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
		QString status = QString::fromStdString(result[i]["status"].asString());
		ContactClientType client_type = (ContactClientType)result[i]["client_type"].asInt();

		Contact *contact = this->contact(id);
		assert(contact);

		slotContactStatusChanged(id, QQUtility::stringToStatus(status), client_type);
	}
}


void Roster::slotIconRequestDone(QString id, QByteArray icon_data)
{
	Talkable *talkable = this->talkable(id);	
	talkable->setIcon(icon_data);

	if ( talkable->type() == Talkable::kContact )
		emit sigContactDataChanged(talkable->id(), talkable->icon(), TDR_Icon);
	else if ( talkable->type() == Talkable::kGroup )
		emit sigGroupDataChanged(talkable->id(), talkable->icon(), TDR_Icon);
}


void Roster::slotContactStatusChanged(QString id, ContactStatus status, ContactClientType type)
{
	Contact *contact = this->contact(id);
	/*
	 * Why i can recive a status change message 
	 * from contact that is not my friend
	 */
	if ( !contact ) 
		return;

	ContactStatus before_status = contact->status();

	contact->setStatus(status);
	emit sigContactDataChanged(id, QVariant::fromValue<ContactStatus>(status), TDR_Status);

	if ( contact->clientType() != type )
	{
		contact->setClientType(type);
		emit sigContactDataChanged(id, type, TDR_ClientType);
	}

	Category *cat = contact->category();
	if ( (before_status == CS_Offline || before_status == CS_Hidden)
			&& (status != CS_Offline && status != CS_Hidden) )
	{
		cat->setOnlineCount(cat->onlineCount() + 1);
		emit sigCategoryDataChanged(cat->index(), cat->onlineCount(), TDR_CategoryOnlineCount);
	}
	else if ( (before_status != CS_Offline && before_status != CS_Hidden)
			&& (status == CS_Offline || status == CS_Hidden)  )
	{
		cat->setOnlineCount(cat->onlineCount() - 1);
		emit sigCategoryDataChanged(cat->index(), cat->onlineCount(), TDR_CategoryOnlineCount);
	}
}


Talkable *Roster::talkable(const QString  &id) const
{
	if ( contacts_.contains(id) )
		return contacts_.value(id);

	if ( groups_.contains(id) )
		return groups_.value(id);

	return NULL;
}


Contact *Roster::contact(QString id) const
{
	if ( contacts_.contains(id) )	
		return contacts_.value(id);
	return NULL;
}


Group *Roster::group(QString id) const
{
	if ( groups_.contains(id) )	
		return groups_.value(id);
	return NULL;
}


Category *Roster::category(int cat_idx) const
{
	foreach (Category *cat, categorys_)
	{
		if (cat->index() == cat_idx )
			return cat;
	}
	return NULL;
}


void Roster::clean()
{
	foreach ( Contact *contact, contacts_.values() )
	{
		delete contact;
		contact = NULL;
	}
	contacts_.clear();


	foreach ( Group *group, groups_.values() )
	{
		delete group;
		group = NULL;
	}
	groups_.clear();


	foreach ( Category *cat, categorys_ )
	{
		delete cat;
		cat = NULL;
	}
	categorys_.clear();
}
