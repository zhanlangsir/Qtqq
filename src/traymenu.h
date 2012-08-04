#ifndef QTQQ_TRAYMENU_H
#define QTQQ_TRAYMENU_H

#include <QObject>

typedef struct _GtkWidget GtkWidget;
class TrayMenuItem;

class TrayMenu : public QObject
{
    Q_OBJECT
public:
    TrayMenu(QObject *parent = 0);

    void appendMenuItem(TrayMenuItem *item);
    void insertMenuItem(TrayMenuItem *item, int idx);

    void popup() const;

private:
    GtkWidget *menu_;
};

#endif // QTQQ_TRAYMENU_H
