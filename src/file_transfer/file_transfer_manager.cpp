#include "file_transfer_manager.h"

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
    connect(Protocol::QQProtocol::instance(), SIGNAL(fileTransferProgress(int, int, int)), &transfer_dlg_, SLOT(onFileTransferProgress(int, int, int)));
    connect(&transfer_dlg_, SIGNAL(parseItem(int)), this, SLOT(onParseItem(int)));

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

void FileTransferManager::onParseItem(int session_id)
{

    Protocol::QQProtocol::instance()->parseTransferFile(session_id);
}

void FileTransferManager::onFilesrvTransferMsg(ShareQQMsgPtr msg)
{
    const QQFilesrvTransferMsg *srv_msg = static_cast<const QQFilesrvTransferMsg *>(msg.data());
    
    if ( srv_msg->file_status == 52 )
    {
        transfer_dlg_.cancelItem(srv_msg->lc_id);
        transfer_dlg_.show();
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
    else
    {
        if ( dlgs_.contains(file_msg->session_id) )
        {
            FileAskDlg *ask_dlg = dlgs_.value(file_msg->session_id); 
            assert(ask_dlg);

            ask_dlg->notifyRefuseMessage();
        }
    }
}

void FileTransferManager::onFileRecAccept()
{
    FileAskDlg *dlg = qobject_cast<FileAskDlg *>(sender());
    ShareQQMsgPtr msg = files_.value(dlg);
    delete dlg;

    const QQFileMsg *file_msg = static_cast<const QQFileMsg*>(msg.data());

    TransferItem item;
    item.id = file_msg->session_id;
    item.sender_name = Roster::instance()->contact(file_msg->from_id)->name();
    item.file_name = file_msg->name;
    item.begin_time = QDateTime::currentMSecsSinceEpoch();

    files_.remove(dlg);
    dlgs_.remove(file_msg->session_id);

    Protocol::QQProtocol::instance()->reciveFile(file_msg->session_id, file_msg->name, file_msg->from_id);

    transfer_dlg_.appendItem(item);
    transfer_dlg_.show();
}

void FileTransferManager::onFileRecReject()
{
    FileAskDlg *dlg = qobject_cast<FileAskDlg *>(sender());
    delete dlg;
}
