#include "qqtextedit.h"

#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMimeData>
#include <QDebug>

#include "qqglobal.h"

QQTextEdit::QQTextEdit(QWidget *parent) : QTextEdit(parent)
{
    setAcceptDrops(true);
}

void QQTextEdit::insertImg(const QString &url, const QString &path)
{
    QTextDocument *doc = document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    QImage img(path);
    QUrl qurl(url);
    doc->addResource(QTextDocument::ImageResource, qurl, img);
    cursor.insertImage(url);
}

void QQTextEdit::insertNameLine(const QString &name, QColor color)
{
    QTextBlockFormat block_format;
    block_format.setTopMargin(5);

    QTextCharFormat char_format;
    char_format.setForeground(QBrush(color));

    QTextCursor cursor(this->document());

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(block_format, char_format);

    cursor.insertText(name);

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
}

void QQTextEdit::insertWord(const QString &text, QFont font, QColor color, int size)
{
    QTextCursor cursor(this->document());
    cursor.movePosition(QTextCursor::End);

    QTextBlockFormat block_format;
    block_format.setLeftMargin(8);
    block_format.setTopMargin(5);

#if QT_VERSION >= 0x040800
	block_format.setLineHeight(5, QTextBlockFormat::LineDistanceHeight);
#endif

    QTextCharFormat char_format;
    char_format.setForeground(color);
    char_format.setFont(font);
    char_format.setFontPointSize(size);

    cursor.setBlockFormat(block_format);
    cursor.setBlockCharFormat(char_format);

    cursor.insertText(text);
}

void QQTextEdit::insertQQFace(const QString &face_id)
{
    QTextDocument *doc = document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    QString path = QQGlobal::resourceDir() + "/qqface/default/" + face_id + ".gif";
    QImage img(path);
    QString src = kQQFacePre+face_id;
    QUrl url(src);
    doc->addResource(QTextDocument::ImageResource, url, img);
    cursor.insertImage(src);
}

void QQTextEdit::replaceIdToName(QString id, QString name)
{
    QTextDocument *doc = this->document();
    QTextCursor cursor = doc->find(id, QTextCursor());

    if (cursor.isNull())
        return;

    cursor.insertText(name);
}

void QQTextEdit::insertFromMimeData( const QMimeData *source )
{
    this->insertHtml(source->text());
}

bool QQTextEdit::canInsertFromMimeData( const QMimeData *source )
{
     qDebug()<<source->html();
     return false;
}

void QQTextEdit::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction();
}

void QQTextEdit::dragMoveEvent(QDragMoveEvent *e)
{
    e->acceptProposedAction();
}

bool QQTextEdit::isQQFace(const QUrl &url) const
{
    if ( url.toString().indexOf("/qqface/default") != -1 )
    {
        return true;
    }
    return false;
}

QString QQTextEdit::getQQFaceId(const QUrl &url) const
{
    QFileInfo qqface(url.toString());
    return qqface.baseName();
}

void QQTextEdit::dropEvent(QDropEvent *e)
{
    if ( e->mimeData()->hasUrls() && e->mimeData()->hasImage() )
    {
        QUrl url = e->mimeData()->urls()[0];
        if ( isQQFace(url) )
        {
            insertQQFace(getQQFaceId(url));
        }
        else
            insertImg(url.path(), url.path());
    }

    e->acceptProposedAction();
}
