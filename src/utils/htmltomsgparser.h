#ifndef HTMLTOMSGPARSER_H
#define HTMLTOMSGPARSER_H

#include <QString>
#include <QRegExp>
#include <QVector>

#include "core/qqchatitem.h"
#include "core/talkable.h"

class HtmlToMsgParser
{
public:
	static QVector<QQChatItem> parse(const QString &raw_msg, Talkable::TalkableType type)
	{
		QVector<QQChatItem> result;

		//提取<p>....</p>内容
		QRegExp p_reg("<p.*>(.*)</p>");
		p_reg.setMinimal(true);

		int pos = 0;
		while ( (pos = p_reg.indexIn(raw_msg, pos)) != -1 )
		{
            if ( !result.empty() )
            {
                if ( result[result.count()-1].type() == QQChatItem::kWord )
                {
                    QQChatItem &pre_word = result[result.count()-1];
                    pre_word.appendContent("\\\\n");
                }
                else
                {
                    QQChatItem word_item(QQChatItem::kWord, "\\\\n");
                    result.push_back(word_item);
                }
            }

			QString content = p_reg.cap(1);
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
						QString img_src = content.mid(img_idx, img_end_idx - img_idx);

						if (img_src.contains(kQQFacePre))
						{
							QQChatItem face_item(QQChatItem::kQQFace, img_src.mid(kQQFacePre.length()));
							result.push_back(face_item);
						}
						else
						{
                            if ( type == Talkable::kContact || type == Talkable::kStranger )
                            {
                                QQChatItem offpic_item(QQChatItem::kFriendOffpic,img_src);
                                result.push_back(offpic_item);
                            }
                            else 
                            {
                                QQChatItem cface_item(QQChatItem::kGroupChatImg, img_src);
                                result.push_back(cface_item);
                            }
						}
					}

					content = content.mid(match_end_idx);
				}
				else
				{
					int idx = content.indexOf("<");
					QString word = content.mid(0,idx);
					jsonEncoding(word);

                    if ( result.empty() || result[result.count()-1].type() != QQChatItem::kWord )
                    {
                        QQChatItem word_item(QQChatItem::kWord, word);
                        result.push_back(word_item);
                    }
                    else 
                    {
                        QQChatItem &pre_word = result[result.count()-1];
                        pre_word.appendContent(word);
                    }

					if (idx == -1)
						content = "";
					else
						content = content.mid(idx);
				}
			}

			pos += p_reg.cap(0).length();
		}

		return result;
	}

	static void jsonEncoding(QString &escasing) 
	{
		escasing.replace(' ', "%20").replace("&lt;", "%3C").replace("&gt;", "%3E").replace("&amp;", "%26").replace('+', "%2B").replace(';', "%3B");
	}
};

#endif //HTMLTOMSGPARSER_H
