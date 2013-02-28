#include "tabwindow.h"
#include "ui_tabwindow.h"

#include <assert.h>

#include <QWidget>
#include <QKeyEvent>
#include <QDebug>

#include "chatwidget/qqchatdlg.h"
#include "core/talkable.h"
#include "trayicon/systemtray.h"
#include "skinengine/qqskinengine.h"

TabWindow::TabWindow() :
    ui(new Ui::TabWindow()),
    current_index_(-1)
{
    ui->setupUi(this);
    connect(ui->tab_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(ui->tab_widget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));

    blink_timer_.setInterval(600);
    connect(&blink_timer_, SIGNAL(timeout()), this, SLOT(onBlinkTimeout()));
}

TabWindow::~TabWindow()
{
    if ( ui )
        delete ui;
    ui = NULL;
}

void TabWindow::activatedTab(QString id)
{
    if ( ((QQChatDlg *)widget(current_index_))->id() == id ) 
        return;

    QQChatDlg *page = pages_.value(id, NULL);

    if ( page )
    {
        ui->tab_widget->setCurrentWidget(page);
    }
}

void TabWindow::addTab(QQChatDlg *page, const QString &label)
{
    Talkable *talkable = page->talkable();
    pages_.insert(talkable->id(), page);
    page->installEditorEventFilter(this);

    connect(page, SIGNAL(chatFinish(QQChatDlg * )), this, SLOT(onPageClosting(QQChatDlg *)));
    connect(page->talkable(),  SIGNAL(dataChanged(QVariant, TalkableDataRole)), this, SLOT(onTalkableDataChanged(QVariant, TalkableDataRole)));

    QIcon icon;
    QPixmap pix = talkable->avatar();
    if ( !pix.isNull() )
        icon.addPixmap(pix);
    else
    {
        if ( page->type() == QQChatDlg::kFriend || page->type() == QQChatDlg::kSess )
        {
            icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_friend_avatar")));
        }
        else if ( page->type() == QQChatDlg::kGroup )
        {
            icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_group_avatar")));
        }
    }

    int index = ui->tab_widget->addTab(page, icon, label);
    ui->tab_widget->setCurrentIndex(index);
    current_index_ = index;
}

inline
QWidget *TabWindow::widget(int index) const
{
    return ui->tab_widget->widget(index);
}

inline
int TabWindow::indexOf(QWidget *page) const
{
    return ui->tab_widget->indexOf(page);
}

void TabWindow::closeTab(int index)
{
    QWidget *page = widget(index);
    page->close();
}

void TabWindow::onPageClosting(QQChatDlg *page)
{
    int index = indexOf(page);
    if ( index != -1 )
    {
        ((QQChatDlg *)widget(index))->removeEditorEventFilter(this);
        ui->tab_widget->removeTab(index);
    }
    if ( ui->tab_widget->count() == 0 )		
        hide();
}

void TabWindow::onTalkableDataChanged(QVariant data, TalkableDataRole role)
{
    Talkable *talkable = qobject_cast<Talkable *>(sender());
    QQChatDlg *changed_page = pages_.value(talkable->id());
    assert(changed_page);

    switch ( role )
    {
        case TDR_Avatar:
            {
                QIcon icon;
                QPixmap pix = talkable->avatar();
                icon.addPixmap(pix);	
                ui->tab_widget->setTabIcon(indexOf(changed_page), icon);
            }
            break;
        case TDR_Status:
            break;
        case TDR_ClientType:
            break;
        default:
            break;
    }
}

void TabWindow::onCurrentTabChanged(int index)
{
    QQChatDlg *before = NULL, *after = NULL;
    if ( current_index_+1 <= pageCount() && current_index_ != -1 &&  index != current_index_ )
    {
        before = (QQChatDlg *)widget(current_index_);
        assert(before);
    }

    if ( index != -1 )
    {
        after = (QQChatDlg *)widget(index);
        QIcon icon;
        QPixmap pix = after->talkable()->avatar();
        if ( !pix.isNull() )
            icon.addPixmap(pix);
        setWindowIcon(icon);
        setWindowTitle(after->windowTitle());
        assert(after);
        stopBlink(after);
    }

    current_index_ = index;

    emit activatedPageChanged(before, after);
}

int TabWindow::pageCount() const
{
    return ui->tab_widget->count();
}

void TabWindow::blink(QQChatDlg *dlg)
{
    if ( blinking_dlg_.contains(dlg) )
        return;

    blinking_dlg_.push_back(dlg);
    blink_timer_.start();
}

void TabWindow::onBlinkTimeout()
{
    static QIcon emptyIcon;
    static bool visiable = false;

    if ( emptyIcon.isNull() )
    {
        QPixmap pix(iconSize());
        pix.fill(QColor(0, 0, 0, 0));
        emptyIcon.addPixmap(pix);
    }

    foreach ( QQChatDlg *dlg, blinking_dlg_ )
    {
        int index = indexOf(dlg);
        if ( index != -1 )
        {
            if ( visiable )
            {
                QIcon icon;
                QPixmap pix = dlg->talkable()->avatar();
                icon.addPixmap(pix);
                ui->tab_widget->setTabIcon(index, icon);
            }
            else
                ui->tab_widget->setTabIcon(index, emptyIcon);
        }
    }

    visiable = !visiable;
}

void TabWindow::stopBlink(QQChatDlg *dlg)
{
    blinking_dlg_.removeOne(dlg);
    int index = indexOf(dlg);

    QIcon icon;
    QPixmap pix = dlg->talkable()->avatar();
    if ( !pix.isNull() )
    {
        icon.addPixmap(pix);
        ui->tab_widget->setTabIcon(index, icon);
    }

    if ( blinking_dlg_.isEmpty() )
    {
        blink_timer_.stop();
    }
}


QQChatDlg *TabWindow::currentChatdlg() const
{
    return (QQChatDlg *)widget(current_index_);
}

void TabWindow::keyPressEvent(QKeyEvent * event)
{
    handleKeyPressEvent(event);
}

bool TabWindow::eventFilter(QObject *watched, QEvent *event)
{
    if ( event->type() == QEvent::KeyPress )
    {
        return handleKeyPressEvent((QKeyEvent *)event);
    }

    return false;
}

bool TabWindow::handleKeyPressEvent(QKeyEvent *event)
{
    if ( event->modifiers() & Qt::AltModifier )
    {
        bool ok;
        int tab_idx = event->text().toInt(&ok);
        if ( !ok || tab_idx == 0 )
            return false;

        if ( ui->tab_widget->count() != 0 && tab_idx <= ui->tab_widget->count() )
        {
            ui->tab_widget->setCurrentIndex(tab_idx-1);
            return true;
        }
    }

    return false;
}

void TabWindow::closeEvent(QCloseEvent *event)
{
    for ( int i = ui->tab_widget->count()-1; i >= 0; --i )    
    {
        closeTab(i);
    }
}
