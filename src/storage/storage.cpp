#include "storage.h"

#include <QString>
#include <QByteArray>

Storage* Storage::instance_ = NULL;

bool Storage::hasIcon(QString &id) const
{
	return icons_.contains(id);
}

void Storage::addIcon(QString &id, QByteArray &data) 
{
	icons_[id] = data;
}
 
QByteArray Storage::takeIcon(QString &id)
{
	if ( hasIcon( id ) )
	{
		QByteArray icon_data = icons_.value(id);
		icons_.remove(id);

		return icon_data;
	}
	else
		return QByteArray();
}
