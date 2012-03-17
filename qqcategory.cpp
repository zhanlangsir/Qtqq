#include "qqcategory.h"

QQCategory::QQCategory(QWidget *parent = 0) :
    QGroupBox(parent)
{
   connect(this, SIGNAL(clicked(bool), this, SLOT())); 
}

/*QQCategory::QQCategory(const QString &title, QWidget *parent = 0) :
    QGroupBox(title, parent)
{
    
}
*/

void toggled()
{
    if (is_expand_)
    {
       for( int i = 0; i < children.size(); ++i)
       {
           children[i]->setVisiable(false);
       }
       is_expand_ = false;
    }
    else
    {
        for( int i = 0; i < children.size(); ++i)
        {
            children[i]->setVisiable(true);
        }
        is_expand_ = true;
    }
}
