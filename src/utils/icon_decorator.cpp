#include "icon_decorator.h"

#include <QPainter>
#include <QPointF>
#include <QPixmap>
#include <QIcon>
#include <QDebug>

#include "utils/contact_status.h"
#include "skinengine/qqskinengine.h"
#include "qqglobal.h"

void IconDecorator::decorateIcon(ContactStatus status, QPixmap &pix)
{
	QIcon icon;
    icon.addPixmap(pix);

    if ( status == CS_Offline )
		pix = icon.pixmap(QSize(40,40), QIcon::Disabled, QIcon::On);
	else
	{
		QPixmap decorated_icon = icon.pixmap(QSize(40,40));
		setStatusDecoration(status ,decorated_icon);

		pix = decorated_icon;
	}
}

void IconDecorator::setStatusDecoration(ContactStatus status, QPixmap &pix)
{
	if (status == CS_Online || status == CS_Offline)
		return;

	QPainter painter(&pix);
	QImage img;

	if ( status == CS_CallMe )
	{
		img.load(QQSkinEngine::instance()->skinRes("status_qme"));
	}
	else if ( status == CS_Busy )
	{
		img.load(QQSkinEngine::instance()->skinRes("status_busy"));
	}
	else if ( status == CS_Away )
	{
		img.load(QQSkinEngine::instance()->skinRes("status_away"));
	}
	else if ( status == CS_Silent )
	{
		img.load(QQSkinEngine::instance()->skinRes("status_silent"));
	}

	QSize avatar_size = pix.size();
	QSize decoration_size = img.size();
	QPointF draw_point(avatar_size.width() - decoration_size.width(), avatar_size.height() - decoration_size.height());
	painter.drawImage(draw_point, img);
}


