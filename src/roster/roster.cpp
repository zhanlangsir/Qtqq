#include "roster.h"

#include <assert.h>

#include <QDebug>

#include "json/json.h"

#include "core/qqutility.h"
#include "event_handle/event_handle.h"
#include "protocol/qq_protocol.h"
#include "roster/group_presister.h"

Roster* Roster::instance_ = NULL;

Roster::Roster()
{
	connect(this, SIGNAL(sigContactDataChanged(QString, QVariant, TalkableDataRole)), this, SIGNAL(sigTalkableDataChanged(QString, QVariant, TalkableDataRole)));
	connect(this, SIGNAL(sigGroupDataChanged(QString, QVariant, TalkableDataRole)), this, SIGNAL(sigTalkableDataChanged(QString, QVariant, TalkableDataRole)));


    EventHandle::instance()->registerObserver(Protocol::ET_OnAvatarUpdate, this);
    EventHandle::instance()->registerObserver(Protocol::ET_OnGroupMemberListUpdate, this);
    EventHandle::instance()->registerObserver(Protocol::ET_OnStrangerAvatarUpdate, this);
}

Roster::~Roster()
{
	clean();

    EventHandle::instance()->removeObserver(Protocol::ET_OnAvatarUpdate, this);
    EventHandle::instance()->removeObserver(Protocol::ET_OnGroupMemberListUpdate, this);
    EventHandle::instance()->removeObserver(Protocol::ET_OnStrangerAvatarUpdate, this);

	instance_ = NULL;
}

void Roster::parseContactList(const QByteArray &array)
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

    if ( contact->status() == CS_Online )
    {
        cat->setOnlineCount(cat->onlineCount() + 1);
		emit sigCategoryDataChanged(cat->index(), cat->onlineCount(), TDR_CategoryOnlineCount);
    }

	Protocol::QQProtocol *proto = Protocol::QQProtocol::instance();
	if ( !proto->isRequesting(contact->id(), JT_Icon) && contact->avatar().isNull() )
	{
		proto->requestIconFor(contact);
	}
}

void Roster::parseGroupList(const QByteArray &array)
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
		QString gcode = QString::number(gnamelist[i]["code"].asLargestInt());

		Group *group = new Group(gid, gcode, name);
		groups_.insert(gid, group);

		emit sigNewGroup(group);

		proto->requestIconFor(group);
	}
}

void Roster::parseGroupMemberList(const QString &gid, const QByteArray &data)
{
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(QString(data).toStdString(), root, false))
        return;

    Group *group = this->group(gid);
    Json::Value mark_names = root["result"]["cards"];
    QHash <QString, QString> uin_marknames;
    for (unsigned int i = 0; i < mark_names.size(); ++i)
    {
        QString uin = QString::number(mark_names[i]["muin"].asLargestInt());
        QString mark_name = QString::fromStdString(mark_names[i]["card"].asString());

        uin_marknames.insert(uin, mark_name);
    }

    Json::Value members = root["result"]["minfo"];
    for (unsigned int i = 0; i < members.size(); ++i)
    {
        QString nick = QString::fromStdString(members[i]["nick"].asString());
        QString uin = QString::number(members[i]["uin"].asLargestInt());

        Contact *contact = Roster::instance()->contact(uin);
        if ( contact )
        {
            contact->addGroup(group);
            contact = contact->clone();
        }
        else
        {
            contact = new Contact(uin, nick, Talkable::kSessStranger);
            contact->addGroup(group);
        }

        contact->setMarkname(uin_marknames.value(uin, ""));
        contact->setStatus(CS_Offline);

        group->addMember(contact); 

        Protocol::QQProtocol *proto = Protocol::QQProtocol::instance();
        if ( contact->avatar().isNull() && !proto->isRequesting(contact->id(), JT_Icon) )
        {
            proto->requestIconFor(contact);
        }
    }

    Json::Value stats = root["result"]["stats"];
    for (unsigned int i = 0; i < stats.size(); ++i)
    {
        ContactClientType client_type = (ContactClientType)stats[i]["client_type"].asInt();

        ContactStatus stat = (ContactStatus)stats[i]["stat"].asInt();
        QString uin = QString::number(stats[i]["uin"].asLargestInt());

        Contact *contact  = group->member(uin);
        contact->setStatus(stat);
        contact->setClientType(client_type);

        group->notifyMemberDataChanged(contact, TDR_Status);
        group->notifyMemberDataChanged(contact, TDR_ClientType);
    }

    Json::Value ginfo = root["result"]["ginfo"];
    QString g_announcement = QString::fromStdString(ginfo["memo"].asString());
    group->setAnnouncement(g_announcement);
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

void Roster::updateTalkableIcon(Talkable *talkable, QByteArray data)
{
    if ( !talkable->avatar().isNull() )
        return;

    if ( talkable->type() == Talkable::kContact || talkable->type() == Talkable::kSessStranger )
    {
        Contact *contact = static_cast<Contact *>(talkable);
        contact->setAvatar(data);

        if ( !contact->groups().isEmpty() )
        {
            foreach ( Group *group, contact->groups() )
            {
                Contact *g_member = group->member(contact->id());
                if ( g_member )
                    g_member->setAvatar(data);

                group->notifyMemberDataChanged(contact, TDR_Avatar);
                GroupPresister::instance()->setModifiedFlag(group->id());
                GroupPresister::instance()->setActivateFlag(group->id());
            }
        }
        emit sigContactDataChanged(contact->id(), contact->avatar(), TDR_Avatar);
    }
    else if ( talkable->type() == Talkable::kGroup )
    {
        Group *group = static_cast<Group *>(talkable);
        group->setAvatar(data);
        emit sigGroupDataChanged(group->id(), group->avatar(), TDR_Avatar);
    }
}

void Roster::onNotify(Protocol::Event *e)
{
    switch ( e->type() )
    {
        case Protocol::ET_OnAvatarUpdate:
            updateTalkableIcon(e->eventFor(), e->data());
            break;
        case Protocol::ET_OnStrangerAvatarUpdate:
            updateTalkableIcon(e->eventFor(), e->data());
            break;
        case Protocol::ET_OnGroupMemberListUpdate:
            {
                Talkable *group = e->eventFor();
                parseGroupMemberList(group->id(), e->data());
            }
            break;
    }
}

void Roster::slotIconRequestDone(QString id, QByteArray icon_data)
{
	Talkable *talkable = this->talkable(id);	
	talkable->setAvatar(icon_data);

	if ( talkable->type() == Talkable::kContact )
		emit sigContactDataChanged(talkable->id(), talkable->avatar(), TDR_Avatar);
	else if ( talkable->type() == Talkable::kGroup )
		emit sigGroupDataChanged(talkable->id(), talkable->avatar(), TDR_Avatar);
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
