#include "group_presister.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "chatwidget/chatdlg_manager.h"
#include "core/talkable.h"
#include "roster/roster.h"
#include "qqglobal.h"

GroupPresister::GroupPresister()
{
    presist_timer_.setInterval(60000);
    connect(&presist_timer_, SIGNAL(timeout()), this, SLOT(onPresistTimeout()));

    presist_timer_.start();
}

GroupPresister *GroupPresister::instance_ = NULL;

void GroupPresister::setModifiedFlag(const QString &gid)
{
    if ( modified_.indexOf(gid) == -1 )
        modified_.append(gid);
}

bool GroupPresister::isModified(const QString &gid)
{
    return !(modified_.indexOf(gid) == -1);
}

void GroupPresister::clearModifiedFlag(const QString &gid)
{
    modified_.removeOne(gid);
}

void GroupPresister::getGroupMember(Group *group)
{
    QStringList connection_names = QSqlDatabase::connectionNames();
    QSqlDatabase db;
    if (connection_names.isEmpty())
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(QQGlobal::configDir() + "/qqgroupdb");
    }
    else
    {
        db = QSqlDatabase::database(connection_names[0]);
    }

    if (!db.open())
        return;

    QSqlQuery query;
    QString read_command = "SELECT * FROM groupmemberinfo WHERE groupmemberinfo.gid == %1";
    query.exec(read_command.arg(group->id()));

    int i = 0;

    while (query.next())
    {
        QString uin = query.value(0).toString();
        QString nick = query.value(2).toString();
        QString mark_name = query.value(3).toString();
        ContactStatus stat = (ContactStatus)query.value(4).toInt();
        QString avatar_path = query.value(5).toString();
        Talkable::TalkableType type = (Talkable::TalkableType)query.value(6).toInt(); 

        Contact *contact = new Contact(uin, nick, type);
        if ( !avatar_path.isEmpty() )
            contact->setAvatarPath(avatar_path);

        contact->setMarkname(mark_name);
        contact->setStatus(stat);

        group->members_.insert(contact->id(), contact);
    }
}

void GroupPresister::presistGroup(Group *group)
{
    qDebug() << "Presisting group, Gid: " << group->id() << ", Group name: " << group->name() << endl;
    {
        QStringList connection_names = QSqlDatabase::connectionNames();
        QSqlDatabase db;
        if ( connection_names.isEmpty() )
        {
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName(QQGlobal::configDir() + "/qqgroupdb");
        }
        else
        {
            db = QSqlDatabase::database(connection_names[0]);
        }

        if ( !db.open() )
            return;

        createSql();

        QSqlQuery query;

        query.exec("SELECT count(*) FROM groupmemberinfo WHERE gid == " + group->id());

        if (!query.first())
            qDebug()<<query.lastError()<<endl;

        int exist_record_count = query.value(0).toInt();

        if ( !exist_record_count )
        {
            QString format = "jpg";
            QString gicon_path = QQGlobal::tempDir() + "/avatar";

            QDir gicon_dir(gicon_path);
            if ( !gicon_dir.exists() )
                gicon_dir.mkdir(gicon_path);

            QString insert_command = "INSERT INTO groupmemberinfo VALUES (%1, %2, '%3', '%4', %5, '%6', %7)";
            QSqlDatabase::database().transaction();
            foreach ( Contact *contact, group->members() )
            {
                //uin, gid, name, mark name, status, avatar path, type
                query.exec(insert_command.arg(contact->id()).arg(group->id()).arg(contact->name().replace('\'', "''")).arg(contact->markname().replace('\'', "''")).arg(contact->status()).arg(contact->avatarPath()).arg(contact->type()));

                if ( query.lastError().isValid() )
                {
                    qDebug() << "Write sql failed:" << contact->name() << "  " <<  query.lastError().text() << endl;
                }
            }
            QSqlDatabase::database().commit();

            group->clearMembers();
        }
    }
    QString name;{
        name = QSqlDatabase::database().connectionName();
        QSqlDatabase::database().close();
    }
    QSqlDatabase::removeDatabase(name);
}

void GroupPresister::onPresistTimeout()
{
    foreach ( QString gid, modified_ )
    {
        /*
         * don't presist the group data which 
         * is opening for chat
         */
        if ( ChatDlgManager::instance()->isOpening(gid) )
            continue;

        if ( isActivate(gid) )
        {
            clearActivateFlag(gid);
            continue;
        }

        if ( isModified(gid) )
        {
            Group *group = Roster::instance()->group(gid);

            if ( group->memberCount() != 0 )
                presistGroup(group);

            clearModifiedFlag(gid);    
        }
    }
}

void GroupPresister::createSql()
{
    QSqlQuery query;

    query.exec("CREATE TABLE IF NOT EXISTS groupmemberinfo ("
            "uin INTEGER,"
            "gid INTERGER,"
            "name VARCHAR(15),"
            "markname VARCHAR(25),"
            "status INTEGER,"
            "avatarpath VARCHAR(20),"
            "type INTEGER,"
            "PRIMARY KEY (uin, gid))");

    if (query.lastError().isValid())
    {
        qDebug() << query.lastError();
    } 
    query.exec("CREATE TABLE IF NOT EXISTS groupinfo ("
            "gid INTERGER,"
            "memo VARCHAR(100),"
            "PRIMARY KEY (gid))");

    if (query.lastError().isValid())
    {
        qDebug() << query.lastError();
    }
}

bool GroupPresister::isActivate(const QString &gid)
{
    return !(activate_.indexOf(gid) == -1);
}

void GroupPresister::setActivateFlag(const QString &gid)
{
    if ( activate_.indexOf(gid) == -1 )
        activate_.append(gid);
}

void GroupPresister::clearActivateFlag(const QString &gid)
{
    activate_.removeOne(gid);
}
