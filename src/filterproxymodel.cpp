#include "filterproxymodel.h"

#include <QModelIndex>

#include "core/qqitem.h"

FilterProxyModel::FilterProxyModel()
{

}

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    QQItem *info = static_cast<QQItem *>(index.internalPointer());


}

/*
QString getChineseSpell(QString str) 
{ 
    if( str.isEmpty() )
        return str; 

    QString result;

    foreach( QChar c, str ) 
    { 
        // 若是字母则直接输出 
        if( ( v >= 'a' && v <='z' ) || ( v >='A' && v <='Z' ) ) 
            result.append( c.toUpper()); 
        else if( 19968 < (int)c && (int) c <= 40869 ) 
        { 
            // 若字符Unicode编码在编码范围则 查汉字列表进行转换输出 
            foreach( QString strList, strChineseCharList ) 
            { 
                if( strList.IndexOf( vChar) > 0 ) 
                { 
                    myStr.Append( strList[0]); 
                    break; 
                } 
            } 
        } 
    } 

    return myStr.ToString() ; 
}
*/
