#include "groupchatlog.h"

#include <QDateTime>
#include <QTcpSocket>
#include <QUrl>
#include <qmath.h>

#include "include/json.h"

#include "request.h"
#include "sockethelper.h"
#include "captchainfo.h"
#include "qqmsg.h"
#include "qqsetting.h"
#include "sockethelper.h"

GroupChatLog::GroupChatLog(QString gid):
    gid_(gid),
    num_per_page_(10),
    curr_page_(1)
{
    init();
}

void GroupChatLog::init()
{
    QString url = "/keycgi/top/chatlogdates?gid=" + gid_ + "&vfwebqq="+ CaptchaInfo::singleton()->vfwebqq() +"&t=" +
            QString::number(QDateTime::currentMSecsSinceEpoch());

    Request req;
    req.create(kGet, url);
    req.addHeaderItem("Host", "cgi.web2.qq.com");
    req.addHeaderItem("Connection", "keep-alive");
    req.addHeaderItem("Content-Type", "utf-8");
    req.addHeaderItem("Referer", "http://cgi.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());

    QTcpSocket fd;
    fd.connectToHost("cgi.web2.qq.com", 80);
    fd.write(req.toByteArray());
    QByteArray result;
    socketReceive(&fd, result);
    fd.close();

    result = result.mid(result.indexOf("\r\n\r\n")+4);

    Json::Reader reader;
    Json::Value root;

    if ( !reader.parse(QString(result).toStdString(), root, false))
        return;

    int begin_seq = root["result"]["info"][0]["begseq"].asInt();
    end_seq_ = root["result"]["info"][root["result"]["info"].size()-1]["endseq"].asInt();

    page_count_ = qCeil((end_seq_ - begin_seq) / (float)num_per_page_);
}

QVector<ShareQQMsgPtr> GroupChatLog::getLog(int page)
{
    int begseq = end_seq_ - page * num_per_page_ +1;
    int endseq = end_seq_ - ((page-1)*num_per_page_);

    QString url = "/keycgi/top/groupchatlog?ps=10&bs="+QString::number(begseq)+"&es="+
            QString::number(endseq)+"&gid="+ gid_ + "&mode=1&vfwebqq="+ CaptchaInfo::singleton()->vfwebqq() +"&t=" +
            QString::number(QDateTime::currentMSecsSinceEpoch());

    Request req;
    req.create(kGet, url);
    req.addHeaderItem("Host", "cgi.web2.qq.com");
    req.addHeaderItem("Connection", "keep-alive");
    req.addHeaderItem("Content-Type", "utf-8");
    req.addHeaderItem("Referer", "http://cgi.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());

    QTcpSocket fd;
    fd.connectToHost("cgi.web2.qq.com", 80);
    fd.write(req.toByteArray());

    QByteArray result;
    socketReceive(&fd, result);
    fd.close();

    result = result.mid(result.indexOf("\r\n\r\n")+4);

    QVector<ShareQQMsgPtr> chat_logs;
    parse(result, chat_logs);

    curr_page_ = page;
    return chat_logs;
}

int GroupChatLog::totalPage() const
{
    return page_count_;
}

int GroupChatLog::currPage() const
{
    return curr_page_;
}

void GroupChatLog::parse(QByteArray &arr, QVector<ShareQQMsgPtr> &chat_logs)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(arr).toStdString(), root, false))
        return;

    if (root["result"]["data"].type() == Json::nullValue)
        return;

    Json::Value content = root["result"]["data"]["cl"][0]["cl"];

    for (unsigned int i = 0; i < content.size(); ++i)
    {
        QQChatMsg *msg = new QQChatMsg();
        msg->from_uin_ = QString::number(content[i]["u"].asLargestInt());
        msg->time_ =content[i]["t"].asLargestInt();

        Json::Value chat_datas = content[i]["il"];
        for (unsigned int j = 0; j < chat_datas.size(); ++j)
        {
            QQChatItem item;
            if (chat_datas[j]["t"].asInt() == 0)  //小写字母l，非数字1，下同
            {
                item.set_type(QQChatItem::kWord);         
                item.set_content(QString::fromStdString(chat_datas[j]["v"].asString()));
            }
            if (chat_datas[j]["t"].asInt() == 2)
            {
                item.set_type(QQChatItem::kGroupChatImg);
                item.set_content(QString::fromStdString(chat_datas[j]["v"].asString()));
                item.set_server_port(QString::number(chat_datas[j]["port"].asInt()));
                item.set_file_id(QString::number(chat_datas[j]["fid"].asLargestInt()));
                item.set_server_ip(QString::number(chat_datas[j]["ip"].asLargestInt()));
            }
            msg->msg_.append(item);
        }
        ShareQQMsgPtr share_msg_ptr(msg);
        chat_logs.append(share_msg_ptr);
    }
}
