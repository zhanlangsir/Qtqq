#include "tabwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QStackedWidget>

TabWidget::TabWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *main_lo = new QVBoxLayout();
    main_lo->setContentsMargins(0,0,0,0);
    main_lo->setSpacing(0);
    setLayout(main_lo);

    tabbar_lo = new QHBoxLayout();
    tabbar_lo->setContentsMargins(0,0,0,0);
    tabbar_lo->setSpacing(0);
    main_lo->addLayout(tabbar_lo);

    content_ = new QStackedWidget();
    main_lo->addWidget(content_);
}

void TabWidget::onTabBtnClicked()
{
    QWidget *sender_wid = qobject_cast<QWidget *>(sender());
    int index = tabbar_lo->indexOf(sender_wid);
    content_->setCurrentIndex(index);
}

int TabWidget::addTab(QWidget *page, const QIcon &tab_icon)
{
    QPushButton *tab = new QPushButton();
    tab->setIcon(tab_icon);
    connect(tab, SIGNAL(clicked()), this, SLOT(onTabBtnClicked()));
    tabbar_lo->addWidget(tab);
    content_->addWidget(page);

    return tabbar_lo->indexOf(tab);
}

void TabWidget::setTabIcon(int index, const QIcon &icon)
{
}
