#include "file_transfer_manager.h"

#include <assert.h>

#include <QAction>
#include <QMenu>
#include <QDateTime>

#include "core/talkable.h"
#include "utils/file_ask_dlg.h"
#include "roster/roster.h"
#include "protocol/qq_protocol.h"
#include "msgprocessor/msg_processor.h"
#include "utils/menu.h"
#include "mainwindow.h"
#include "qtqq.h"

FileTransferManager *FileTransferManager::instance_ = NULL;

FileTransferManager::FileTransferManager()
{
    connect(MsgProcessor::instance(), SIGNAL(newFileMsg(ShareQQMsgPtr)), this, SLOT(onFileMsg(ShareQQMsgPtr)));
    connect(MsgProcessor::instance(), SIGNAL(newFilesrvTransferMsg(ShareQQMsgPtr)), this, SLOT(onFilesrvTransferMsg(ShareQQMsgPtr)));
    connect(Protocol::QQProtocol::instance(), SIGNAL(fileTransferProgress(int, int, int)), transfer_dlg_.recvWidget(), SLOT(onFileTransferProgress(int, int, int)));
    connect(Protocol::QQProtocol::instance(), SIGNAL(sendFileProgress(QString, int, int)), transfer_dlg_.sendWidget(), SLOT(onSendFileProgress(QString, int, int)));

    MainWindow *main_win = Qtqq::instance()->mainWindow();

    QAction *act_open_transfer_dlg = new QAction(tr("Open File Transfer Dialog"), main_win->mainMenu());
    act_open_transfer_dlg->setCheckable(false);
    connect(act_open_transfer_dlg, SIGNAL(triggered()), this, SLOT(openFileTransferDlg()));

    main_win->mainMenu()->addPluginAction(act_open_transfer_dlg);
}

void FileTransferManager::openFileTransferDlg()
{
    transfer_dlg_.show();
}

void FileTransferManager::onFilesrvTransferMsg(ShareQQMsgPtr msg)
{
    const QQFilesrvTransferMsg *srv_msg = static_cast<const QQFilesrvTransferMsg *>(msg.data());
    
    //发送文件完毕
    if ( srv_msg->operation == 1 && srv_msg->file_status == 11 )
    {
        transfer_dlg_.setUploadDone(srv_msg->name, srv_msg->lc_id);
    }
    else if ( srv_msg->operation == 1 && srv_msg->file_status == 50 )
    {
        transfer_dlg_.pauseSending(srv_msg->lc_id);
        transfer_dlg_.showSendWidget();
    }
    else if ( srv_msg->file_status == 52 )
    {
        //对方停止发送文件
        transfer_dlg_.pauseRecving(srv_msg->lc_id);
        transfer_dlg_.showRecvWidget();
    }
    else if ( srv_msg->file_status == 0 )
    {
        transfer_dlg_.sendWidget()->setSendDone(srv_msg->lc_id);
    }
}

void FileTransferManager::onFileMsg(ShareQQMsgPtr msg)
{
    const QQFileMsg *file_msg = static_cast<const QQFileMsg*>(msg.data());
    if ( file_msg->mode == QQFileMsg::kRecv )
    {
        QString sender_name = Roster::instance()->contact(file_msg->from_id)->name();
        QString file_name = file_msg->name;

        FileAskDlg *ask_dlg = new FileAskDlg(sender_name, file_name);
        connect(ask_dlg, SIGNAL(accepted()), this, SLOT(onFileRecAccept()));
        connect(ask_dlg, SIGNAL(rejected()), this, SLOT(onFileRecReject()));

        files_.insert(ask_dlg, msg);
        dlgs_.insert(file_msg->session_id, ask_dlg);

        ask_dlg->show();
    }
    else if ( file_msg->mode == QQFileMsg::kSendAck )
    {
        const QQFileMsg *file_msg = static_cast<const QQFileMsg*>(msg.data());
        transfer_dlg_.sendWidget()->onSendAck(file_msg->session_id);
    }
    else  if ( file_msg->mode == QQFileMsg::kRefuse )
    {
        if ( file_msg->cancel_type == 1 && dlgs_.contains(file_msg->session_id) )
        {
            //对方取消发送文件
            FileAskDlg *ask_dlg = dlgs_.value(file_msg->session_id); 
            assert(ask_dlg);

            ask_dlg->notifyRefuseMessage();
        }
        else if ( file_msg->cancel_type == 2 )
        {
            //对方拒绝接收文件
            transfer_dlg_.sendWidget()->setRefuseRecvFile(file_msg->session_id);
        }
    }
}

void FileTransferManager::onFileRecAccept()
{
    FileAskDlg *dlg = qobject_cast<FileAskDlg *>(sender());
    ShareQQMsgPtr msg = files_.value(dlg);

    const QQFileMsg *file_msg = static_cast<const QQFileMsg*>(msg.data());

    RecvFileItem item;
    item.id = file_msg->session_id;
    item.sender_name = Roster::instance()->contact(file_msg->from_id)->name();
    item.file_name = file_msg->name;
    item.begin_time = QDateTime::currentMSecsSinceEpoch();

    files_.remove(dlg);
    dlgs_.remove(file_msg->session_id);

    delete dlg;

    Protocol::QQProtocol::instance()->reciveFile(file_msg->session_id, file_msg->name, file_msg->from_id);

    transfer_dlg_.appendRecvItem(item);
    transfer_dlg_.showRecvWidget();
}

void FileTransferManager::onFileRecReject()
{
    FileAskDlg *dlg = qobject_cast<FileAskDlg *>(sender());
    ShareQQMsgPtr msg = files_.value(dlg);

    const QQFileMsg *file_msg = static_cast<const QQFileMsg*>(msg.data());

    Protocol::QQProtocol::instance()->refuseRecvFile(file_msg->talkTo(), file_msg->session_id);
    delete dlg;
}

void FileTransferManager::sendFile(const QString &to_id, const QString &to_name, const QString &file_path)
{
    QFile file(file_path);
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();

    Protocol::QQProtocol::instance()->sendFile(file_path, to_id, file_data);

    transfer_dlg_.appendSendItem(to_id, to_name, file_path);
    transfer_dlg_.showSendWidget();
}


void FileTransferManager::sendOffFile(const QString &to_id, const QString &to_name, const QString &file_path)
{
    QFile file(file_path);
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();

    Protocol::QQProtocol::instance()->sendOffFile(file_path, to_id, file_data);

    transfer_dlg_.appendSendItem(to_id, to_name, file_path);
    transfer_dlg_.showSendWidget();
}
