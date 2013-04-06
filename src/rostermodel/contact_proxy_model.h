#ifndef CONTACT_PROXY_MODEL_H
#define CONTACT_PROXY_MODEL_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QSortFilterProxyModel>

class QModelIndex;

class ContactProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	ContactProxyModel(QObject *parent = NULL);
	~ContactProxyModel();

    void setFilter(const QVector<QString> &filter)
    {
        filter_ = filter;
        invalidateFilter();
    }
    void endFilter()
    {
        filter_.clear();
        invalidateFilter();
    }

protected:
    virtual bool filterAcceptsRow(int row, const QModelIndex &parent) const;

private:
	virtual bool lessThan(const QModelIndex & left, const QModelIndex & right) const;

private:
	QVector<QString> filter_;
};

#endif //CONTACT_PROXY_MODEL_H
