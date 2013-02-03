#ifndef IMGSENDER_H
#define IMGSENDER_H

#include <assert.h>

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>
#include <QHttp>
#include <QUuid>

#include "core/types.h"

namespace Protocol
{
    class ImgSender;
};

class Talkable;

class Protocol::ImgSender : public QObject
{
    Q_OBJECT
public:
    ImgSender();

    QByteArray prepareSendingData(Talkable *talkable, QString file_name, QByteArray data);

    QByteArray createMsgData(const QString &file_path, const QByteArray &file_data, const QString &boundary);
    QByteArray createGroupMsgData(const QString &file_path, const QByteArray &file_data, const QString &boundary);

    bool parseMsgResult(const QByteArray &array, QString &img_id);
    bool parseGroupMsgResult(const QByteArray &array, QString &img_id);

    QString boundary() const
    { return boundary_; }

    FileInfo getUploadedImgInfo(QString img_id)
    {
        assert(sended_imgs_.contains(img_id));
        return sended_imgs_.value(img_id);
    }

    QString key() const
    {
        return key_;
    }

    QString sig() const
    {
        return sig_;
    }

private slots:
    void getKeyAndSigDone(bool err);

private:
    QString getUniqueId()
    {
        return QUuid::createUuid().toString();
    }

    void getKeyAndSig();

private:
    QString key_;
    QString sig_;

    QString boundary_;
    QMap<QString, FileInfo> sended_imgs_;

    QHttp http_;
};

#endif //IMGSENDER_H
