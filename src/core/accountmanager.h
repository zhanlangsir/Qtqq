#ifndef QTQQ_CORE_ACCOUNTMANAGER_H
#define QTQQ_CORE_ACCOUNTMANAGER_H

#include <QVector>
#include <QString>

#include "utils/contact_status.h"

struct AccountRecord
{
    QString id_;
    QString pwd_;
    ContactStatus login_status_;
    bool rem_pwd_;
};

Q_DECLARE_METATYPE(AccountRecord*)

class AccountManager
{
public:
    AccountManager();
    ~AccountManager()
    {
        foreach (AccountRecord *record, login_records_)
        {
            delete record;
            record = NULL;
        }
    }

public:
    void readAccounts();
    void saveAccounts();

    int count() const
    { return login_records_.count(); }

    QVector<AccountRecord*> accounts()
    { return login_records_; }
    AccountRecord* findAccountById(QString id)
    {
        foreach(AccountRecord *record, login_records_)
        {
            if (record->id_ == id)
                return record;
        }
        return NULL;
    }

    void setCurrLoginAccount(const AccountRecord &account);

private:
    QString save_path_;
    QVector<AccountRecord*> login_records_;
    QString curr_login_id_;
};

#endif // QTQQ_CORE_ACCOUNTMANAGER_H
