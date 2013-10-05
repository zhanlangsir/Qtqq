#include "shortcuts_dlg.h"

#include <QDomDocument>
#include <QDomElement>
#include <QCloseEvent>
#include <QDebug>

#include "shortcut_manager.h"
#include "setting/setting.h"

void ShortcutDlg::createTreeModel()
{
    model_.clear();
    model_.setColumnCount(2);
    model_.setHorizontalHeaderLabels(QStringList() << tr("Shortcut") << tr("Key"));

    foreach ( SCGroup *group, ShortcutManager::instance()->groups() )
    {
        QStandardItem *name_item = new QStandardItem(group->descript);
        name_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        name_item->setData(group->order, MDR_SC_SORTROLE);

        QStandardItem *key_item = new QStandardItem();

        model_.invisibleRootItem()->appendRow(QList<QStandardItem *>() << name_item << key_item);

        foreach ( Shortcut *shortcut, group->shortcuts )
        {
            QStandardItem *sc_name_item = new QStandardItem(shortcut->descript());
            sc_name_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            sc_name_item->setData(0, MDR_SC_SORTROLE);
            sc_name_item->setData(shortcut->id(), MDR_SC_IDROLE);

            QStandardItem *sc_key_item = new QStandardItem(shortcut->key());
            sc_key_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            sc_key_item->setData(shortcut->id(), MDR_SC_IDROLE);
            sc_key_item->setData(shortcut->key(), MDR_SC_KEYROLE); name_item->appendRow(QList<QStandardItem *>() << sc_name_item << sc_key_item);
            sc_items_.insert(shortcut->id(), sc_name_item);
        }
    }
}

void ShortcutDlg::on_ok_btn_clicked()
{
    if ( changed_scs_.empty() )
    {
        close();
        return;
    }

    QDomElement shortcut_elem = Setting::instance()->documentElement().firstChildElement(SHORTCUT_ELEMENT_TAG);
    if ( shortcut_elem.isNull() )
    {
        shortcut_elem = Setting::instance()->createElement(SHORTCUT_ELEMENT_TAG);
        Setting::instance()->documentElement().appendChild(shortcut_elem);
    }

    foreach ( const QString &id, changed_scs_ )
    {
        Shortcut *sc = ShortcutManager::instance()->shortcut(id);
        if ( sc != NULL )
        {
            QStandardItem *name_item = sc_items_[id];
            QStandardItem *key_item = name_item->parent()->child(name_item->row(), KEY_COL);

            QString key = key_item->data(MDR_SC_KEYROLE).toString();
            if ( key.isEmpty() )
            {
                sc->setEnabled(false);
            }
            else
            {
                sc->setEnabled(true);
                qDebug() << key_item->data(MDR_SC_KEYROLE).toString();
                sc->setKey(key_item->data(MDR_SC_KEYROLE).toString());

                SCGroup *group = sc->group();
                QDomElement group_elem = shortcut_elem.firstChildElement(group->id);
                if ( group_elem.isNull() )
                {
                    group_elem = Setting::instance()->createElement(group->id);
                    shortcut_elem.appendChild(group_elem);
                }

                QDomElement sc_elem = group_elem.firstChildElement(sc->id());
                if ( sc_elem.isNull() )
                {
                    sc_elem = Setting::instance()->createElement(sc->id());
                    group_elem.appendChild(sc_elem);
                }
                sc_elem.setAttribute(KEY_ATTR, sc->key());

                Setting::instance()->save();
            }
        }
        else
        {
            qDebug() << "Not exist shortcut, id: " << id << endl;
        }
    }

    close();
}

void ShortcutDlg::on_cancel_btn_clicked()
{
    reject();
}

void ShortcutDlg::closeEvent(QCloseEvent *ev)
{
    changed_scs_.clear();
}
