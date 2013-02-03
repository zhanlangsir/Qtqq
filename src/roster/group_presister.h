/*
 * 如果一个群的所有成员在60秒内没有发生过变化,
 * 而且这个群没有被打开来接收消息,那么为了节省
 * 内存,这个群的成员的数据将会被持久化。
 */

#ifndef GROUP_PRESISTER_H
#define GROUP_PRESISTER_H

#include <QTimer>
#include <QList>

class Contact;
class Group;

class GroupPresister : public QObject
{
    Q_OBJECT
public:
    static GroupPresister *instance()
    {
        if ( !instance_ )
            instance_ = new GroupPresister();

        return instance_;
    }

    void setModifiedFlag(const QString &gid);
    void clearModifiedFlag(const QString &gid);
    void setActivateFlag(const QString &gid);
    void clearActivateFlag(const QString &gid);
    void getGroupMember(Group *group);

private slots:
    void onPresistTimeout();

private:
    bool isModified(const QString &gid);
    bool isActivate(const QString &gid);
    void presistGroup(Group *group);
    void createSql();

private:
    QList<QString> modified_;
    QList<QString> activate_;
    QTimer presist_timer_;

private:
    GroupPresister();
    GroupPresister(const GroupPresister &);
    GroupPresister& operator=(const GroupPresister &);

    static GroupPresister *instance_;
};

#endif //GROUP_PRESISTER_H
