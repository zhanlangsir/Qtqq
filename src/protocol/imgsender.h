#ifndef IMGSENDER_H
#define IMGSENDER_H

#include <assert.h>

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>
#include <QHttp>
#include <QUuid>

namespace Protocol
{
    struct FileInfo
    {
        int size;
        QString name;
        QString network_path;
    };

    class ImgSender;
};

class Talkable;

class Protocol::ImgSender : public QObject
{
    Q_OBJECT
public:
    ImgSender();

    QByteArray createOffpicBody(const QString &file_path, const QByteArray &file_data);
    QByteArray createGroupImgBody(const QString &file_path, const QByteArray &file_data);

    bool parseMsgResult(QString file_path, const QByteArray &array);
    bool parseGroupMsgResult(QString file_path, const QByteArray &array);

    QString boundary() const
    { return boundary_; }

    FileInfo getUploadedImgInfo(const QString &file_path)
    {
        assert(sended_imgs_.contains(file_path));
        return sended_imgs_.value(file_path);
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
