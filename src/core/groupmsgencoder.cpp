#include "groupmsgencoder.h"

#include <QRegExp>

#include "core/captchainfo.h"
#include "../qqchatdlg.h"
#include "../groupchatdlg.h"

GroupMsgEncoder::GroupMsgEncoder(QQChatDlg *dlg) :
    MsgEncoder(dlg)
{
}


QString GroupMsgEncoder::encode(const QString &raw_msg)
{
    GroupChatDlg *group_dlg = static_cast<GroupChatDlg*>(dlg_);

    bool has_gface = false;
    QString msg_template;

    //提取<p>....</p>内容
    QRegExp p_reg("(<p.*</p>)");
    p_reg.setMinimal(true);

    int pos = 0;
    while ( (pos = p_reg.indexIn(raw_msg, pos)) != -1 )
    {
        QString content = p_reg.cap(0);
        while (!content.isEmpty())
        {
            if (content[0] == '<')
            {           
                int match_end_idx = content.indexOf('>')+1;
                QString single_chat_item = content.mid(0, match_end_idx);

                int img_idx = single_chat_item.indexOf("src");
                if (img_idx != -1)
                {
                    img_idx += 5;
                    int img_end_idx = content.indexOf("\"", img_idx);
                    QString src = content.mid(img_idx, img_end_idx - img_idx);

                    if (src.contains(kQQFacePre))
                    {
                        msg_template.append("[\\\"face\\\"," + src.mid(kQQFacePre.length()) + "],");
                    }
                    else
                    {
                        has_gface = true;
                        msg_template.append("[\\\"cface\\\",\\\"group\\\",\\\"" + group_dlg->getUploadedFileInfo(src).name + "\\\"],");
                    } 
                }

                content = content.mid(match_end_idx);
            }
            else
            {
                int idx = content.indexOf("<");
                QString word = content.mid(0,idx);
                jsonEncoding(word);
                msg_template.append("\\\"" + word + "\\\",");

                if (idx == -1)
                    content = "";
                else
                    content = content.mid(idx);
            }
        }

        msg_template.append("\\\"\\\\n\\\",");
        pos += p_reg.cap(0).length();
    }

    msg_template = msg_template +
            "[\\\"font\\\",{\\\"name\\\":\\\"%E5%AE%8B%E4%BD%93\\\",\\\"size\\\":\\\"10\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\","
            "\"msg_id\":" + QString::number(msg_id_++) + ",\"clientid\":\"5412354841\","
            "\"psessionid\":\""+ CaptchaInfo::instance()->psessionid() +"\"}"
            "&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();

    if (has_gface)
        msg_template = "r={\"group_uin\":" + group_dlg->id() +",\"group_code\":" + group_dlg->code() + "," + "\"key\":\"" + group_dlg->key()+ "\"," +
            "\"sig\":\"" + group_dlg->sig() + "\", \"content\":\"[" + msg_template;
    else
        msg_template = "r={\"group_uin\":" + group_dlg->id() + ",\"content\":\"[" + msg_template;

    return msg_template;
}
