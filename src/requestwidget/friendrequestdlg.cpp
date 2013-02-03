#include "friendrequestdlg.h"
#include "ui_friendrequestdlg.h"

#include <QDir>
#include <QPixmap>
#include <QTcpSocket>

#include "core/captchainfo.h"
#include "core/qqavatarrequester.h"
#include "core/qqmsg.h"
#include "core/request.h"
#include "core/talkable.h"
#include "qqiteminfohelper.h"
#include "roster/roster.h"
#include "strangermanager/stranger_manager.h"

FriendRequestDlg::FriendRequestDlg(const ShareQQMsgPtr msg, Contact *contact, QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::FriendRequestDlg),
	contact_(contact)
{
    ui_->setupUi(this);

	initUi(msg);
	initConnections();

	const QQSystemMsg *sys_msg = static_cast<const QQSystemMsg*>(msg.data());
    id_ = sys_msg->from_;
    account_ = sys_msg->account_;
}

void FriendRequestDlg::initUi(ShareQQMsgPtr msg)
{
	const QQSystemMsg *sys_msg = static_cast<const QQSystemMsg*>(msg.data());

	if ( contact_ )
	{
		QPixmap pix = contact_->avatar();
		if ( !pix.isNull() )
			ui_->lbl_avatar_->setPixmap(pix);

		ui_->lbl_account_->setText(contact_->name());
	}
	else
	{
		ui_->lbl_account_->setText(sys_msg->account_);
	}
	ui_->pte_msg_->appendPlainText(sys_msg->msg_);

	foreach ( Category *cat, Roster::instance()->categorys() )
	{
		ui_->cb_group_->addItem(cat->name(), cat->index());
	}
}

void FriendRequestDlg::initConnections()
{
	connect(StrangerManager::instance(), SIGNAL(newStrangerInfo(QString, Contact *)), this, SLOT(updateRequesterInfo(QString, Contact *)));
	connect(StrangerManager::instance(), SIGNAL(newStrangerIcon(QString, QPixmap)), this, SLOT(updateRequesterIcon(QString, QPixmap)));


	connect(ui_->pb_ok_ , SIGNAL(clicked()), this, SLOT(slotOkClicked()));
	connect(ui_->pb_ignore_ , SIGNAL(clicked()), this, SLOT(slotIgnoreClicked()));
	connect(ui_->rb_deny_, SIGNAL(toggled(bool)), this, SLOT(slotToggleDenyReason(bool)));
}

void FriendRequestDlg::updateRequesterInfo(QString id, Contact *contact)
{
	if ( id != id_ )
		return;

	contact_ = contact;
	QPixmap pix = contact->avatar();
	if ( !pix.isNull() )
		ui_->lbl_avatar_->setPixmap(pix);

	ui_->lbl_account_->setText(contact->name());
}

void FriendRequestDlg::updateRequesterIcon(QString id, QPixmap pix)
{
	if ( id != id_ )
		return;

	ui_->lbl_avatar_->setPixmap(pix);
}

void FriendRequestDlg::slotOkClicked()
{
    if (ui_->rb_allow_and_add_->isChecked())
    {
		QString markname = ui_->le_comment_->text();
        QString allow_and_add_url = "/api/allow_and_add2";

        int category_idx = ui_->cb_group_->itemData(ui_->cb_group_->currentIndex()).toInt();
        QByteArray msg = "r={\"account\":" + account_.toAscii() + ",\"gid\":"+ QString::number(category_idx).toAscii() +
                ",\"mname\":\"" + markname.toAscii() + "\",\"vfwebqq\":\"" + CaptchaInfo::instance()->vfwebqq().toAscii() + "\"}";

        Request req;
        req.create(kPost, allow_and_add_url);
        req.addHeaderItem("Host", "s.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
        req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=2");
        req.addHeaderItem("Content-Length", QString::number(msg.length()));
        req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
        req.addRequestContent(msg);

        QTcpSocket fd;
        fd.connectToHost("s.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        QByteArray result = fd.readAll();
        fd.close();
		qDebug() << "add friend result: " << result << endl;

        //int stat_idx = result.indexOf("stat")+6;
        //int stat_end_idx = result.indexOf("}", stat_idx);
		//Contact *contact = new Contact();
		Roster *roster = Roster::instance();
		Category *cat = roster->category(category_idx);
		contact_->setMarkname(markname);
		roster->addContact(contact_, cat);
        //model_->addItem(id_, ui_->le_comment_->text().toAscii(), category_idx, (ContactStatus)result.mid(stat_idx, stat_end_idx - stat_idx).toInt());
        accept();
    }
    else if (ui_->rb_allow_->isChecked())
    {
        QString allow_add_request_url = "/api/allow_added_request2";
        QByteArray msg = "r={\"account\":" + account_.toAscii() + ",\"vfwebqq\":\"" + CaptchaInfo::instance()->vfwebqq().toAscii() + "\"}";

        Request req;
        req.create(kPost, allow_add_request_url);
        req.addHeaderItem("Host", "s.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
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
                "\",\"vfwebqq\":\"" + CaptchaInfo::instance()->vfwebqq().toAscii() + "\"}";

        Request req;
        req.create(kPost, deny_add_request);
        req.addHeaderItem("Host", "s.web2.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
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

void FriendRequestDlg::slotIgnoreClicked()
{
    this->reject();
}

void FriendRequestDlg::slotToggleDenyReason(bool check)
{
    ui_->le_deny_reason_->setEnabled(check);
}
