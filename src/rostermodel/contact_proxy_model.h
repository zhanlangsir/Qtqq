#ifndef CONTACT_PROXY_MODEL_H
#define CONTACT_PROXY_MODEL_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QSortFilterProxyModel>

class QModelIndex;

class FriendSearcher;

class ContactProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	ContactProxyModel(QObject *parent = NULL);
	~ContactProxyModel();

	void setFilter(FriendSearcher *searcher);

protected:
	bool filterAcceptsRow(int row, const QModelIndex &parent) const;

private slots:
	void onSearch(const QString &str);

private:
	virtual bool lessThan(const QModelIndex & left, const QModelIndex & right) const;

private:
	QVector<QString> search_result_;
	FriendSearcher *searcher_;
	bool is_searching_;
};

#endif //CONTACT_PROXY_MODEL_H
