#ifndef SHORTCUTS_DLG_H
#define SHORTCUTS_DLG_H

#include <QDialog>
#include <QStandardItemModel> 
#include <QSortFilterProxyModel>
#include <QTreeWidget>
#include <QHeaderView>
#include <QVector>
#include <QMap>
#include <QStringList>

#include "ui_shortcutsdlg.h"
#include "shortcut_delegate.h"
#include "shortcut.h"

#define NAME_COL 0
#define KEY_COL 1

class ShortcutDlg : public QDialog, public Ui::ShortcutsDlg
{
    Q_OBJECT
public:
    ShortcutDlg()
    {
        setupUi(this);
        setAttribute(Qt::WA_DeleteOnClose);
    }

    void init()
    {
        model_.clear();
        model_.setColumnCount(2);
        model_.setHorizontalHeaderLabels(QStringList() << tr("Shortcut") << tr("Key"));

        createTreeModel();

        proxy_.setSourceModel(&model_);
        ShortcutDelegate *sc_delegate = new ShortcutDelegate(tw_shortcuts);
        connect(sc_delegate, SIGNAL(shortcutChanged(const QString &, const QString &)), this, SLOT(onShortcutChanged(const QString &, const QString &)));
        tw_shortcuts->setItemDelegate(sc_delegate);
        tw_shortcuts->setModel(&proxy_);

        tw_shortcuts->header()->setResizeMode(NAME_COL,QHeaderView::Stretch);
        tw_shortcuts->header()->setResizeMode(KEY_COL,QHeaderView::ResizeToContents);
        tw_shortcuts->expandAll();
    }

    void createTreeModel();

private slots:
    void onShortcutChanged(const QString &id, const QString &key)
    {
        changed_scs_.append(id);
    }

    void onShortcutAdded(const Shortcut *sc, const SCGroup *group)
    {
        QList<QStandardItem *> groups = model_.findItems(group->descript);
        if ( groups.isEmpty() )
            return;

        QStandardItem *name_item = new QStandardItem(sc->descript());
        name_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        name_item->setData(0, MDR_SC_SORTROLE);
        name_item->setData(sc->id(), MDR_SC_IDROLE);

        QStandardItem *key_item = new QStandardItem(sc->key());
        key_item->setData(sc->key(), MDR_SC_KEYROLE);
        groups[0]->appendRow(QList<QStandardItem *>() << name_item << key_item);

        sc_items_.insert(sc->id(), name_item);
    }

    void onShortcutRemoved(const Shortcut *sc, const SCGroup *group)
    {
        QList<QStandardItem *> groups = model_.findItems(group->descript);
        if ( groups.isEmpty() )
            return;

        int index = group->shortcuts.indexOf((Shortcut *)sc);
        groups[0]->removeRow(index);
    }

    void onGroupAdded(const SCGroup *group)
    {
        QStandardItem *name_item = new QStandardItem(group->descript);
        name_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        name_item->setData(group->order, MDR_SC_SORTROLE);

        QStandardItem *key_item = new QStandardItem();
        model_.invisibleRootItem()->appendRow(QList<QStandardItem *>() << name_item << key_item);
    }

    void on_ok_btn_clicked();
    void on_cancel_btn_clicked();

private:
    virtual void closeEvent(QCloseEvent *ev);

private:
    class SortFilterProxyModel : public QSortFilterProxyModel
    {
    protected:
        virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const
        {
            bool left_has_child = left.child(0,0).isValid();
            bool right_has_child = right.child(0,0).isValid();

            if (left_has_child && !right_has_child)
                return true;
            else if (!left_has_child && right_has_child)
                return false;
            else if (left_has_child && right_has_child)
                return left.data(MDR_SC_SORTROLE).toInt() < right.data(MDR_SC_SORTROLE).toInt();

            return QSortFilterProxyModel::lessThan(left,right);
        }
    };

    SortFilterProxyModel proxy_;
    QStandardItemModel model_;

    QMap<QString, QStandardItem *> sc_items_;
    QStringList changed_scs_;
};

#endif //SHORTCUTS_DLG_H
