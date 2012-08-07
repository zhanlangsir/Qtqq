#ifndef FRIENDSEARCHLINEEDIT_H
#define FRIENDSEARCHLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>

#include "friendsearcher.h"

class QModelIndex;
class QStandardItemModel;
class QListView;


class FriendSearchLineEdit : public QLineEdit
{
    Q_OBJECT
signals:
	void friendActivated(const QString&);

public:
    FriendSearchLineEdit(QWidget *parent = 0);
	~FriendSearchLineEdit();

    void setSearcher(FriendSearcher *searcher)
    {
        searcher_ = searcher; 
        connect (searcher_, SIGNAL(findMatchItem(QVector<QQItem*>)), this, SLOT(addFindedFriend(QVector<QQItem*>)));
    }

public slots:
    void complete(const QString &str);
    void onFriendItemClicked(const QModelIndex &index);
    void addFindedFriend(QVector<QQItem*> items);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
	virtual void focusOutEvent(QFocusEvent *e);

private:
	QListView *view_;
    QStandardItemModel *model_;

    FriendSearcher *searcher_;
};

#endif //FRIENDSEARCHLINEEDIT_H
