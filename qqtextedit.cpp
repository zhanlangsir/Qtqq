#include "qqtextedit.h"
#include "imgloader.h"

#include <QHelpEvent>
#include <QToolTip>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>

QQTextEdit::QQTextEdit(QWidget *parent) : QTextEdit(parent)
{

}

void QQTextEdit::appendDocument(const QTextDocument *doc)
{
    QTextCursor cursor(this->document());

    QTextBlockFormat format;
    format.setLeftMargin(8);
    format.setTopMargin(5);
    format.setLineHeight(5, QTextBlockFormat::LineDistanceHeight);
    cursor.movePosition(QTextCursor::End);
    cursor.mergeBlockFormat(format);
    cursor.insertHtml(doc->toHtml());
}

void QQTextEdit::insertNameLine(const QString &name, QColor color)
{
    QTextBlockFormat block_format;

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
    QTextBlockFormat block_format;
    block_format.setLeftMargin(8);
    block_format.setTopMargin(5);
    block_format.setLineHeight(5, QTextBlockFormat::LineDistanceHeight);

    QTextCharFormat char_format;
    char_format.setForeground(color);
    char_format.setFont(font);
    char_format.setFontPointSize(size);

    QTextCursor cursor(this->document());
    cursor.movePosition(QTextCursor::End);
    cursor.setBlockFormat(block_format);
    cursor.setBlockCharFormat(char_format);
    cursor.insertText(text);
}

void QQTextEdit::insertQQFace(const QString &face_id)
{
    QTextDocument *doc = document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    QString path = "images/qqface/default/"+ face_id + ".gif";
    QImage img(path);
    QUrl url(face_id);
    doc->addResource(QTextDocument::ImageResource, url, img);
    cursor.insertImage(face_id);

    if(file_ids_.contains(face_id)){ //同一个gif 使用同一个movie
        return;
    }else{
       file_ids_.append(face_id);
    }

   QMovie* movie = new QMovie(this);
   movie->setFileName(path);
   movie->setCacheMode(QMovie::CacheNone);

   id_mov_hash_.insert(face_id, movie);

   //换帧时刷新
   connect(movie, SIGNAL(frameChanged(int)), this, SLOT(animate(int)));
   movie->start();
}

void QQTextEdit::insertImgProxy(const QString &unique_id)
{
    QTextDocument *doc = document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    QImage img("images/loading/loading.gif");   
    QUrl url(unique_id);
    doc->addResource(QTextDocument::ImageResource, url, img);
    cursor.insertImage(unique_id);

    QMovie* movie = new QMovie(this);
    movie->setFileName("images/loading/loading.gif");
    movie->setCacheMode(QMovie::CacheNone);

    id_mov_hash_.insert(unique_id, movie);

    //换帧时刷新
    connect(movie, SIGNAL(frameChanged(int)), this, SLOT(animate(int)));
    movie->start();
}

void QQTextEdit::replaceIdToName(QString id, QString name)
{
    QTextDocument *doc = this->document();
    QTextCursor cursor = doc->find(id, QTextCursor());

    if (cursor.isNull())
        return;

    cursor.insertText(name);
}

void QQTextEdit::setRealImg(const QString &unique_id, const QString &path)
{
    file_ids_.append(unique_id);

    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();

    QImage real_img;
    real_img.loadFromData(data);

    document()->addResource(QTextDocument::ImageResource,   //替换图片为当前帧
                            QUrl(unique_id), real_img);
    this->update();
    QMovie *mov = id_mov_hash_.value(unique_id);
    mov->stop();
    mov->setFileName(path);
    mov->setCacheMode(QMovie::CacheNone);
    connect(mov, SIGNAL(frameChanged(int)), this, SLOT(animate(int)));
    mov->start();
}

void QQTextEdit::addAnimaImg(const QString &unique_id, const QVariant &resource, QMovie *mov)
{
    this->document()->addResource(QTextDocument::ImageResource, QUrl(unique_id), resource);
    disconnect(mov);
    connect(mov, SIGNAL(frameChanged(int)), this, SLOT(animate(int)));
    file_ids_.append(unique_id);
    id_mov_hash_.insert(unique_id, mov);
}

void QQTextEdit::animate(int)
{
    if (QMovie* movie = qobject_cast<QMovie*>(sender()))
    {

       // qDebug()<<"No."<<lstMovie.indexOf(movie)<<a<<"time is"<<QTime::currentTime();
           document()->addResource(QTextDocument::ImageResource,   //替换图片为当前帧
                    id_mov_hash_.key(movie), movie->currentPixmap());

           setLineWrapColumnOrWidth(lineWrapColumnOrWidth()); // ..刷新显示
    }
}
