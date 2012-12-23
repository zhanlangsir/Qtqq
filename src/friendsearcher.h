#ifndef FRIENDSEARCHER_H
#define FRIENDSEARCHER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <QStringList>

struct ContactSpell
{
	QString id;
    QString spell;
};

class FriendSearcher : public QObject
{
public:
    FriendSearcher();

	void initialize();

    void search(const QString &str, QVector<QString> &result);

private:
    QString getChineseSpell(QString str);
    bool readCn2LetterData();
    void getFriendSpell();

private:
    QVector<ContactSpell> contact_spells_;
    QStringList cn2letter_;
};

#endif //FRIENDSEARCHER_H
