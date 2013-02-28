#ifndef FILESENDER_H
#define FILESENDER_H

#include <assert.h>

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>
#include <QUuid>

namespace Protocol
{
    class FileSender;
};

class Talkable;

class Protocol::FileSender : public QObject
{
    Q_OBJECT
public:
    FileSender();

    QByteArray createFileData(const QString &file_path, const QByteArray &file_data);
    QByteArray createOffFileData(const QString &file_path, const QString &to_id, const QByteArray &file_data);

    QByteArray boundary() const
    { return boundary_; }

private:
    QByteArray boundary_;
};

#endif //FILESENDER_H
