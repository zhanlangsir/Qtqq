#pragma once

#include <QLabel>

class QQNameLabel : public QLabel
{
    Q_OBJECT

public:
    QQNameLabel(QString text);

protected:
    void mouseMoveEvent(QMouseEvent *ev);
};
