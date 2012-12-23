#ifndef ROSTER_MODEL_H
#define ROSTER_MODEL_H

#include <assert.h>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QHash>

#include "rostermodel/roster_model_base.h"
#include "core/talkable.h"

class RosterIndex;

class RosterModel : public __RosterModelBase
{
	Q_OBJECT
public:
	RosterModel(QObject *parent = NULL);
	~RosterModel();

	void clean();

public slots:
	void slotNewCategoryItem(const Category *cat);
	void slotNewContactItem(const Contact *contact);
	void slotNewGroupItem(const Group *group);
	
	void slotTalkableDataChanged(QString id, QVariant data, TalkableDataRole role);
	void slotCategoryDataChanged(int index, QVariant data, TalkableDataRole role);

	void slotOnDoubleclicked(const QModelIndex &index);


private:
	RosterIndex *findCategoryIndex(int cat_index);
	RosterIndex *findRosterIndexById(QString id);

	void changedData(QString id, QString gcode, QVariant data, TalkableDataRole role);
private:
	QHash<QString, RosterIndex *> indexs_;
};

#endif //ROSTER_MODEL_H
