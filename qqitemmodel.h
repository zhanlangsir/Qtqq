#ifndef QQItemModel_H
#define QQItemModel_H

#include <QAbstractItemModel>
#include <QString>
#include <QVector>

#include "types.h"
#include "qqitem.h"

class QQItemModel : public QAbstractItemModel
{
public:
    QQItemModel(QObject *parent = 0);

public:
    void setRoot(QQItem *root);
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    void parse(const QByteArray &str);

private:
    QQItem* itemFromIndex(const QModelIndex &index) const ;

private:
    QQItem *root_;
};

#endif // QQItemModel_H
