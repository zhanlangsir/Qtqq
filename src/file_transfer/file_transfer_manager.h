#ifndef FILE_TRANSFER_MANAGER_H
#define FILE_TRANSFER_MANAGER_H

#include <QMap>
#include <QObject>

#include "core/qqmsg.h"

#include "file_transfer/file_transfer_dlg.h"

class FileAskDlg;

class FileTransferManager : public QObject
{
    Q_OBJECT
public:
    static FileTransferManager *instance()
    {
        if ( !instance_ )
            instance_ = new FileTransferManager();

        return instance_;
    }

    void sendFile(const QString &to_id, const QString &to_name, const QString &file_path);
    void sendOffFile(const QString &to_id, const QString &to_name, const QString &file_path);

private slots:
    void onFileMsg(ShareQQMsgPtr msg);
    void onFilesrvTransferMsg(ShareQQMsgPtr msg);
    void onFileRecAccept();
    void onFileRecReject();
    void openFileTransferDlg();

private:
    FileTransferDlg transfer_dlg_;
    QMap<FileAskDlg *, ShareQQMsgPtr> files_;
    QMap<int, FileAskDlg *> dlgs_;

private:
    FileTransferManager();
    FileTransferManager(const FileTransferManager &);
    FileTransferManager &operator=(const FileTransferManager &);

    static FileTransferManager *instance_;
};

#endif //FILE_TRANSFER_MANAGER_H
