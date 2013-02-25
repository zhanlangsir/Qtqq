#include "qqtextedit.h"

#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QMimeData>
#include <QMimeData>
#include <QDebug>

#include "qqglobal.h"

QQTextEdit::QQTextEdit(QWidget *parent) : QTextEdit(parent)
{

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
    QUrl url(kQQFacePre+face_id);
    doc->addResource(QTextDocument::ImageResource, url, img);
    cursor.insertImage(kQQFacePre+face_id);
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
