#ifndef ICON_DECORATOR_H
#define ICON_DECORATOR_H

#include "utils/contact_status.h"

class QPixmap;

class IconDecorator
{
public:
	static void decorateIcon(ContactStatus status, QPixmap &pix);
	static void setStatusDecoration(ContactStatus status, QPixmap &px);

private:
};

#endif //ICON_DECORATOR_H
