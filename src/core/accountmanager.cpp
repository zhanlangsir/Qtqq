#include "accountmanager.h"

#include <fstream>

#include <QFile>

#include "json/json.h"
#include "qqglobal.h"

AccountManager::AccountManager() :
    save_path_(QQGlobal::configDir() + "/users.json")
{
    qRegisterMetaType<AccountRecord>("AccountRecord");
}

void AccountManager::readAccounts()
{
    std::ifstream is;
	is.open(QString(QQGlobal::configDir() + "/users.json").toStdString().c_str(), std::ios::in);
    if (!is.is_open())
        return;

    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(is, value, false))
    {
        return;
    }

    is.close();

    Json::Value users = value["users"];
    for (unsigned int i = 0; i < users.size(); ++i)
    {
        AccountRecord *record = new AccountRecord;
        record->id_ =  QString::fromStdString(users[i]["id"].asString());
        record->pwd_ =  QString::fromStdString(users[i]["pwd"].asString());
        record->login_status_  = (ContactStatus)users[i]["login_status"].asInt();
        record->rem_pwd_ =  users[i]["rem_pwd"].asBool();

        login_records_.append(record);
    }
}

void AccountManager::saveAccounts()
{
    Json::FastWriter writer;
    Json::Value login_info;
    Json::Value root;
    Json::Value users;

    foreach(AccountRecord *record, login_records_)
    {
        login_info["id"] = record->id_.toStdString();
        login_info["pwd"] = record->rem_pwd_ ? record->pwd_.toStdString() : "";
        login_info["login_status"] = (int)record->login_status_;
        login_info["rem_pwd"] = record->rem_pwd_;
        users.append(login_info);
    }

    root["users"] = users;

    std::ofstream os;
	QString path = QQGlobal::configDir() + "/users.json";
    os.open(path.toStdString().c_str(), std::ios::out);
    os<<writer.write(root);
    os.close();
}

void AccountManager::setCurrLoginAccount(const AccountRecord &account)
{
    curr_login_id_ = account.id_;

    AccountRecord *same_account = findAccountById(account.id_);
    if ( same_account )
    {
        login_records_.remove(login_records_.indexOf(same_account));
        login_records_.push_front(same_account);

        same_account->login_status_ = account.login_status_;
        same_account->pwd_ = account.pwd_;
        same_account->rem_pwd_ = account.rem_pwd_;
    }
    else
    {
        same_account = new AccountRecord();
        same_account->id_ = account.id_;
        same_account->login_status_ = account.login_status_;
        same_account->pwd_ = account.pwd_;
        same_account->rem_pwd_ = account.rem_pwd_;

        login_records_.push_front(same_account);
    }
}
