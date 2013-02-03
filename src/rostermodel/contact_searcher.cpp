#include "contact_searcher.h"

#include <QFile>
#include <QDebug>

#include "core/talkable.h"
#include "qqglobal.h"

ContactSearcher::ContactSearcher(QObject *parent) : QObject(parent)
{
}

QStringList ContactSearcher::cn2letter()
{
    static const QStringList cn2letter = readCn2LetterData();
    return cn2letter;
}

QStringList ContactSearcher::readCn2LetterData()
{
    QFile cn2str_fd(QQGlobal::dataDir() + "/misc/words.data"); 

    QStringList cn2letter;

    if ( !cn2str_fd.open(QIODevice::ReadOnly) )
    {
        qDebug() << "Read cn2letter data failed " << endl;
        return QStringList();
    }

  	while ( !cn2str_fd.atEnd() )
    {
        cn2letter.append(cn2str_fd.readLine());
    }

    return cn2letter;
}

void ContactSearcher::initialize(const QVector<Contact *> &contacts)
{
    getSpell(contacts);
}

void ContactSearcher::getSpell(const QVector<Contact *> &contacts)
{
    foreach ( Contact *contact, contacts )
    { 
        appendSpell(contact);
    }
}

QString ContactSearcher::getChineseSpell(QString str) 
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
            foreach(QString strlist, cn2letter()) 
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

void ContactSearcher::search(const QString &str, QVector<QString> &result)
{
    foreach ( const ContactSpell &spell, contact_spells_ ) 
    {
        if ( spell.spell.contains(str.toUpper()) || spell.name.toUpper().contains(str.toUpper()) )
            result.append(spell.id);
    }
}

void ContactSearcher::appendSpell(const Contact *contact)
{
    ContactSpell spell = createSpell(contact);

    if ( contact_spells_.indexOf(spell) == -1 )
        contact_spells_.append(spell);
}

void ContactSearcher::removeSpell(const Contact *contact)
{
    ContactSpell spell = createSpell(contact);

    int index = contact_spells_.indexOf(spell);
    if ( index != -1 )
        contact_spells_.remove(index);
}

ContactSpell ContactSearcher::createSpell(const Contact *contact)
{
    ContactSpell spell;
    spell.id = contact->id();
    spell.name = contact->markname();
    spell.spell = getChineseSpell(contact->markname());

    return spell;
}
