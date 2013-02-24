#include "friendrequestdlg.h"
#include "ui_friendrequestdlg.h"

#include <assert.h>

#include <QDir>
#include <QPixmap>
#include <QTcpSocket>
#include <QDesktopWidget>

#include "core/captchainfo.h"
#include "core/qqmsg.h"
#include "core/request.h"
#include "core/talkable.h"
#include "qqiteminfohelper.h"
#include "roster/roster.h"
#include "strangermanager/stranger_manager.h"

FriendRequestDlg::FriendRequestDlg(QString requester_id, QString requester_qq_number, QString msg, Contact *contact, QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::FriendRequestDlg),
    id_(requester_id),
    qq_number_(requester_qq_number),
    msg_(msg)
{
    ui_->setupUi(this);

	initUi(contact);
	initConnections();
}

void FriendRequestDlg::initUi(Contact *contact)
{
	if ( contact )
	{
		QPixmap pix = contact->avatar();
		if ( !pix.isNull() )
			ui_->lbl_avatar_->setPixmap(pix);

        if ( contact->name().isEmpty() )
            ui_->lbl_account_->setText(qq_number_);
        else
            ui_->lbl_account_->setText(contact->name());
	}
	else
	{
		ui_->lbl_account_->setText(qq_number_);
	}
	ui_->pte_msg_->appendPlainText(msg_);

	foreach ( Category *cat, Roster::instance()->categorys() )
	{
		ui_->cb_group_->addItem(cat->name(), cat->index());
	}

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);
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
        if ( Roster::instance()->contact(id_) )
        {
            accept();
            return;
        }

		QString markname = ui_->le_comment_->text();
        QString allow_and_add_url = "/api/allow_and_add2";

        int category_idx = ui_->cb_group_->itemData(ui_->cb_group_->currentIndex()).toInt();
        QByteArray msg = "r={\"account\":" + qq_number_.toAscii() + ",\"gid\":"+ QString::number(category_idx).toAscii() +
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

		qDebug() << "Added friend result:\n" 
            << result << endl;

        ContactStatus status = extractStatus(result);

		Roster *roster = Roster::instance();
		Category *cat = roster->category(category_idx);
        Contact *stranger = StrangerManager::instance()->takeStranger(id_);
        assert(stranger);

        stranger->setType(Talkable::kContact);
        stranger->setStatus(status);
		stranger->setMarkname(markname);
		roster->addContact(stranger, cat);
        accept();
    }
    else if (ui_->rb_allow_->isChecked())
    {
        if ( Roster::instance()->contact(id_) )
            return;

        QString allow_add_request_url = "/api/allow_added_request2";
        QByteArray msg = "r={\"account\":" + qq_number_.toAscii() + ",\"vfwebqq\":\"" + CaptchaInfo::instance()->vfwebqq().toAscii() + "\"}";

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

        QByteArray msg = "r={\"account\":" + qq_number_.toAscii() + ",\"msg\":\""+ ui_->le_deny_reason_->text().toAscii() +
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

    deleteLater();
}

ContactStatus FriendRequestDlg::extractStatus(const QByteArray &result)
{
    int status_s_idx = result.indexOf("stat")+6; 
    int status_e_idx = result.indexOf('}', status_s_idx);

    int stat = result.mid(status_s_idx, status_e_idx - status_s_idx).toInt();
    qDebug() << "stat" << stat << endl;

    return (ContactStatus)stat;
}

void FriendRequestDlg::slotIgnoreClicked()
{
    this->reject();
    deleteLater();
}

void FriendRequestDlg::slotToggleDenyReason(bool check)
{
    ui_->le_deny_reason_->setEnabled(check);
}
