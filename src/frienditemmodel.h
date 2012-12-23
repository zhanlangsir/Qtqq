#ifndef QTQQ_FRIENDITEMMODEL_H
#define QTQQ_FRIENDITEMMODEL_H

#include "qqitemmodel.h"

class NameConvertor;
class Category;

class FriendItemModel : public QQItemModel
{
    Q_OBJECT
public:
    FriendItemModel(QObject *parent = 0) : QQItemModel(parent)
    {
    }

public:
    void parse(const QByteArray &array, NameConvertor *convertor);
    void setMarkName(QString mark_name, QString id);
    void addItem(QString id, QString mark_name, QString groupidx, ContactStatus status);
    QQItem *category(QString idx) const;
    QVector<QQItem*> categorys() const;
    int getNewPosition(const QQItem *item) const;
    
public slots:
    void changeFriendStatus(QString id, ContactStatus status, ContactClientType client_type);

public:
	virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
	virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

private:
	int getOnlineContactCount(Category *cat) const;

private:
    NameConvertor *convertor_;

	QVector<Category*> categorys_;
};

#endif // QTQQ_FRIENDITEMMODEL_H
