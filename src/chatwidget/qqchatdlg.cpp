#include "qqchatdlg.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QDateTime>
#include <QMap>
#include <QMenu>
#include <QPointer>
#include <QRegExp>
#include <QDesktopServices>
#include <QShortcut>

#include "chatlogwin.h"
#include "core/curr_login_account.h"
#include "core/qqchatlog.h"
#include "event_handle/event_handle.h"
#include "protocol/qq_protocol.h"
#include "qqfacepanel.h"
#include "setting/setting.h"
#include "skinengine/qqskinengine.h"
#include "skinengine/qqskinengine.h"
#include "soundplayer/soundplayer.h"
#include "utils/htmltomsgparser.h"

QQChatDlg::QQChatDlg(Talkable *talkable, ChatDlgType type, QWidget *parent) :
    QWidget(parent),
    talkable_(talkable),
    qqface_panel_(NULL),
    type_(type),
    sc_close_win_(NULL)
{
    setObjectName("chatWindow");

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

    send_by_return_ = Setting::instance()->value("send_by_return").toBool();

    act_return_->setChecked(send_by_return_);
    act_ctrl_return_->setChecked(!send_by_return_);

    te_input_.installEventFilter(this);

    sc_close_win_ = new QShortcut(QKeySequence("Alt+C"),this);
    connect(sc_close_win_, SIGNAL(activated()), this, SLOT(close()));

    connect(&msgbrowse_, SIGNAL(imageDoubleClicked(QString)), this, SLOT(onImageDoubleClicked(QString)));

    EventHandle::instance()->registerObserver(Protocol::ET_OnImgLoadDone, this);
}

QQChatDlg::~QQChatDlg()
{
    if (qqface_panel_)
        delete qqface_panel_;
    qqface_panel_ = NULL;
}

void QQChatDlg::setSendByReturn(bool checked)
{
    Q_UNUSED(checked)
    if (!send_by_return_)
    {
        Setting::instance()->setValue("send_by_return", true);
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
        Setting::instance()->setValue("send_by_return", false);
        send_by_return_ = false;
        act_return_->setChecked(false);
    }
}

void QQChatDlg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    EventHandle::instance()->removeObserver(Protocol::ET_OnImgLoadDone, this);
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
            if ( key_event->modifiers() & Qt::ControlModifier )
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
        if ( content != "<br />" )
        {
            int img_idx = 0;
            while ( (img_idx = content.indexOf("<img", img_idx)) != -1 ) 
            {
                int src_s_idx = content.indexOf("src=", img_idx);
                //qqface,不需要缩放
                if ( !(content.mid(src_s_idx+5, kQQFacePre.length()) == kQQFacePre) )
                {
                    //添加onload js函数进行缩放和用外部工具打开功能
                    content.insert(img_idx + 4, " onload=\"onImageLoadDone()\" ondblclick=\"imageDoubleClick()\" ");
                }
                ++img_idx;
            }

            converted_html = converted_html + content;
        }

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
    int i = 0;
    while ( raw_html[i] == ' ' )
    {
        ++i;
    }
    QString result = raw_html.mid(i);
    QString space = "";
    while ( i > 0 )
    {
        space += "&nbsp;";
        --i;
    }

    return space + result.replace('<', "&lt").replace('>', "&gt");
}

void QQChatDlg::openPathDialog(bool)
{
    QString file_path = QFileDialog::getOpenFileName(this, tr("select the image to send"), QString(), tr("Image Files(*.png *.jpg *.bmp *.gif)"));

    if (file_path.isEmpty())
        return;

    insertImage(file_path);
}

void QQChatDlg::insertImage(const QString &file_path)
{
    te_input_.insertImg(file_path, file_path);
}

void QQChatDlg::onNotify(Protocol::Event *event)
{
    if ( event->type() == Protocol::ET_OnImgLoadDone )
    {
        Protocol::ImgLoadDoneEvent *img_e = static_cast<Protocol::ImgLoadDoneEvent *>(event);

        switch ( img_e->type() )
        {
            case IMGT_FriendOffpic:
            case IMGT_FriendCface:
                if ( talkable_->id() != img_e->forId() )
                    return;

                break;
            case IMGT_GroupImg:
                if ( talkable_->id() != img_e->forId() )
                    return;

                break;
            default:
                return;
        }

        if ( img_e->data().isEmpty() )
            return;

        QString save_path = saveImage(img_e->file(), img_e->data());
        msgbrowse_.replaceRealImg(img_e->file(), save_path);
    }
}

QString QQChatDlg::saveImage(const QString &file, const QByteArray &data)
{
    QString save_fold_dir = QQGlobal::configDir() + "/chat_img";
    QFileInfo info(file);
    QString file_name = info.baseName();

    QString save_path = save_fold_dir + '/' + file_name + '.' + getImageFormat(data);

    QDir save_dir(save_fold_dir);
    if ( !save_dir.exists() )
        save_dir.mkdir(save_fold_dir);

    if ( !save_dir.exists(save_path) )
    {
        QFile qfile(save_path);
        qfile.open(QIODevice::WriteOnly);
        qfile.write(data);
        qfile.close();
    }

    return save_path;
}

QString QQChatDlg::getImageFormat(const QByteArray &data)
{
    if ( (unsigned char)data[0] == 0xff && (unsigned char)data[1] == 0xd8 )
        return "jpg";
    else if ( (unsigned char)data[0] == 0x89 && (unsigned char)data[1] == 0x50 )
        return "png";
    else if ( (unsigned char)data[0] == 0x47 && (unsigned char)data[1] == 0x49 )
        return "gif";
    else if ( (unsigned char)data[0] == 0x42 && (unsigned char)data[1] == 0x4D )
        return "bmp";
    else if ( (unsigned char)data[0] == 0x00 && (unsigned char)data[2] == 0x01 )
        return "ico";
    else
        qDebug() << "Unknown image format on QQChatDlg::getImageFormat()" << endl;

    return "unknown";
}

void QQChatDlg::send(const QVector<QQChatItem> &msgs)
{
    Protocol::QQProtocol::instance()->sendMsg(talkable_, NULL, msgs);
}

void QQChatDlg::sendMsg()
{
    if (te_input_.document()->isEmpty())
    {
        te_input_.setToolTip(tr("the message can not be empty,please input the message..."));
        te_input_.showToolTip();
        return;
    }

    QString msg = te_input_.toHtml();
    QVector<QQChatItem> chat_items = HtmlToMsgParser::parse(msg, talkable_->type());

    send(chat_items);

    ShowOptions options;
    options.is_msg_in = false;
    QDir avatar_dir(CurrLoginAccount::avatarPath());
    options.sender_avatar_path = avatar_dir.absolutePath();
    options.sender_name = CurrLoginAccount::name();
    options.sender_uin = CurrLoginAccount::id();
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

    QMap<QString, QString> names;
    if ( talkable_->type() == Talkable::kContact || talkable_->type() == Talkable::kStranger )
    {
        names.insert(CurrLoginAccount::id(), CurrLoginAccount::name());
        names.insert(talkable_->id(), talkable_->name());
    }
    else
    {
        foreach ( Contact *contact, ((Group *)talkable_)->members() )
        {
            names.insert(contact->id(), contact->markname());
        }
    }
    QPointer<ChatLogWin> chatlog_win = new ChatLogWin(names);
    chatlog_win->setChatLog(chatlog);
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

void QQChatDlg::showMsg(ShareQQMsgPtr msg)
{
    if ( msg->type() == QQMsg::kOffFile )
    {
        showOtherMsg(msg);
        return;
    }

    const QQChatMsg *chat_msg = static_cast<const QQChatMsg*>(msg.data());

    qint64 time = chat_msg->time();
    QDateTime date_time;
    date_time.setMSecsSinceEpoch(time * 1000);

    ShowOptions options;
    options.is_msg_in = true;
    options.time = date_time;
    options.sender_uin = msg->sendUin();
    options.type = MsgBrowse::kWord;

    Contact *sender = getSender(msg->sendUin());
    if ( sender == NULL || sender->avatar().isNull() )
        options.sender_avatar_path = QQSkinEngine::instance()->skinRes("default_friend_avatar");
    else
        options.sender_avatar_path = sender->avatarPath();

    options.sender_name = sender ? sender->markname() : msg->sendUin();

    QString appending_content;
    for (int i = 0; i < chat_msg->msgs_.size(); ++i)
    {
        switch ( chat_msg->msgs_[i].type() )
        {
            case QQChatItem::kWord:
                {
                    if ( options.type == MsgBrowse::kImg )
                    {
                        msgbrowse_.appendContent(appending_content + "</span>", options);
                        appending_content.clear();
                    }

                    QString escaped_html = escape(chat_msg->msgs_[i].content());
                    if ( i >= 1 && options.type == MsgBrowse::kWord )
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

                    QString face_path = QQGlobal::resourceDir() + "/qqface/default/" + chat_msg->msgs_[i].content() + ".gif";

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

                    QString file = group_msg->msgs_[i].content();
                    Protocol::QQProtocol::instance()->loadGroupImg(
                            talkable_->id(),
                            file, 
                            group_msg->sendUin(), 
                            group_msg->info_seq_,
                            group_msg->msgs_[i].file_id(),
                            group_msg->msgs_[i].server_ip(),
                            group_msg->msgs_[i].server_port(),
                            QString::number(group_msg->time_)
                            );

                    appending_content += "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><img class=\"" + file + "\" src=\""+ QQSkinEngine::instance()->skinRes("loading_img") +"\" ondblclick=\"imageDoubleClick()\" />";
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

                    QString file = chat_msg->msgs_[i].content();
                    Protocol::QQProtocol::instance()->loadFriendCface(
                            file,
                            talkable_->id(), 
                            chat_msg->msg_id_);

                    appending_content += "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><img class=\""+ file + "\" src=\""+  QQSkinEngine::instance()->skinRes("loading_img") + "\" ondblclick=\"imageDoubleClick()\" />";
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

                    QString file = chat_msg->msgs_[i].content();
                    Protocol::QQProtocol::instance()->loadFriendOffpic(
                            file,
                            chat_msg->sendUin()
                            );

                    appending_content += "<span style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><img class=\""+ file + "\" src=\"" + QQSkinEngine::instance()->skinRes("loading_img") + "\" ondblclick=\"imageDoubleClick()\" />";
                    options.type = MsgBrowse::kImg;
                }
                break;
        }

        if (i == (chat_msg->msgs_.size()-1) )
            msgbrowse_.appendContent(appending_content + "</span>", options);
    }

    if ( this->isMinimized() )
    {
        SoundPlayer::singleton()->play(SoundPlayer::kMsg);
        QApplication::alert(this, 1500);
    }
}

void QQChatDlg::showOtherMsg(ShareQQMsgPtr msg)
{
}

void QQChatDlg::onImageDoubleClicked(QString src)
{
    QDesktopServices::openUrl(QUrl::fromPercentEncoding(src.toAscii()));
}

void QQChatDlg::installEditorEventFilter(QObject *filter_obj)
{
    te_input_.installEventFilter(filter_obj);
}

void QQChatDlg::removeEditorEventFilter(QObject *filter_obj)
{
    te_input_.removeEventFilter(filter_obj);
}
