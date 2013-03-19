#ifndef QTQQ_QQTEXTEDIT
#define QTQQ_QQTEXTEDIT

#include <QTextEdit>
#include <QList>
#include <QMovie>
#include <QUrl>
#include <QToolTip>

class QWidget;
class QTextDocument;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

const QString kQQFacePre="qqface:";

class QQTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    QQTextEdit(QWidget *parent = 0);

public:
    void insertImg(const QString &url, const QString &path);
    void insertNameLine(const QString &text, QColor color);
    void insertWord(const QString &text, QFont font, QColor color, int size);
    void replaceIdToName(QString id, QString name);

    void clearAll()
    {
        file_ids_.clear();
        this->clear();
    }

    const QList<QString>& resourceIds() const
    { return file_ids_; }
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

protected:
    void insertFromMimeData( const QMimeData *source );
    bool canInsertFromMimeData( const QMimeData *source );
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

private:
    bool isQQFace(const QUrl &url) const;
    QString getQQFaceId(const QUrl &url) const;

private:
    QList<QString> file_ids_;
};

#endif //QTQQ_QQTEXTEDIT
