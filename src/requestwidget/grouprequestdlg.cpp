#include "grouprequestdlg.h"
#include "ui_grouprequestdlg.h"

#include <assert.h>

#include <QDateTime>
#include <QDesktopWidget>

#include "json/json.h"

#include "core/qqmsg.h"
#include "core/captchainfo.h"
#include "core/talkable.h"
#include "core/request.h"
#include "qqiteminfohelper.h"

GroupRequestDlg::GroupRequestDlg(const ShareQQMsgPtr msg, Contact *contact, Group *group, QWidget *parent) : QDialog(parent),
    ui_(new Ui::GroupRequestDlg)
{
	assert(group);

    ui_->setupUi(this);

	QQSystemGMsg *sysg_msg = (QQSystemGMsg *)msg.data();
    QString name;

    if (contact)
    {
        name = contact->name();
    }
    else
    {
        
		name = sysg_msg->talkTo();
    }
    ui_->lbl_request_name_->setText(name);
    ui_->lbl_group_name_->setText(group->name());

	if ( contact )
	{
		ui_->lbl_avatar_->setPixmap(contact->avatar());
	}
    ui_->pte_msg_->appendPlainText(msg->msg());

    gid_ = msg->talkTo();
    id_ = msg->sendUin();

    connect(ui_->pb_ok_ , SIGNAL(clicked()), this, SLOT(slotOkClicked()));
    connect(ui_->pb_ignore_ , SIGNAL(clicked()), this, SLOT(slotIgnoreClicked()));
    connect(ui_->rb_deny_, SIGNAL(toggled(bool)), this, SLOT(slotToggleDenyReason(bool)));


    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);
}

void GroupRequestDlg::slotOkClicked()
{
    if (ui_->rb_allow_->isChecked())
    {
        QString allow_url = "/channel/op_group_join_req?group_uin=" + gid_ + "&req_uin=" + id_ +
                "&msg=&op_type=2&clientid=5412354841&psessionid=" + CaptchaInfo::instance()->psessionid() +"&t=" + QString::number(QDateTime::currentMSecsSinceEpoch());

        Request req;
        req.create(kGet, allow_url);
        req.addHeaderItem("Host", "d.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
        req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=2");
        req.addHeaderItem("Content-Type", "utf-8");

        QTcpSocket fd;
        fd.connectToHost("d.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        qDebug()<<"allow add group request: "<<fd.readAll();

        accept();
    }
    else
    {
        QString deny_url = "/channel/op_group_join_req?group_uin=" + gid_ + "&req_uin=" + id_ +
                "&msg=" + ui_->le_deny_reason_->text() + "&op_type=3&clientid=5412354841&psessionid=" + CaptchaInfo::instance()->psessionid() +"&t=" + QString::number(QDateTime::currentMSecsSinceEpoch());

        Request req;
        req.create(kGet, deny_url);
        req.addHeaderItem("Host", "d.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
        req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=2");
        req.addHeaderItem("Content-Type", "utf-8");

        QTcpSocket fd;
        fd.connectToHost("d.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        qDebug()<<"deny add group request"<<fd.readAll();

        fd.close();
        reject();
    }
}

void GroupRequestDlg::updateRequesterInfo(Contact *contact)
{
    ui_->lbl_request_name_->setText(contact->name());
}

void GroupRequestDlg::updateRequesterIcon(const QByteArray &icon_data)
{
	QPixmap pix;
	pix.loadFromData(icon_data);

	ui_->lbl_avatar_->setPixmap(pix);
}

void GroupRequestDlg::slotIgnoreClicked()
{
    this->reject();
}

void GroupRequestDlg::slotToggleDenyReason(bool check)
{
    ui_->le_deny_reason_->setEnabled(check);
}
