#include "friendchatlog.h"

#include <QDateTime>
#include <QTextEdit>
#include <QTcpSocket>

#include <json/json.h>

#include "request.h"
#include "sockethelper.h"
#include "captchainfo.h"
#include "qqmsg.h"
#include "core/curr_login_account.h"

FriendChatLog::FriendChatLog(QString to_id):
    to_id_(to_id),
    page_count_(0),
    curr_page_(0)
{
}

QVector<ShareQQMsgPtr> FriendChatLog::getLog(int page)
{
    QString url = "/cgi-bin/webqq_chat/?cmd=1&tuin=" + to_id_ + "&vfwebqq="+ CaptchaInfo::instance()->vfwebqq() +"&page="+ QString::number(page_count_ - page + 1) +
            "&row=10&callback=alloy.app.chatLogViewer.rederChatLog&t=" +
            QString::number(QDateTime::currentMSecsSinceEpoch());

    Request req;
    req.create(kGet, url);
    req.addHeaderItem("Host", "web.qq.com");
    req.addHeaderItem("Connection", "keep-alive");
    req.addHeaderItem("Referer", "http://web.qq.com/");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost("web.qq.com", 80);
    fd.write(req.toByteArray());
    QByteArray result;
    socketReceive(&fd, result);

    fd.close();

    int json_part_s_idx = result.indexOf("ChatLog(")+8;
    int json_part_e_idx = result.lastIndexOf("}")+1;
    result = result.mid(json_part_s_idx, json_part_e_idx - json_part_s_idx);

    QVector<ShareQQMsgPtr> chat_logs;
    parse(result, chat_logs);

    curr_page_ = page;
    return chat_logs;
}

int FriendChatLog::totalPage() const
{
    return page_count_;
}

int FriendChatLog::currPage() const
{
    return curr_page_;
}

void FriendChatLog::parse(QByteArray &arr, QVector<ShareQQMsgPtr> &chat_logs)
{
    if (arr.indexOf("chatlogs") == -1)
        return;

    int uin_s_idx = arr.indexOf("tuin") + 5;
    int uin_e_idx = arr.indexOf(',', uin_s_idx);
    QString uin = arr.mid(uin_s_idx, uin_e_idx - uin_s_idx);

    int curr_page_s_idx = arr.indexOf("page")+5;
    int curr_page_e_idx = arr.indexOf(',', curr_page_s_idx);
    curr_page_ = arr.mid(curr_page_s_idx, curr_page_e_idx - curr_page_s_idx).toInt();

    int total_page_s_idx = arr.indexOf("total")+6;
    int total_page_e_idx = arr.indexOf(',', total_page_s_idx);
    page_count_ = arr.mid(total_page_s_idx, total_page_e_idx - total_page_s_idx).toInt();

    int log_s_idx = uin_e_idx;

    while (true)
    {
        QQChatMsg *msg = new QQChatMsg();

        int cmd_s_idx = arr.indexOf("cmd:", log_s_idx)+4;
        int cmd_e_idx = arr.indexOf(',', cmd_s_idx);

        int cmd = arr.mid(cmd_s_idx, cmd_e_idx - cmd_s_idx).toInt();

        msg->from_uin_ = cmd == 17 ? uin : CurrLoginAccount::id();

        int time_s_idx = arr.indexOf("time:", log_s_idx)+5;

        if (time_s_idx == 4)
            break;

        int time_e_idx=  arr.indexOf(",", time_s_idx);
        msg->time_ = arr.mid(time_s_idx, time_e_idx - time_s_idx).toLong();

        int msg_s_idx = arr.indexOf("msg:[", time_e_idx)+5;

        int chat_item_s_idx = msg_s_idx;
        int chat_item_e_idx;

        while (true)
        {
            QQChatItem item;
            if (arr.at(chat_item_s_idx) == '"')
            {
                item.set_type(QQChatItem::kWord);

                int curr_item_s_idx = chat_item_s_idx + 1;
                int word_e_idx = curr_item_s_idx;

                while ((word_e_idx = arr.indexOf('"',word_e_idx)) != -1)
                {
                    if (arr.at(word_e_idx-1) != '\\')
                    {
                        break;
                    }
                    word_e_idx += 1;
                }

                QByteArray content = arr.mid(curr_item_s_idx ,word_e_idx - curr_item_s_idx);
                QByteArray result;
                decodeString(content, result);
                item.set_content(result);

                chat_item_e_idx = word_e_idx;
            }
            else if (arr.at(chat_item_s_idx) == '[')
            {
                int img_type_s_idx = chat_item_s_idx + 2;
                int img_type_e_idx = arr.indexOf('"', img_type_s_idx);

                QString type = arr.mid(img_type_s_idx, img_type_e_idx - img_type_s_idx);

                if (type == "face")
                {
                    item.set_type(QQChatItem::kQQFace);
                    int faceid_s_idx = img_type_e_idx + 2;
                    int faceid_e_idx = arr.indexOf("]", faceid_s_idx);
                    item.set_content(arr.mid(faceid_s_idx, faceid_e_idx-faceid_s_idx));

                    chat_item_e_idx = faceid_e_idx;
                }
                else if (type == "offpic")
                {
                    item.set_type(QQChatItem::kFriendOffpic);
                    if (arr.at(img_type_e_idx + 3) != '{')
                    {
                        item.set_content("");
                        chat_item_e_idx = arr.indexOf(']', img_type_e_idx);
                    }
                    else
                    {
                        int file_path_s_idx = arr.indexOf("file_path", img_type_e_idx) + 13;
                        int file_path_e_idx = arr.indexOf('"', file_path_s_idx);

                        item.set_content(arr.mid(file_path_s_idx, file_path_e_idx - file_path_s_idx));

                        chat_item_e_idx = file_path_e_idx + 2;
                    }
                }
                else if (type == "cface")
                {
                    item.set_type(QQChatItem::kFriendCface);
                    int cface_s_idx = img_type_e_idx + 3;
                    int cface_e_idx = arr.indexOf('"', cface_s_idx);

                    item.set_content(arr.mid(cface_s_idx, cface_e_idx - cface_s_idx));

                    chat_item_e_idx = arr.indexOf("]", cface_e_idx);
                }
            }

            msg->msgs_.append(item);
            if (arr.at(chat_item_e_idx + 1) != ',')
            {
                log_s_idx = chat_item_e_idx + 3;
                break;
            }

            chat_item_s_idx = chat_item_e_idx + 2;
        }
        ShareQQMsgPtr share_msg_ptr(msg);
        chat_logs.append(share_msg_ptr);
    }
}

void FriendChatLog::decodeString(const QByteArray &arr, QByteArray &decoded)
{
    int loc = 0;

    while (loc != arr.length())
    {
        char c = arr[loc++];
        if (c == '\\')
        {
            switch ( arr[loc++] )
            {
            case '"': decoded += '"'; break;
            case '/': decoded += '/'; break;
            case '\\': decoded += '\\'; break;
            case 'b': decoded += '\b'; break;
            case 'f': decoded += '\f'; break;
            case 'n': decoded += '\n'; break;
            case 'r': decoded += '\r'; break;
            case 't': decoded += '\t'; break;
            case 'u':
               {
                   unsigned int unicode;
                   if ( !decodeUnicodeCodePoint( arr, loc, unicode ) )
                       return;
                   decoded += codePoint2Utf8(unicode);
               }
               break;
            default:
                return;
            }
        }
        else
            decoded += c;
    }
}

bool FriendChatLog::decodeUnicodeCodePoint( const QByteArray &arr, int &loc, unsigned int &unicode )
{
    if ( !decodeUnicodeEscapeSequence( arr, loc, unicode ) )
        return false;

    if ( 0xD800 <= unicode && unicode <= 0xD8FF)
    {
        unsigned int surrogate_pair;
        if ( arr[loc++] == '\\' && arr[loc++] == 'u' )
        {
            if ( decodeUnicodeEscapeSequence( arr, loc, surrogate_pair ) )
                unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogate_pair & 0x3FF);
            else
                return false;
        }
        else
            return false;
    }
    return true;
}

bool FriendChatLog::decodeUnicodeEscapeSequence( const QByteArray &arr, int &loc, unsigned int &unicode )
{
    unicode = 0;
    for (int i = 0; i < 4; ++i)
    {
        char c = arr[loc++];
        unicode *= 16;
        if ( '0' <= c && c <= '9' )
            unicode += (c - '0');
        else if ( 'a' <= c && c <= 'f' )
            unicode += (c - 'a' + 10);
        else if ( 'A' <= c && c <= 'F' )
            unicode += (c - 'A' + 10);
        else
            return false;
    }
    return true;
}

QByteArray FriendChatLog::codePoint2Utf8(unsigned int cp)
{
    QByteArray result;

    // based on description from http://en.wikipedia.org/wiki/UTF-8
    if (cp <= 0x7f)
    {
        result.resize(1);
        result[0] = static_cast<char>(cp);
    }
    else if (cp <= 0x7FF)
    {
        result.resize(2);
        result[1] = static_cast<char>(0x80 | (0x3f & cp));
        result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
    }
    else if (cp <= 0xFFFF)
    {
        result.resize(3);
        result[2] = static_cast<char>(0x80 | (0x3f & cp));
        result[1] = 0x80 | static_cast<char>((0x3f & (cp >> 6)));
        result[0] = 0xE0 | static_cast<char>((0xf & (cp >> 12)));
    }
    else if (cp <= 0x10FFFF)
    {
        result.resize(4);
        result[3] = static_cast<char>(0x80 | (0x3f & cp));
        result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
    }

    return result;
}
