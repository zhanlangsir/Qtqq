#ifndef QTQQ_CORE_CHATMANAGER_H
#define QTQQ_CORE_CHATMANAGER_H

#include <QObject>
#include <QVector>

class QQChatDlg;
class MainWindow;
class NameConvertor;

class ChatManager : public QObject
{
    Q_OBJECT
public:
    ChatManager(MainWindow *main_win, const NameConvertor *convertor);
    ~ChatManager();

    bool isOpening(const QString &id) const;
    QQChatDlg* findChatDlg(QString id) const;
    QString getFriendAvatarPath(const QString &id) const;
    QString getGroupAvatarPath(const QString &id) const;

public slots:
    void openFriendChatDlg(QString id);
    void openGroupChatDlg(QString id, QString gcode);
    void openSessChatDlg(QString id, QString group_id);

private slots:
    void closeChatDlg(QQChatDlg *listener);

private:
    QVector<QQChatDlg*> opening_chatdlg_;
    MainWindow *main_win_;
    const NameConvertor *convertor_;
};

#endif // QTQQ_CORE_CHATMANAGER_H
