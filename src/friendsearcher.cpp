#include "friendsearcher.h"

#include <QFile>
#include <QDebug>

#include "qqglobal.h"
#include "roster/roster.h"
#include "core/talkable.h"

FriendSearcher::FriendSearcher()
{
}

void FriendSearcher::initialize()
{
    if ( readCn2LetterData() )
        getFriendSpell();
}

bool FriendSearcher::readCn2LetterData()
{
    QFile cn2str_fd(QQGlobal::dataDir() + "/misc/words.data"); 

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

void FriendSearcher::getFriendSpell()
{
	const Roster *roster = Roster::instance();

    foreach ( Contact *contact, roster->contacts() )
    { 
		ContactSpell spell;
		spell.id = contact->id();
		spell.spell = getChineseSpell(contact->markname());
        contact_spells_.append(spell);
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

void FriendSearcher::search(const QString &str, QVector<QString> &result)
{
    foreach ( const ContactSpell &spell, contact_spells_ ) 
    {
		Contact *contact = Roster::instance()->contact(spell.id);
        if ( spell.spell.contains(str.toUpper()) || contact->markname().contains(str.toUpper()) )
            result.append(spell.id);
    }
}
