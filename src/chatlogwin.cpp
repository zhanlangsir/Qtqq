#include "chatlogwin.h"
#include "ui_chatlogwin.h"

#include <QDateTime>
#include <QDesktopWidget>
#include <QTextBlockFormat>
#include <QTextCodec>
#include <QTextCursor>
#include <QUrl>

#include "core/qqchatlog.h"
#include "core/qqmsg.h"
#include "qqglobal.h"

ChatLogWin::ChatLogWin(QMap<QString, QString> names, QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::ChatLogWin),
    chat_log_(NULL),
    names_(names)
{
    ui_->setupUi(this);

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);
    ui_->msgbrowse->setReadOnly(true);

    connect(ui_->btn_first, SIGNAL(clicked()), this, SLOT(getFirstPage()));
    connect(ui_->btn_last, SIGNAL(clicked()), this, SLOT(getLastPage()));
    connect(ui_->btn_next, SIGNAL(clicked()), this, SLOT(getNextPage()));
    connect(ui_->btn_prep, SIGNAL(clicked()), this, SLOT(getPrePage()));
}

ChatLogWin::~ChatLogWin()
{
    delete ui_;
    if (chat_log_)
    {
        delete chat_log_;
        chat_log_ = NULL;
    }
}

void ChatLogWin::closeEvent(QCloseEvent *)
{
    this->deleteLater();
}

void ChatLogWin::getFirstPage()
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

    ui_->lbl_page_count->setText("total " + QString::number(chat_log_->totalPage()) + " page");
    ui_->le_curr_page->setText(QString::number(chat_log_->currPage()));
}

void ChatLogWin::getNextPage()
{
    QVector<ShareQQMsgPtr>chat_logs = chat_log_->getLog(chat_log_->currPage()-1);
    showChatLog(chat_logs);

    if (chat_log_->currPage() == 1)
    {
        ui_->btn_first->setEnabled(false);
        ui_->btn_next->setEnabled(false);
    }

    ui_->btn_prep->setEnabled(true);
    ui_->btn_last->setEnabled(true);
    ui_->le_curr_page->setText(QString::number(chat_log_->currPage()));
}

void ChatLogWin::getPrePage()
{
    QVector<ShareQQMsgPtr>chat_logs = chat_log_->getLog(chat_log_->currPage()+1);
    showChatLog(chat_logs);

    if (chat_log_->currPage() == chat_log_->totalPage())
    {
        ui_->btn_last->setEnabled(false);
        ui_->btn_prep->setEnabled(false);
    }

    ui_->btn_next->setEnabled(true);
    ui_->btn_first->setEnabled(true);
    ui_->le_curr_page->setText(QString::number(chat_log_->currPage()));
}

void ChatLogWin::getLastPage()
{
    QVector<ShareQQMsgPtr>chat_logs = chat_log_->getLog(chat_log_->totalPage());
    showChatLog(chat_logs);
    ui_->btn_prep->setEnabled(false);
    ui_->btn_last->setEnabled(false);
    ui_->btn_first->setEnabled(true);
    ui_->btn_next->setEnabled(true);
    ui_->le_curr_page->setText(QString::number(chat_log_->currPage()));
}

void ChatLogWin::insertNameLine(const QString &name, QColor color)
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

void ChatLogWin::insertQQFace(const QString &face_id)
{
    QTextDocument *doc = ui_->msgbrowse->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    QString path = QQGlobal::resourceDir() + "/qqface/default/" + face_id + ".gif";
    QImage img(path);
    QUrl url(face_id);
    doc->addResource(QTextDocument::ImageResource, url, img);
    cursor.insertImage(face_id);
}

void ChatLogWin::insertWord(const QString &text, QFont font, QColor color, int size)
{
    QTextCursor cursor(ui_->msgbrowse->document());
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

void ChatLogWin::insertImg(const QString &img_name)
{
    QTextDocument *doc = ui_->msgbrowse->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

    QImage img(QQGlobal::tempDir() + "/" + img_name);
    QUrl url(img_name);
    doc->addResource(QTextDocument::ImageResource, url, img);
    cursor.insertImage(img_name);
}

void ChatLogWin::showChatLog(QVector<ShareQQMsgPtr> &chat_logs)
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

        QString send_id = chat_msg->sendUin();
        insertNameLine(names_.value(send_id, send_id) + " " + time_str, Qt::blue);

        for (int i = 0; i < chat_msg->msgs_.size(); ++i)
        {
            if (chat_msg->msgs_[i].type() == QQChatItem::kQQFace)
            {
                insertQQFace(chat_msg->msgs_[i].content());
            }
            else if (chat_msg->msgs_[i].type() == QQChatItem::kWord)
                insertWord(chat_msg->msgs_[i].content(), QFont(), Qt::black, 9);
            else
            {
                insertImg(chat_msg->msgs_[i].content());
            }
        }
    }
}
