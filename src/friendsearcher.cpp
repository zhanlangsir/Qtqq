#include "friendsearcher.h"

#include <QFile>
#include <QDebug>

#include "core/qqitem.h"
#include "frienditemmodel.h"
#include "qqglobal.h"
#include "mainwindow.h"

FriendSearcher::FriendSearcher(MainWindow *main_win)
{
    if ( readCn2LetterData() )
        getFriendSpell(main_win);
}

bool FriendSearcher::readCn2LetterData()
{
    QFile cn2str_fd(QQGlobal::dataPath() + "/misc/words.data"); 

    if ( !cn2str_fd.open(QIODevice::ReadOnly) )
    {
        qDebug() << "Read cn2letter data failed " << endl;
        return false;
    }

  	while ( !cn2str_fd.atEnd() )
    {
        cn2letter_.append(cn2str_fd.readLine());
    }

    return true;
}

void FriendSearcher::getFriendSpell(MainWindow *main_win)
{
    foreach ( QQItem *item, main_win->friendModel()->items() )
    { 
        if ( item->type() == QQItem::kCategory )
            continue;

        ItemSpell spell;
        spell.item = item;
        spell.spell = getChineseSpell(item->markName());
        item_spells_.insert(item->id(), spell);
    }
}

QString FriendSearcher::getChineseSpell(QString str) 
{ 
    if( str.isEmpty() )
        return str ; 

    QString result;
    foreach( QChar c, str) 
    { 
        // 若是字母则直接输出 
        if( ( c.unicode() >= 'a' && c.unicode() <='z' ) || ( c.unicode() >='A' && c.unicode() <='Z' ) ) 
            result.append( c.toUpper(  ) ); 
        else if( c.unicode() >= 19968 && c.unicode() <= 40869 ) 
        { 
            // 若字符Unicode编码在编码范围则 查汉字列表进行转换输出 
            foreach(QString strlist, cn2letter_) 
            { 
                if( strlist.indexOf(c) > 0 ) 
                { 
                    result.append(strlist[0]); 
                    break; 
                } 
            } 
        } 
    } 
    return result;
}

void FriendSearcher::search(const QString &str)
{
    QVector<QQItem*> result;
    foreach ( ItemSpell spell, item_spells_.values() ) 
    {
        if ( spell.spell.contains(str.toUpper()) || spell.item->markName().contains(str.toUpper()) )
            result.append(spell.item);
    }

    emit findMatchItem(result);
}
