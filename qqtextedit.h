#pragma once

#include "types.h"
#include "imgloader.h"
#include "imgsender.h"

#include <QTextEdit>
#include <QList>
#include <QMovie>
#include <QUrl>
#include <QWidget>
#include <QEvent>
#include <QToolTip>

class QTextDocument;

class QQTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    QQTextEdit(QWidget *parent = 0);

public:
    void insertImgProxy(const QString &unique_id);
    void insertNameLine(const QString &text, QColor color);
    void insertWord(const QString &text, QFont font, QColor color, int size);
    void addAnimaImg(const QString& unique_id, const QVariant &resource, QMovie *mov);
    void appendDocument(const QTextDocument *doc);
    void replaceIdToName(QString id, QString name);

    void clearAll()
    {
        file_ids_.clear();
        id_mov_hash_.clear();
        this->clear();
    }

    const QList<QString>& resourceIds() const
    { return file_ids_; }
    QMovie * getMovieById(const QString &id)
    { return id_mov_hash_.value(id); }
    void showToolTip()
    { QToolTip::showText(QCursor::pos(), toolTip()); }
    bool containsImg(const QString &file_id)
    { return file_ids_.contains(file_id); }
    void insertExistImg(const QString &file_id)
    {
        QTextDocument *doc = document();
        QTextCursor cursor(doc);
        cursor.movePosition(QTextCursor::End);
        cursor.insertImage(file_id);
    }

public slots:
    void insertQQFace(const QString &face_id);

private slots:
    void animate(int);
    void setRealImg(const QString &unique_id, const QString &path);

private:
    QList<QString> file_ids_;
    QHash<QString, QMovie*> id_mov_hash_;
};
