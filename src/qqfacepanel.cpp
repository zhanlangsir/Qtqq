#include "qqfacepanel.h"
#include "ui_qqfacepanel.h"

#include <QTableWidgetItem>

#include "qqglobal.h"

QQFacePanel::QQFacePanel(QWidget *parent) : QDialog(parent), ui_(new Ui::QQFacePanel),
    qqface_path_(QQGlobal::resourceDir() + "/qqface/default/")
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
    for ( unsigned int i = 0; i < sizeof(no2path)/sizeof(no2path[0]); ++i )
    {
        int row = i / kPerColumn_;
        int column = i % kPerColumn_;

        if (row > ui_->tbw_qqface_->rowCount()-1)
        {
            ui_->tbw_qqface_->insertRow(ui_->tbw_qqface_->rowCount());
        }

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setIcon(QIcon(qqface_path_ + QString::number(no2path[i]) + ".gif"));
        item->setData(Qt::UserRole, QString::number(no2path[i]));

        ui_->tbw_qqface_->setItem(row, column,item);
    }
}

const int QQFacePanel::no2path[105] = {
        14,  1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,   0,   
        50,  51,  96,  53,  54,  73,  74,  75,  76,  77,  78,  55,  56,  57,   58,  
        79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  97,  98,  99,  100,  101, 
        102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 32,  113, 114,  115, 
        63,  64,  59,  33,  34,  116, 66,  68,  38,  91,  92,  93,  29,  117,  72,
        45,  42,  39,  62,  46,  47,  71,  95,  118, 119, 120, 121, 122, 123,  124,
        27,  21,  23,  25,  26,  125, 126, 127, 128, 129, 130, 131, 132, 133,  134 
        };

