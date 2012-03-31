#include "qqchatdlg.h"
#include "imgloader.h"

#include <QFileDialog>
#include <QKeyEvent>
#include <QApplication>

QQChatDlg::QQChatDlg(QString id, QString name, FriendInfo curr_user_info, 
                     QWidget *parent) :
    QDialog(parent),
    id_(id),
    msg_id_(4462000),
    name_(name),
    curr_user_info_(curr_user_info),
    img_loader_(NULL),
    img_sender_(NULL),
    qqface_panel_(NULL),
    msg_sender_(NULL)
{
    qRegisterMetaType<FileInfo>("FileInfo");
    setFontStyle(QFont(), Qt::black, 9);

    te_messages_.setReadOnly(true);
}

QQChatDlg::~QQChatDlg()
{
    if (img_sender_)
    {
        img_sender_->quit(); 
        delete img_sender_;
    }
    img_sender_ = NULL;

    if (img_loader_)
    {
        img_loader_->quit();
        delete img_loader_;
    }
    img_loader_ = NULL;

    if (qqface_panel_)
        delete qqface_panel_;
    qqface_panel_ = NULL;

    if (msg_sender_)
    {
        msg_sender_->terminate();
        msg_sender_->quit();
        msg_sender_->wait();
        delete msg_sender_;
    }
    msg_sender_ = NULL;
}

void QQChatDlg::closeEvent(QCloseEvent *)
{
    emit chatFinish(this);
}

void QQChatDlg::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Return:
        if (e->modifiers()& Qt::ControlModifier)
        {
            sendMsg();
        }
        break;
    case Qt::Key_C:
        if (e->modifiers()& Qt::AltModifier)
        {
            close();
        }
        break;
    }
}

void QQChatDlg::showQQFace(QString face_id)
{
    QImage img("images/qqface/default/"+face_id+".gif");
    QTextDocument *doc = te_messages_.document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);
    cursor.insertImage(img);
}

void QQChatDlg::openPathDialog(bool)
{
    QString file_path = QFileDialog::getOpenFileName(this, tr("select the image to send"), QString(), tr("Image Files(*.png *.jpg *.bmp *.gif)"));

    if (file_path.isEmpty())
        return;

    if (!img_sender_)
    {
        img_sender_ = createImgSender();
        connect(img_sender_, SIGNAL(postResult(QString,FileInfo)), this, SLOT(setFileInfo(QString, FileInfo)));
        connect(img_sender_, SIGNAL(sendDone(const QString &, const QString&)), &te_input_, SLOT(setRealImg(const QString&,const QString&)));   
    }

    QString unique_id = getUniqueId();
    img_sender_->send(unique_id, file_path, curr_user_info_.id());
    te_input_.insertImgProxy(unique_id);
}

void QQChatDlg::setFileInfo(QString unique_id, FileInfo file_info)
{
    id_file_hash_.insert(unique_id, file_info);
}

void QQChatDlg::setFontStyle(QFont font, QColor color, int size)
{
    QTextBlockFormat block_format;
    block_format.setTopMargin(5);
    block_format.setLineHeight(5, QTextBlockFormat::LineDistanceHeight);

    QTextCharFormat char_format;
    char_format.setForeground(color);
    char_format.setFont(font);
    char_format.setFontPointSize(size);

    QTextCursor cursor(te_input_.document());
    cursor.setBlockFormat(block_format);
    cursor.setBlockCharFormat(char_format);
}

void QQChatDlg::showMsg(const QQMsg *msg)
{
    const QQChatMsg *chat_msg = static_cast<const QQChatMsg*>(msg);

    qint64 time = chat_msg->time_;

    QDateTime date_time;
    date_time.setMSecsSinceEpoch(time * 1000);
    QString time_str = date_time.toString("dd ap hh:mm:ss");

    te_messages_.insertNameLine(convertor_.convert(chat_msg->sendUin()) + " " + time_str, Qt::blue);

    for (int i = 0; i < chat_msg->msg_.size(); ++i)
    {
        if (chat_msg->msg_[i].type() == QQChatItem::kQQFace)
        {
            te_messages_.insertQQFace(chat_msg->msg_[i].content());
        }
        else if (chat_msg->msg_[i].type() == QQChatItem::kWord)
            te_messages_.insertWord(chat_msg->msg_[i].content(), QFont(), Qt::black, 9);
        else
        {
            if (!img_loader_)
            {
                img_loader_ = new ImgLoader();
                connect(img_loader_, SIGNAL(loadDone(const QString&, const QString&)), &te_messages_, SLOT(setRealImg(const QString&, const QString&)));
                img_loader_->start();
            }

            if (te_messages_.containsImg(chat_msg->msg_[i].content()))
            {
                te_messages_.insertExistImg(chat_msg->msg_[i].content());
            }
            else
            {
                if (chat_msg->msg_[i].type() == QQChatItem::kGroupChatImg)
                {
                    const QQGroupChatMsg *chat_msg = static_cast<const QQGroupChatMsg*>(msg);
                    img_loader_->loadGroupChatImg(chat_msg->msg_[i].content(), chat_msg->info_seq_, QString::number(chat_msg->time_));
                }
                else if (chat_msg->msg_[i].type() == QQChatItem::kFriendCface)
                    img_loader_->loadFriendCface(chat_msg->msg_[i].content(), id_, chat_msg->msg_id_);
                else
                    img_loader_->loadFriendOffpic(chat_msg->msg_[i].content(), id_, chat_msg->msg_id_);

                te_messages_.insertImgProxy(chat_msg->msg_[i].content());
            }
        }
    }

    if (this->isHidden())
        QApplication::alert(this, 3000);
}

void QQChatDlg::sendMsg()
{
    if (te_input_.document()->isEmpty())
    {
        te_input_.setToolTip(tr("the message can not be empty,please input the message..."));
        te_input_.showToolTip();
        return;
    }

    if (img_sender_ && img_sender_->isSendding())
    {
        te_input_.setToolTip(tr("image uploading,please wait..."));
        te_input_.showToolTip();
        return;
    }

    QString msg = te_input_.toHtml();

    QString json_msg = converToJson(msg);

    Request req;
    req.create(kPost, send_url_);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
    req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    req.addHeaderItem("Content-Length", QString::number(json_msg.length()));
    req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
    req.addRequestContent(json_msg.toAscii());

    if (!msg_sender_)
    {
        msg_sender_ = new QQMsgSender();
    }
    
    msg_sender_->send(req);

    //清除te_input,添加到te_messages中
    QTextDocument *inp_doc = te_input_.document();

    for (int i = 0; i < te_input_.resourceIds().size(); ++i)
    {
        QString file_id = te_input_.resourceIds().at(i);
        te_messages_.addAnimaImg(file_id, inp_doc->resource(QTextDocument::ImageResource, QUrl(file_id)), te_input_.getMovieById(file_id));
    }

    te_messages_.insertNameLine(curr_user_info_.name() + " " + QDateTime::currentDateTime().toString("dd ap hh:mm:ss"), Qt::darkGreen);
    te_messages_.appendDocument(te_input_.document());

    te_input_.clearAll();
    te_input_.setFocus();
}

void QQChatDlg::openQQFacePanel()
{
    if (!qqface_panel_)
    {
        qqface_panel_ = new QQFacePanel();
        connect(qqface_panel_, SIGNAL(qqfaceClicked(QString)), &te_input_, SLOT(insertQQFace(QString)));
    }

    //移动QQ表情面板位置
    QPoint face_btn_pos = QCursor::pos();
    QRect qqface_panel_geometry = qqface_panel_->frameGeometry();
    int new_x = face_btn_pos.x() - qqface_panel_geometry.width() / 2;
    int new_y = face_btn_pos.y() - qqface_panel_geometry.height() + 5;
    qqface_panel_->setGeometry(new_x, new_y, qqface_panel_geometry.width(), qqface_panel_geometry.height());
    qqface_panel_->show();
}

void QQChatDlg::showOldMsg(QVector<QQMsg *> msgs)
{
    QQMsg *msg;
    foreach(msg, msgs)
    {
        showMsg(msg);

        if (type() == kGroup)
        {
            unconvert_ids_.append(msg->sendUin());
        }
    }

    this->show();
}
