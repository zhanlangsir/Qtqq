#ifndef QTQQ_TRAYMENUITEM_H
#define QTQQ_TRAYMENUITEM_H

#include <QObject>

typedef struct _GtkWidget GtkWidget;
class TrayMenu;

class TrayMenuItem : public QObject
{
    Q_OBJECT
signals:
    void triggered();

public:
    TrayMenuItem(QString label, QObject *parent = 0);
    void emitTriggered()
    {
        emit triggered();
    }

    friend class TrayMenu;

private:
    GtkWidget *item_;
};

#endif // QTQQ_TRAYMENUITEM_H
