#ifndef FRIENDSEARCHER_H
#define FRIENDSEARCHER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <QStringList>


class QQItem;
class MainWindow;

struct ItemSpell
{
    QQItem *item;
    QString spell;
};

class FriendSearcher : public QObject
{
    Q_OBJECT
signals:
    void findMatchItem(QVector<QQItem*> items);

public:
    FriendSearcher(MainWindow *main_win);

public slots:
    void search(const QString &str);

private:
    QString getChineseSpell(QString str);
    bool readCn2LetterData();
    void getFriendSpell(MainWindow *main_win);

private:
    QMap<QString, ItemSpell> item_spells_;
    QStringList cn2letter_;
};

#endif //FRIENDSEARCHER_H
