#include "qqgrouprequestdlg.h"
#include "ui_grouprequestdlg.h"

#include <QDateTime>

#include "qqiteminfohelper.h"
#include "groupitemmodel.h"
#include "frienditemmodel.h"
#include "qqmsg.h"
#include "include/json/json.h"

QQGroupRequestDlg::QQGroupRequestDlg(const QQMsg *msg, FriendItemModel *f_model, GroupItemModel *g_model, QWidget *parent) : QDialog(parent),
    ui_(new Ui::QQGroupRequestDlg)
{
    ui_->setupUi(this);

    QQItem *f_item = f_model->find(msg->sendUin());
    QString name;
    if (f_item)
    {
        name = f_item->name();
    }
    else
    {
        QByteArray result = QQItemInfoHelper::getStrangetInfo2(msg->sendUin(), msg->talkTo());
        result = result.mid(result.indexOf("\r\n\r\n")+4);
        Json::Value root;
        Json::Reader reader;

        if (reader.parse(QString(result).toStdString(), root, false))
        {
            name = QString::fromStdString(root["result"]["nick"].asString());
        }
    }
    ui_->lbl_request_name_->setText(name);
    QQItem *g_item = g_model->find(msg->talkTo());
    ui_->lbl_group_name_->setText(g_item->name());

    QPixmap pix(g_item->avatarPath());
    ui_->lbl_avatar_->setPixmap(pix);
    ui_->pte_msg_->appendPlainText(msg->msg());

    gid_ = msg->talkTo();
    id_ = msg->sendUin();

    connect(ui_->pb_ok_ , SIGNAL(clicked()), this, SLOT(slotOkClicked()));
    connect(ui_->pb_ignore_ , SIGNAL(clicked()), this, SLOT(slotIgnoreClicked()));
    connect(ui_->rb_deny_, SIGNAL(toggled(bool)), this, SLOT(slotToggleDenyReason(bool)));

    delete msg;
}

void QQGroupRequestDlg::slotOkClicked()
{
    if (ui_->rb_allow_->isChecked())
    {
        QString allow_url = "/channel/op_group_join_req?group_uin=" + gid_ + "&req_uin=" + id_ +
                "&msg=&op_type=2&clientid=5412354841&psessionid=" + CaptchaInfo::singleton()->psessionid() +"&t=" + QString::number(QDateTime::currentMSecsSinceEpoch());

        Request req;
        req.create(kGet, allow_url);
        req.addHeaderItem("Host", "d.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
        req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=2");
        req.addHeaderItem("Content-Type", "utf-8");

        QTcpSocket fd;
        fd.connectToHost("d.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        qDebug()<<fd.readAll();

        accept();
    }
    else
    {
        QString deny_url = "/channel/op_group_join_req?group_uin=" + gid_ + "&req_uin=" + id_ +
                "&msg=" + ui_->le_deny_reason_->text() + "&op_type=3&clientid=5412354841&psessionid=" + CaptchaInfo::singleton()->psessionid() +"&t=" + QString::number(QDateTime::currentMSecsSinceEpoch());

        Request req;
        req.create(kGet, deny_url);
        req.addHeaderItem("Host", "d.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
        req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=2");
        req.addHeaderItem("Content-Type", "utf-8");

        QTcpSocket fd;
        fd.connectToHost("d.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        qDebug()<<fd.readAll();

        fd.close();
        reject();
    }
}

void QQGroupRequestDlg::slotIgnoreClicked()
{
    this->reject();
}

void QQGroupRequestDlg::slotToggleDenyReason(bool check)
{
    ui_->le_deny_reason_->setEnabled(check);
}
