#pragma once

#include "qqitembase.h"

class QQCategory : public QQItemBase
{
public:
    QQCategory();

private:
    QVector<QQItemBase*>children;
};
