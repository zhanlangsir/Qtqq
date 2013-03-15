#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QWidget>

class QHBoxLayout;
class QStackedWidget;

class TabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);

    int addTab(QWidget *page, const QIcon &tab_icon);
    void setTabIcon(int index, const QIcon &icon);

private slots:
    void onTabBtnClicked();
    
private:
    QHBoxLayout *tabbar_lo;
    QStackedWidget *content_;
};

#endif //TABWIDGET_H
