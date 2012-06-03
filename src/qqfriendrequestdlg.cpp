#include "qqfriendrequestdlg.h"
#include "ui_friendrequestdlg.h"

#include <QTcpSocket>

#include "frienditemmodel.h"
#include "core/qqavatarrequester.h"
#include "core/qqmsg.h"
#include "core/request.h"
#include "core/captchainfo.h"

QQFriendRequestDlg::QQFriendRequestDlg(const ShareQQMsgPtr msg, FriendItemModel *model, QWidget *parent) : QDialog(parent),
    ui_(new Ui::QQFriendRequestDlg),
    model_(model)
{
    ui_->setupUi(this);
    initialize();
    const QQSystemMsg *sys_msg = static_cast<const QQSystemMsg*>(msg.data());
    QString path = QQAvatarRequester::requestOne(QQAvatarRequester::getTypeNumber(QQItem::kFriend),
                                                 sys_msg->from_,
                                                 "temp/avatar/");
    QPixmap pix(path);
    ui_->lbl_avatar_->setPixmap(pix);
    ui_->lbl_account_->setText(sys_msg->account_);
    ui_->pte_msg_->appendPlainText(sys_msg->msg_);

    connect(ui_->pb_ok_ , SIGNAL(clicked()), this, SLOT(slotOkClicked()));
    connect(ui_->pb_ignore_ , SIGNAL(clicked()), this, SLOT(slotIgnoreClicked()));
    connect(ui_->rb_deny_, SIGNAL(toggled(bool)), this, SLOT(slotToggleDenyReason(bool)));

    QQItem *item = NULL;
    foreach (item, model->categorys())
    {
        ui_->cb_group_->addItem(item->name(), item->id());
    }
    id_ = sys_msg->from_;
    account_ = sys_msg->account_;
}

void QQFriendRequestDlg::slotOkClicked()
{
    if (ui_->rb_allow_and_add_->isChecked())
    {
        QString allow_and_add_url = "/api/allow_and_add2";

        QString groupidx = ui_->cb_group_->itemData(ui_->cb_group_->currentIndex()).toString();
        QByteArray msg = "r={\"account\":" + account_.toAscii() + ",\"gid\":"+ groupidx.toAscii() +
                ",\"mname\":\"" + ui_->le_comment_->text().toAscii() + "\",\"vfwebqq\":\"" + CaptchaInfo::singleton()->vfwebqq().toAscii() + "\"}";

        Request req;
        req.create(kPost, allow_and_add_url);
        req.addHeaderItem("Host", "s.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
        req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=2");
        req.addHeaderItem("Content-Length", QString::number(msg.length()));
        req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
        req.addRequestContent(msg);

        QTcpSocket fd;
        fd.connectToHost("s.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        QByteArray result = fd.readAll();

        int stat_idx = result.indexOf("stat")+6;
        int stat_end_idx = result.indexOf("}", stat_idx);
        model_->addFriend(id_, ui_->le_comment_->text().toAscii(), groupidx, (FriendStatus)result.mid(stat_idx, stat_end_idx - stat_idx).toInt());
        fd.close();
        accept();
    }
    else if (ui_->rb_allow_->isChecked())
    {
        QString allow_add_request_url = "/api/allow_added_request2";
        QByteArray msg = "r={\"account\":" + account_.toAscii() + ",\"vfwebqq\":\"" + CaptchaInfo::singleton()->vfwebqq().toAscii() + "\"}";

        Request req;
        req.create(kPost, allow_add_request_url);
        req.addHeaderItem("Host", "s.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
        req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=2");
        req.addHeaderItem("Content-Length", QString::number(msg.length()));
        req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
        req.addRequestContent(msg);

        QTcpSocket fd;
        fd.connectToHost("s.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        qDebug()<<"Allow friend add reqeust:\n"<<fd.readAll();
        fd.close();
        accept();
    }
    else
    {
        QString deny_add_request = "/api/deny_added_request2";

        QByteArray msg = "r={\"account\":" + account_.toAscii() + ",\"msg\":\""+ ui_->le_deny_reason_->text().toAscii() +
                "\",\"vfwebqq\":\"" + CaptchaInfo::singleton()->vfwebqq().toAscii() + "\"}";

        Request req;
        req.create(kPost, deny_add_request);
        req.addHeaderItem("Host", "s.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
        req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=2");
        req.addHeaderItem("Content-Length", QString::number(msg.length()));
        req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
        req.addRequestContent(msg);

        QTcpSocket fd;
        fd.connectToHost("s.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        qDebug()<<"Deny friend add reqeust\n: "<<fd.readAll();

        fd.close();
        reject();
    }
}

void QQFriendRequestDlg::slotIgnoreClicked()
{
    this->reject();
}

void QQFriendRequestDlg::slotToggleDenyReason(bool check)
{
    ui_->le_deny_reason_->setEnabled(check);
}

void QQFriendRequestDlg::initialize()
{

}
