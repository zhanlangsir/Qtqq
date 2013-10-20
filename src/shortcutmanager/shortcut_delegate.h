#ifndef SHORTCUT_DELEGATE_H
#define SHORTCUT_DELEGATE_H

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QLineEdit>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

#include "shortcut.h"

#define MDR_SC_SORTROLE Qt::UserRole+1
#define MDR_SC_KEYROLE Qt::UserRole+2
#define MDR_SC_IDROLE Qt::UserRole+3

class ShortcutDelegate : public QStyledItemDelegate
{
    Q_OBJECT
signals:
    void shortcutChanged(const QString &id, const QString &key) const;

public:
    ShortcutDelegate(QObject *parent = 0) :
    QStyledItemDelegate(parent)
    {
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        Q_UNUSED(index)

        QLineEdit *editor = new QLineEdit(parent);
        editor->installEventFilter((QObject *)this);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString key = index.data(MDR_SC_KEYROLE).toString();
        QLineEdit *key_editor = qobject_cast<QLineEdit *>(editor);
        key_editor->setText(key);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QLineEdit *key_editor = static_cast<QLineEdit *>(editor);
        QString key = key_editor->text();
        QString pre_key = index.data(MDR_SC_KEYROLE).toString();
        qDebug() << "pre key: " << pre_key << endl;

        if ( pre_key != key )
        {
            QString id = index.data(MDR_SC_IDROLE).toString();
            qDebug() << "shortcut changed :" << id << endl;
            model->setData(index, key, Qt::DisplayRole);
            model->setData(index, key, MDR_SC_KEYROLE);

            emit shortcutChanged(id, key);
        }
    }

    bool eventFilter(QObject *obj, QEvent *ev)
    {
        QLineEdit *key_editor = qobject_cast<QLineEdit *>(obj);
        if ( key_editor )
        {
            if ( ev->type() == QEvent::KeyPress )
            {
                //static const int esc_key = 0x010000000;
                static const QList<int> control_key = QList<int>() << Qt::Key_Shift << Qt::Key_Control << Qt::Key_Alt << Qt::Key_Meta;
                static const int modif_key = Qt::ALT | Qt::SHIFT | Qt::CTRL;

                QKeyEvent *key_ev = static_cast<QKeyEvent *>(ev);
                if ( key_ev->key() == 0 || key_ev->key() == Qt::Key_unknown )
                    return true;
                if ( control_key.contains(key_ev->key()) )
                    return true;

                QKeySequence key_seq((key_ev->modifiers() & modif_key) | key_ev->key());
                key_editor->setText(key_seq.toString(QKeySequence::NativeText));
                return true;
            }
            else if ( ev->type() == QEvent::KeyRelease )
            {
                emit commitData(key_editor);
                emit closeEditor(key_editor);
            }
        }

        return QStyledItemDelegate::eventFilter(obj, ev);
    }
};

#endif //SHORTCUT_DELEGATE_H
