#include "qqtextedit.h"
#include "imgloader.h"

#include <QHelpEvent>
#include <QToolTip>

QQTextEdit::QQTextEdit(QWidget *parent) : QTextEdit(parent)
{

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

void QQTextEdit::showProxyFor(const QString &unique_id)
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

void QQTextEdit::insertText(const QString &text)
{
    QTextDocument *doc = document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    cursor.insertText(text);
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
