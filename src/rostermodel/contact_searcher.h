#ifndef CONTACT_SEARCHER_H
#define CONTACT_SEARCHER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QStringList>

struct ContactSpell
{
	QString id;
    QString spell;
    QString name;
};

inline bool operator==(const ContactSpell &f, const ContactSpell &s)
{
    return ((f.id == s.id) && (f.spell == s.spell));
}

class Contact;

class ContactSearcher : public QObject
{
public:
    ContactSearcher(QObject *parent = NULL);

	void initialize(const QVector<Contact *> &contacts);

    void search(const QString &str, QVector<QString> &result);
    void appendSpell(const Contact *contact);
    void removeSpell(const Contact *contact);

private:
    QString getChineseSpell(QString str);
    void getSpell(const QVector<Contact *> &contacts);
    ContactSpell createSpell(const Contact *contact);

    static QStringList readCn2LetterData();
    static QStringList cn2letter();

private:
    QVector<ContactSpell> contact_spells_;
};

#endif //CONTACT_SEARCHER_H
