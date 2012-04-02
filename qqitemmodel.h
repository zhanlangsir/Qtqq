#ifndef QTQQ_QQITEMMODEL_H
#define QTQQ_QQITEMMODEL_H

#include <QAbstractItemModel>
#include <QString>

#include "qqavatarrequester.h"

class QQItem;

class QQItemModel : public QAbstractItemModel
{
    Q_OBJECT
signals:
    void noAvatar(QQItem *item) const;

public:
    QQItemModel(QObject *parent = 0);
    ~QQItemModel();

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
    QPixmap getDefaultPixmap(const QQItem *item) const;
    QPixmap getPixmap(const QQItem *item) const;
    void setPixmapDecoration(const QQItem *item, QPixmap &pixmap) const;

private slots:
    void requestAvatar(QQItem *item);

private:
    QQAvatarRequester avatar_requester_;
    QQItem *root_;
};

#endif //QTQQ_QQITEMMODEL_H 
