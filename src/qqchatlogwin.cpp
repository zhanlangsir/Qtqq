#include "qqchatlogwin.h"
#include "ui_qqchatlogwin.h"

#include <QTextBlockFormat>
#include <QTextCursor>
#include <QUrl>
#include <QDateTime>
#include <QTextCodec>

#include "core/qqchatlog.h"
#include "core/qqmsg.h"
#include "core/qqsetting.h"
#include "core/nameconvertor.h"

QQChatLogWin::QQChatLogWin(QWidget *parent) :
    QQWidget(parent),
    ui_(new Ui::QQChatLogWin),
    chat_log_(NULL)
{
    ui_->setupUi(contentWidget());

    connect(ui_->btn_first, SIGNAL(clicked()), this, SLOT(getFirstPage()));
    connect(ui_->btn_last, SIGNAL(clicked()), this, SLOT(getLastPage()));
    connect(ui_->btn_next, SIGNAL(clicked()), this, SLOT(getNextPage()));
    connect(ui_->btn_prep, SIGNAL(clicked()), this, SLOT(getPrePage()));
}

QQChatLogWin::~QQChatLogWin()
{
    delete ui_;
    if (chat_log_)
    {
        delete chat_log_;
        chat_log_ = NULL;
    }
}

void QQChatLogWin::closeEvent(QCloseEvent *)
{
    this->deleteLater();
}

void QQChatLogWin::getFirstPage()
{
    QVector<ShareQQMsgPtr>chat_logs = chat_log_->getLog(1);
    showChatLog(chat_logs);

    if (chat_log_->totalPage() == 0 || chat_log_->totalPage() == 1)
    {
        ui_->btn_first->setEnabled(false);
        ui_->btn_last->setEnabled(false);
        ui_->btn_next->setEnabled(false);
        ui_->btn_prep->setEnabled(false);
        ui_->le_curr_page->setEnabled(false);
    }
    else
    {
        ui_->btn_first->setEnabled(false);
        ui_->btn_next->setEnabled(false);
        ui_->btn_last->setEnabled(true);
        ui_->btn_prep->setEnabled(true);
    }

    ui_->lbl_page_count->setText("total" + QString::number(chat_log_->totalPage()) + "page");
    ui_->le_curr_page->setText(QString::number(chat_log_->currPage()));
}

void QQChatLogWin::getNextPage()
{
    QVector<ShareQQMsgPtr>chat_logs = chat_log_->getLog(chat_log_->currPage()-1);
    showChatLog(chat_logs);
    ui_->btn_prep->setEnabled(true);
    ui_->btn_last->setEnabled(true);
    ui_->le_curr_page->setText(QString::number(chat_log_->currPage()));
}

void QQChatLogWin::getPrePage()
{
    QVector<ShareQQMsgPtr>chat_logs = chat_log_->getLog(chat_log_->currPage()+1);
    showChatLog(chat_logs);

    ui_->btn_next->setEnabled(true);
    ui_->btn_first->setEnabled(true);
    ui_->le_curr_page->setText(QString::number(chat_log_->currPage()));
}

void QQChatLogWin::getLastPage()
{
    QVector<ShareQQMsgPtr>chat_logs = chat_log_->getLog(chat_log_->totalPage());
    showChatLog(chat_logs);
    ui_->btn_prep->setEnabled(false);
    ui_->btn_last->setEnabled(false);
    ui_->btn_first->setEnabled(true);
    ui_->btn_next->setEnabled(true);
    ui_->le_curr_page->setText(QString::number(chat_log_->currPage()));
}

void QQChatLogWin::insertNameLine(const QString &name, QColor color)
{
    QTextBlockFormat block_format;
    block_format.setTopMargin(5);

    QTextCharFormat char_format;
    char_format.setForeground(QBrush(color));

    QTextCursor cursor(ui_->msgbrowse->document());

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(block_format, char_format);

    cursor.insertText(name);

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
}

void QQChatLogWin::insertQQFace(const QString &face_id)
{
    QTextDocument *doc = ui_->msgbrowse->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    QString path = QQSettings::instance()->resourcePath() + "/qqface/default/" + face_id + ".gif";
    QImage img(path);
    QUrl url(face_id);
    doc->addResource(QTextDocument::ImageResource, url, img);
    cursor.insertImage(face_id);
}

void QQChatLogWin::insertWord(const QString &text, QFont font, QColor color, int size)
{
    QTextCursor cursor(ui_->msgbrowse->document());
    cursor.movePosition(QTextCursor::End);

    QTextBlockFormat block_format;
    block_format.setLeftMargin(8);
    block_format.setTopMargin(5);
    block_format.setLineHeight(5, QTextBlockFormat::LineDistanceHeight);

    QTextCharFormat char_format;
    char_format.setForeground(color);
    char_format.setFont(font);
    char_format.setFontPointSize(size);

    cursor.setBlockFormat(block_format);
    cursor.setBlockCharFormat(char_format);

    cursor.insertText(text);
}

void QQChatLogWin::insertImg(const QString &img_name)
{
    QTextDocument *doc = ui_->msgbrowse->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    QImage img(QQSettings::instance()->tempPath() + "/" + img_name);
    QUrl url(img_name);
    doc->addResource(QTextDocument::ImageResource, url, img);
    cursor.insertImage(img_name);
}

void QQChatLogWin::showChatLog(QVector<ShareQQMsgPtr> &chat_logs)
{
    ui_->msgbrowse->clear();
    ShareQQMsgPtr msg;
    foreach (msg, chat_logs)
    {

        QQGroupChatMsg *chat_msg = static_cast<QQGroupChatMsg*>(msg.data());

        qint64 time = chat_msg->time();

        QDateTime date_time;
        date_time.setMSecsSinceEpoch(time * 1000);
        QString time_str = date_time.toString("dd ap hh:mm:ss");

        insertNameLine(convertor_->convert(chat_msg->sendUin()) + " " + time_str, Qt::blue);

        for (int i = 0; i < chat_msg->msg_.size(); ++i)
        {
            if (chat_msg->msg_[i].type() == QQChatItem::kQQFace)
            {
                insertQQFace(chat_msg->msg_[i].content());
            }
            else if (chat_msg->msg_[i].type() == QQChatItem::kWord)
                insertWord(chat_msg->msg_[i].content(), QFont(), Qt::black, 9);
            else
            {
                insertImg(chat_msg->msg_[i].content());
            }
        }
    }
}

