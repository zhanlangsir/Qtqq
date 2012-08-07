#include "friendsearchlineedit.h"

#include <QStandardItemModel>
#include <QListView>
#include <QModelIndex>
#include <QStandardItem>
#include <QPoint>
#include <QDebug>

#include "core/qqitem.h"

FriendSearchLineEdit::FriendSearchLineEdit(QWidget *parent) :
    QLineEdit(parent),
    searcher_(NULL)
{
	view_ = new QListView(this);
	model_ = new QStandardItemModel();
	view_->setModel(model_);
	view_->setWindowFlags(Qt::ToolTip);
    
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(complete(const QString &)));
	connect(view_, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onFriendItemClicked(const QModelIndex &)));
}

FriendSearchLineEdit::~FriendSearchLineEdit()
{
    if ( searcher_ )
        delete searcher_;
    searcher_ = NULL;
}

void FriendSearchLineEdit::complete(const QString &str)
{
    if ( str.isEmpty() )
        return;

    searcher_->search(str);
}

void FriendSearchLineEdit::addFindedFriend(QVector<QQItem*> items)
{
    if ( items.isEmpty() )
        return;

    model_->clear();
    foreach ( QQItem* item, items )
	{
		QFile avatar_fd(item->avatarPath());
		avatar_fd.open(QIODevice::ReadOnly);

		QPixmap pix;
		pix.loadFromData(avatar_fd.readAll());

		QIcon avatar;
		avatar.addPixmap(pix);

		QStandardItem *view_item = new QStandardItem(avatar, item->markName());

		model_->appendRow(view_item);
    }

	view_->setMinimumWidth(width());
	view_->setMaximumWidth(width());
    QPoint p(0, height());  
    int x = mapToGlobal(p).x();  
    int y = mapToGlobal(p).y() + 1;  
	view_->move(x, y);
	view_->show();
}

void FriendSearchLineEdit::onFriendItemClicked(const QModelIndex &index)
{
	emit friendActivated(index.data(Qt::UserRole).toString());
}

void FriendSearchLineEdit::keyPressEvent(QKeyEvent *e)
{
	if (!view_->isHidden()) {
		int key = e->key();
		int count = view_->model()->rowCount();
		QModelIndex curr_idx = view_->currentIndex();
        if (Qt::Key_Down == key) {  
            // 按向下方向键时，移动光标选中下一个完成列表中的项  
			int row = curr_idx.row() + 1;
            if (row >= count) {  
                row = 0;  
            }  
			QModelIndex index = view_->model()->index(row, 0);
			view_->setCurrentIndex(index);
        } else if (Qt::Key_Up == key) {  
            // 按向下方向键时，移动光标选中上一个完成列表中的项  
			int row = curr_idx.row() - 1;
            if (row < 0) {  
                row = count - 1;  
            }  
			QModelIndex index = view_->model()->index(row, 0);
			view_->setCurrentIndex(index);
        } else if (Qt::Key_Escape == key) {  
            // 按下Esc键时，隐藏完成列表  
			view_->hide();
        } else if (Qt::Key_Enter == key || Qt::Key_Return == key) {  
            // 按下回车键时，使用完成列表中选中的项，并隐藏完成列表  
			if (curr_idx.isValid()) {
				QString text = view_->currentIndex().data().toString();
				QString clicked_uin = view_->currentIndex().data(Qt::UserRole).toString();

				setText(text);
				emit friendActivated(clicked_uin);
            }  
			view_->hide();
        } else {  
            // 其他情况，隐藏完成列表，并使用QLineEdit的键盘按下事件  
			view_->hide();
            QLineEdit::keyPressEvent(e);  
        }  
    } else {  
        QLineEdit::keyPressEvent(e);  
    }  
}

void FriendSearchLineEdit::focusOutEvent(QFocusEvent *e)
{
	if ( view_->isVisible() )
		view_->hide();

	QLineEdit::focusOutEvent(e);
}
