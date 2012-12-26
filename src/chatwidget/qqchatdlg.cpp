#include "qqchatdlg.h"

#include <assert.h>

#include <QFileDialog>
#include <QKeyEvent>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QPointer>
#include <QKeySequence>
#include <QShortcut>
#include <QRegExp>

#include "soundplayer/soundplayer.h"
#include "core/imgloader.h"
#include "core/imgsender.h"
#include "core/captchainfo.h"
#include "chatlogwin.h"
#include "core/qqchatlog.h"
#include "core/groupchatlog.h"
#include "core/qqitem.h"
#include "msghandle/htmltomsgparser.h"
#include "core/curr_login_account.h"

QQChatDlg::QQChatDlg(Talkable *talkable, ChatDlgType type, QWidget *parent) :
    QWidget(parent),
	msg_id_(4462000),
	talkable_(talkable),
    img_sender_(NULL),
    img_loader_(NULL),
    qqface_panel_(NULL),
    msg_sender_(NULL),
	type_(type),
    sc_close_win_(NULL)
{
    setObjectName("chatWindow");
    qRegisterMetaType<FileInfo>("FileInfo");

    te_input_.setMinimumHeight(70);

    send_type_menu_ = new QMenu(this);
    act_return_ = new QAction(tr("send by return"), send_type_menu_);   
    act_return_->setCheckable(true);
    act_ctrl_return_ = new QAction(tr("send by ctrl+return"), send_type_menu_);
    act_ctrl_return_->setCheckable(true);

    connect(act_return_, SIGNAL(triggered(bool)), this, SLOT(setSendByReturn(bool)));
    connect(act_ctrl_return_, SIGNAL(triggered(bool)), this, SLOT(setSendByCtrlReturn(bool)));

    send_type_menu_->addAction(act_return_);
    send_type_menu_->addAction(act_ctrl_return_);

    QSettings setting(QQGlobal::configDir() + "/options.ini", QSettings::IniFormat);
    send_by_return_ = setting.value("send_by_return").toBool();

    act_return_->setChecked(send_by_return_);
    act_ctrl_return_->setChecked(!send_by_return_);

    te_input_.installEventFilter(this);

    sc_close_win_ = new QShortcut(QKeySequence("Alt+C"),this);
    connect(sc_close_win_, SIGNAL(activated()), this, SLOT(close()));
}

QQChatDlg::~QQChatDlg()
{
    if (img_sender_)
    {
        img_sender_->quit(); 
        delete img_sender_;
        img_sender_ = NULL;
    }

    if (img_loader_)
    {
        img_loader_->quit();
        delete img_loader_;
        img_loader_ = NULL;
    }

    if (qqface_panel_)
        delete qqface_panel_;
    qqface_panel_ = NULL;

    if (msg_sender_)
    {
        msg_sender_->terminate();
        msg_sender_->wait();
        msg_sender_->deleteLater();
    }
    msg_sender_ = NULL;
}

void QQChatDlg::setSendByReturn(bool checked)
{
    Q_UNUSED(checked)
    if (!send_by_return_)
    {
        QSettings setting(QQGlobal::configDir() + "/options.ini", QSettings::IniFormat);
        setting.setValue("send_by_return", true);
        send_by_return_ = true;

        act_ctrl_return_->setChecked(false);
    }
}

void QQChatDlg::onMsgSendDone(bool ok, QString msg)
{
    Q_UNUSED(msg)
    emit sigMsgSended(talkable_->id());
}

void QQChatDlg::setSendByCtrlReturn(bool checked)
{
    Q_UNUSED(checked)
    if (send_by_return_)
    {
        QSettings setting(QQGlobal::configDir() + "/options.ini", QSettings::IniFormat);
        setting.setValue("send_by_return", false);
        send_by_return_ = false;
        act_return_->setChecked(false);
    }
}

void QQChatDlg::closeEvent(QCloseEvent *)
{
    emit chatFinish(this);
}

bool QQChatDlg::eventFilter(QObject *obj, QEvent *e)
{
    if (obj != &te_input_ || e->type() != QEvent::KeyPress)
        return QWidget::eventFilter(obj, e);

    QKeyEvent *key_event = static_cast<QKeyEvent *>(e);

    switch (key_event->key())
    {
    case Qt::Key_Return:
        if (key_event->modifiers()& Qt::ControlModifier)
        {
            if (send_by_return_)
            {
                te_input_.insertPlainText("\n");
            }
            else
                sendMsg();
            return true;
        }
        else
        {
            if (send_by_return_)
            {
                sendMsg();
                return true;
            }
        }
        break;
    }
    return false;
}

ImgLoader *QQChatDlg::getImgLoader() const
{
    return new ImgLoader();
}

QString QQChatDlg::converToShow(const QString &converting_html)
{
    QRegExp p_reg("<p.*>(.*)</p>");
    p_reg.setMinimal(true);

    QString converted_html;

    int pos = 0;
    pos = p_reg.indexIn(converting_html, pos);
    while ( pos != -1 )
    {
        QString content = p_reg.cap(1);

        QRegExp img_req("<img src=\"[^\"]*\"");
        if ( img_req.indexIn(content) != -1 )
        {
            foreach (QString img_str, img_req.capturedTexts())
            {
                if ( img_str.indexOf(kQQFacePre) == -1 )
                {
                    QRegExp uuid_req("<img src=\"([^\"]*)\"");
                    uuid_req.indexIn(content);
                    content.replace(uuid_req.cap(1), id_file_hash_[uuid_req.cap(1)].local_path);
                }
            }
        }
        converted_html = converted_html + content;
        pos += p_reg.cap(0).length();
        pos = p_reg.indexIn(converting_html, pos);

        if ( pos != -1 )
            converted_html += "<br>";
    }

    QRegExp qqface_reg(kQQFacePre + "([0-9]*)");
    QString face_path = QQGlobal::resourceDir() + "/qqface/default/\\1.gif";
    converted_html.replace(qqface_reg, face_path);
    converted_html = "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">" + converted_html + "</span>";
    return converted_html;
}

QString QQChatDlg::escape(QString raw_html) const
{
    return raw_html.replace('<', "&lt").replace('>', "&gt");
}

/*
QQChatLog *QQChatDlg::getChatlog() const
{
    return NULL;
}
*/

void QQChatDlg::openPathDialog(bool)
{
    QString file_path = QFileDialog::getOpenFileName(this, tr("select the image to send"), QString(), tr("Image Files(*.png *.jpg *.bmp *.gif)"));

    if (file_path.isEmpty())
        return;

    if (!img_sender_)
    {
        img_sender_ = getImgSender();
        connect(img_sender_, SIGNAL(postResult(QString,FileInfo)), this, SLOT(setFileInfo(QString, FileInfo)));
    }

    QString unique_id = getUniqueId();
    img_sender_->send(unique_id, file_path);

    FileInfo info;
    info.local_path = file_path;
    id_file_hash_.insert(unique_id, info);
}

void QQChatDlg::setFileInfo(QString unique_id, FileInfo file_info)
{
    id_file_hash_[unique_id].name = file_info.name;
    id_file_hash_[unique_id].size = file_info.size;
    id_file_hash_[unique_id].network_path = file_info.network_path;

    te_input_.insertImg(unique_id, id_file_hash_[unique_id].local_path);
}

void QQChatDlg::showMsg(ShareQQMsgPtr msg)
{
    const QQChatMsg *chat_msg = static_cast<const QQChatMsg*>(msg.data());

    QString name;
    QString avatar_path;
    bool ok;
    getInfoById(msg->sendUin(), name, avatar_path, ok);

    qint64 time = chat_msg->time();
    QDateTime date_time;
    date_time.setMSecsSinceEpoch(time * 1000);

    ShowOptions options;
    QDir avatar_dir(avatar_path);
    options.send_avatar_path = avatar_dir.absolutePath();
    options.is_msg_in = true;
    options.time = date_time;
    options.send_uin = msg->sendUin();
    options.send_name = name;
    options.type = MsgBrowse::kWord;

    QString appending_content;
    for (int i = 0; i < chat_msg->msg_.size(); ++i)
    {
        switch ( chat_msg->msg_[i].type() )
        {
        case QQChatItem::kWord:
        {
            if (options.type == MsgBrowse::kImg)
            {
                msgbrowse_.appendContent(appending_content + "</span>", options);
                appending_content.clear();
            }

            QString escaped_html = escape(chat_msg->msg_[i].content());
            if (i >= 1 && options.type == MsgBrowse::kWord)
                appending_content += escaped_html;
            else
                appending_content += "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">" + escaped_html;

            options.type = MsgBrowse::kWord;
        }
            break;

        case QQChatItem::kQQFace:
        {
            if (options.type == MsgBrowse::kImg)
            {
                msgbrowse_.appendContent(appending_content + "</span>", options);
                appending_content.clear();
            }

            QString face_path = QQGlobal::resourceDir() + "/qqface/default/" + chat_msg->msg_[i].content() + ".gif";

            if (i >= 1 && options.type == MsgBrowse::kWord)
               appending_content += "<img src=\""+ face_path +"\" />";
            else  
               appending_content += "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><img src=\""+ face_path +"\" />";

            options.type = MsgBrowse::kWord;
        }
            break;

        case QQChatItem::kGroupChatImg:
        {
            if (!appending_content.isEmpty())
            {
                msgbrowse_.appendContent(appending_content + "</span>", options);
                appending_content.clear();
            }

            const QQGroupChatMsg *group_msg = static_cast<const QQGroupChatMsg *>(chat_msg);
            QString url = "http://web.qq.com/cgi-bin/get_group_pic?type=0&gid=" + group_msg->info_seq_ +
                    "&uin=" + group_msg->sendUin() + "&rip=" + group_msg->msg_[i].server_ip() +
                    "&rport=" + group_msg->msg_[i].server_port() + "&fid=" + group_msg->msg_[i].file_id() +
                    "&pic=" + group_msg->msg_[i].content() + "&vfwebqq="+ CaptchaInfo::instance()->vfwebqq() +
                    "&t="+ QString::number(group_msg->time_);

            appending_content += "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><img src=\""+ url +"\" />";
            options.type = MsgBrowse::kImg;
        }
            break;
        case QQChatItem::kFriendCface:
        {
            if (!appending_content.isEmpty())
            {
                msgbrowse_.appendContent(appending_content + "</span>", options);
                appending_content.clear();
            }

            if (!img_loader_)
            {
                img_loader_ = getImgLoader();
                connect(img_loader_, SIGNAL(loadDone(const QString&, const QString&)), &msgbrowse_, SLOT(replaceRealImg(const QString&, const QString&)));
            }

            QString uuid = getUniqueId();
            img_loader_->loadFriendCface(uuid, chat_msg->msg_[i].content(), talkable_->id(), chat_msg->msg_id_);
            appending_content += "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><img id=\""+ uuid + "\" src=\""+  QQGlobal::resourceDir() + "/loading/loading.gif\" />";
            options.type = MsgBrowse::kImg;
        }
            break;
        case QQChatItem::kFriendOffpic:
        {
            if (!appending_content.isEmpty())
            {
                msgbrowse_.appendContent(appending_content + "</span>", options);
                appending_content.clear();
            }

            if (!img_loader_)
            {
                img_loader_ = getImgLoader();
                connect(img_loader_, SIGNAL(loadDone(const QString&, const QString&)), &msgbrowse_, SLOT(replaceRealImg(const QString&, const QString&)));
            }

            QString uuid = getUniqueId();
            appending_content += "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><img id=\""+ uuid + "\" src=\""+  QQGlobal::resourceDir() + "/loading/loading.gif\" />";
            options.type = MsgBrowse::kImg;

            img_loader_->loadFriendOffpic(uuid, chat_msg->msg_[i].content(), talkable_->id());

        }
            break;
        }

        if (i == (chat_msg->msg_.size()-1) )
            msgbrowse_.appendContent(appending_content + "</span>", options);
    }

    if (this->isMinimized())
    {
        SoundPlayer::singleton()->play(SoundPlayer::kMsg);
		QApplication::alert(this, 1500);
    }
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
	QVector<QQChatItem> chat_items = HtmlToMsgParser::parse(msg);
    QString json_msg = chatItemToJson(chat_items);

    Request req;
    req.create(kPost, send_url_);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
    req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    req.addHeaderItem("Content-Length", QString::number(json_msg.length()));
    req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
    req.addRequestContent(json_msg.toAscii());

    if (!msg_sender_)
    {
        msg_sender_ = new MsgSender();
        connect(msg_sender_, SIGNAL(sendDone(bool, QString)), this, SLOT(onMsgSendDone(bool, QString)));
    }
    
    msg_sender_->send(req);

    ShowOptions options;
    options.is_msg_in = false;
    QDir avatar_dir(CurrLoginAccount::avatarPath());
    options.send_avatar_path = avatar_dir.absolutePath();
    options.send_name = CurrLoginAccount::name();
    options.send_uin = CurrLoginAccount::id();
    options.type = MsgBrowse::kWord;
    options.time = QDateTime::currentDateTime();

    QString converted_html = converToShow(te_input_.toHtml());
    msgbrowse_.appendContent(converted_html, options);

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
    if ( new_x < 0 ) new_x = 0;
    int new_y = face_btn_pos.y() - qqface_panel_geometry.height() + 5;
    qqface_panel_->setGeometry(new_x, new_y, qqface_panel_geometry.width(), qqface_panel_geometry.height());
    qqface_panel_->show();
}

void QQChatDlg::openChatLogWin()
{
    QQChatLog *chatlog = getChatlog();

    QPointer<ChatLogWin> chatlog_win = new ChatLogWin();
    chatlog_win->setChatLog(chatlog);
    chatlog_win->setNameConvertor(&convertor_);
    chatlog_win->getFirstPage();
    chatlog_win->show();
}

void QQChatDlg::showOldMsg(QVector<ShareQQMsgPtr> msgs)
{
    ShareQQMsgPtr msg;
    foreach(msg, msgs)
    {
        showMsg(msg);

        if (type() == kGroup)
        {
            unconvert_ids_.append(msg->sendUin());
        }
    }
}

