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
signals:
    void sigDoubleClicked(const RosterIndex *index);

public:
	RosterModel(QObject *parent = NULL);
	~RosterModel();

	void clean();

public slots:
	void addCategoryItem(Category *cat);
	void addContactItem(Contact *contact);
	void addGroupItem(Group *group);
	
	void talkableDataChanged(QString id, QVariant data, TalkableDataRole role);
	void categoryDataChanged(int index, QVariant data, TalkableDataRole role);

	void onDoubleClicked(const QModelIndex &index);


private:
	RosterIndex *findCategoryIndex(int cat_index);
	RosterIndex *findRosterIndexById(QString id);

	void changedData(QString id, QString gcode, QVariant data, TalkableDataRole role);
private:
	QHash<QString, RosterIndex *> indexs_;
};

#endif //ROSTER_MODEL_H
