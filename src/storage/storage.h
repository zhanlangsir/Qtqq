#ifndef STORAGE_H
#define STORAGE_H

#include <QHash>

class Storage
{
public:
	static Storage* instance()
	{
		if ( !instance_ )
			instance_ = new Storage();
		return instance_;
	}

	bool hasIcon(QString &id) const;
	QByteArray icon(QString &id) const
	{
		return icons_[id];
	}

	QByteArray takeIcon(QString &id);
	void addIcon(QString &id, QByteArray &data);

	QString singleLongNick(QString &id) const
	{
		return slns_[id];
	}
	QString takeSingleLongNick(QString &id) 
	{
		return slns_[id];
	}

private:
	QHash<QString, QByteArray> icons_;
	QHash<QString, QString> slns_;  //single long nick

private:
	Storage() {}
	Storage(const Storage&);
	Storage& operator=(const Storage&);

	static Storage* instance_;
};

#endif //STORAGE_H
