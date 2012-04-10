#include "groupitemmodel.h"

#include "include/json/json.h"
#include "nameconvertor.h"

void GroupItemModel::parse(const QByteArray &array, NameConvertor *convertor)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(array).toStdString(), root, false))
    {
        return;
    }

    const Json::Value result = root["result"];
    const Json::Value gnamelist = result["gnamelist"];

    for (unsigned int i = 0; i < gnamelist.size(); ++i)
    {
        QString name = QString::fromStdString(gnamelist[i]["name"].asString());
        QString gid = QString::number(gnamelist[i]["gid"].asLargestInt());
        QString code = QString::number(gnamelist[i]["code"].asLargestInt());

        GroupInfo *info= new GroupInfo;
        info->set_name(name);
        info->set_id(gid);
        info->set_gCode(code);

        QQItem *group = new QQItem(QQItem::kGroup, info, root_);

        id_item_hash_[gid] = group;
        convertor->addUinNameMap(gid, name);;

        root_->append(group);
    }
}
