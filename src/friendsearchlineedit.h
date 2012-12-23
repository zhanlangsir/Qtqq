/*
 *  remain this class, in order to changed
 *  the look of lineedit
 */

#ifndef FRIENDSEARCHLINEEDIT_H
#define FRIENDSEARCHLINEEDIT_H

#include <QLineEdit>

class QWidget;

class FriendSearchLineEdit : public QLineEdit
{
public:
    FriendSearchLineEdit(QWidget *parent = 0);
	~FriendSearchLineEdit();
};

#endif //FRIENDSEARCHLINEEDIT_H
