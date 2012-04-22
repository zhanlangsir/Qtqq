#ifndef QTQQ_QQITEMMODEL_H
#define QTQQ_QQITEMMODEL_H

#include <QAbstractItemModel>
#include <QString>
#include <QHash>

#include "qqavatarrequester.h"

class NameConvertor;

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

    virtual void parse(const QByteArray &str, NameConvertor *convertor) { Q_UNUSED(str) Q_UNUSED(convertor)}
    QQItem *find(QString id);

public slots:
    virtual void changeFriendStatus(QString id, FriendStatus status, ClientType client_type) { Q_UNUSED(id) Q_UNUSED(status) Q_UNUSED(client_type)}

private:
    QQItem* itemFromIndex(const QModelIndex &index) const ;
    virtual QPixmap getDefaultPixmap(const QQItem *item) const;
    virtual QPixmap getPixmap(const QQItem *item) const;
    void setPixmapDecoration(const QQItem *item, QPixmap &pixmap) const;

private slots:
    void requestAvatar(QQItem *item);

protected:
    QQAvatarRequester avatar_requester_;
    QQItem *root_;
    QHash<QString, QQItem*> id_item_hash_;
};

#endif //QTQQ_QQITEMMODEL_H 
