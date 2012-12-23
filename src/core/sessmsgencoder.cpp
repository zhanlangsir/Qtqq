#include "sessmsgencoder.h"

#include "chatwidget/qqchatdlg.h"
#include "chatwidget/groupchatdlg.h"
#include "core/captchainfo.h"

SessMsgEncoder::SessMsgEncoder(QQChatDlg *dlg, QString code, QString sig) :
    MsgEncoder(dlg),
    code_(code),
    sig_(sig)
{
}

QString SessMsgEncoder::encode(const QString &raw_msg)
{
    QString msg_template = "r={\"to\":" + dlg_->id() + ",\"group_sig\":\"" + sig_ + "\",\"face\":291,\"content\":\"[";

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
                        msg_template.append("[\\\"offpic\\\",\\\""+ dlg_->getUploadedFileInfo(src).network_path + "\\\",\\\""+
                                            dlg_->getUploadedFileInfo(src).name + "\\\"," + QString::number(dlg_->getUploadedFileInfo(src).size) + "],");
                    }
                }

                content = content.mid(match_end_idx);
            }
            else
            {
                int idx = content.indexOf("<");
                //&符号的html表示为&amp;而在json中为%26,所以要进行转换
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
        "\"msg_id\":" + QString::number(msg_id_++) + ",\"service_type\":0,\"clientid\":\"5412354841\","
        "\"psessionid\":\""+ CaptchaInfo::instance()->psessionid() +"\"}"
        "&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();

    return msg_template;
}
