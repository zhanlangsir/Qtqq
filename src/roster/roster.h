#ifndef ROSTER_H
#define ROSTER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <QHash>

#include "core/talkable.h"

class Roster : public QObject
{
	Q_OBJECT
signals:
	void sigNewCategory(const Category *cat);
	void sigNewContact(const Contact *contact);
	void sigNewGroup(const Group *group);
	void sigContactDataChanged(QString id, QVariant data, TalkableDataRole role);
	void sigGroupDataChanged(QString id, QVariant data, TalkableDataRole role);
	void sigCategoryDataChanged(int index, QVariant data, TalkableDataRole role);
	void sigTalkableDataChanged(QString, QVariant data, TalkableDataRole role); 

public:
	~Roster();

	static Roster* instance()
	{
		if ( !instance_ )
			instance_ = new Roster();
		return instance_;
	}

	void parseContact(const QByteArray &array);
	void parseGroup(const QByteArray &array);
	void parseContactStatus(const QByteArray &array);

	void addContact(Contact *contact, Category *cat);

	Talkable *talkable(const QString  &id) const;;
	Contact *contact(QString id) const;
	Group *group(QString id) const;

	Category *category(int cat_idx) const;
	QVector<Category *> &categorys() 
	{ return categorys_; }

	QVector<Contact *> contacts() const
	{
		return contacts_.values().toVector();
	}

	void clean();

private slots:
	void slotIconRequestDone(QString id, QByteArray icon_data);
	void slotContactStatusChanged(QString id, ContactStatus status, ContactClientType type);

private:
	QHash<QString, Contact*> contacts_;
	QHash<QString, Group*> groups_;
	QVector<Category*> categorys_;

private:
	Roster();
   	Roster(const Roster&);
	Roster& operator=(const Roster&);

	static Roster* instance_;
};

#endif //ROSTER_H
