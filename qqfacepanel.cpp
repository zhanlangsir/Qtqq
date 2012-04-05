#include "qqfacepanel.h"
#include "ui_qqfacepanel.h"

#include <QDir>
#include <QFileInfoList>

QQFacePanel::QQFacePanel(QWidget *parent) : QDialog(parent), ui_(new Ui::QQFacePanel), qqface_path_("./images/qqface/default/")
{
    ui_->setupUi(this);
    ui_->tbw_qqface_->setIconSize(QSize(30,30));
    ui_->tbw_qqface_->setColumnCount(kPerColumn_);
    ui_->tbw_qqface_->setRowCount(1);
    ui_->tbw_qqface_->setEditTriggers(QTableWidget::NoEditTriggers);
    connect(ui_->tbw_qqface_, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onItemClicked(QTableWidgetItem*)));
    loadFace();
    setWindowOpacity(1);
    setWindowFlags(Qt::FramelessWindowHint);
}

QQFacePanel::~QQFacePanel()
{
    delete ui_;
}

void QQFacePanel::onItemClicked(QTableWidgetItem *item)
{
    this->hide();
    emit qqfaceClicked(item->data(Qt::UserRole).toString());
}

void QQFacePanel::leaveEvent(QEvent *)
{
    this->hide();
}

void QQFacePanel::loadFace()
{
    QDir qqface_dir(qqface_path_);
    QFileInfoList file_list = qqface_dir.entryInfoList(QDir::Files);
    QFileInfo file;

    int curr_file_count = 0;
    foreach(file, file_list)
    {
        int row = curr_file_count / kPerColumn_;
        int column = curr_file_count % kPerColumn_;

        if (row > ui_->tbw_qqface_->rowCount()-1)
        {
            ui_->tbw_qqface_->insertRow(ui_->tbw_qqface_->rowCount());
        }

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setIcon(QIcon(qqface_path_ + file.fileName()));
        item->setData(Qt::UserRole, file.baseName());

        ui_->tbw_qqface_->setItem(row, column,item);

        ++curr_file_count;
    }
}
