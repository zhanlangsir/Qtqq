#ifndef LOADIMG_JOB_H
#define LOADIMG_JOB_H

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpSocket>
#include <QUrl>
#include <QDebug>

#include "protocol/request_jobs/job_base.h"
#include "protocol/request_jobs/img_type.h"
#include "protocol/qq_protocol.h"
#include "core/captchainfo.h"

class LoadImgJob : public __JobBase
{
	Q_OBJECT
public:
	LoadImgJob(QString file, ImgType img_type, JobType type = JT_LoadImg);

	virtual void run();
    virtual void getImgUrl();

    void reciveFile(QString file_url);

    static QString getRequestUrl(const QString &url);
    static QString getHost(const QString &url);

protected:
    QString request_url_;
    QString host_;
	QTcpSocket http_socket_;

    QString file_;
    ImgType img_type_;

private slots:
    void onRequestImgDone();
    void onGetImgUrlDone();

private:
    virtual void triggerEvent(const QByteArray &data) = 0;
};

class LoadOffpicJob: public LoadImgJob 
{
    Q_OBJECT
public:
    LoadOffpicJob(const QString &file_name, const QString &to_id, ImgType type = IMGT_FriendOffpic) :
        LoadImgJob(file_name, type),
        to_id_(to_id)
    {
        request_url_ = "http://d.web2.qq.com/channel/get_offpic2?file_path=" +file_name + "&f_uin=" + to_id + "&clientid=5412354841&psessionid="+
            CaptchaInfo::instance()->psessionid();
        host_ = "d.web2.qq.com";   
    }
    
private:
    virtual void triggerEvent(const QByteArray &data);

private:
    QString to_id_;
};

class LoadCfaceJob : public LoadImgJob
{
    Q_OBJECT
public:
    LoadCfaceJob(const QString &file_name, const QString &to_id, const QString &msg_id, ImgType type = IMGT_FriendCface) :
        LoadImgJob(file_name, type),
        to_id_(to_id)
    {
        request_url_ = "http://d.web2.qq.com/channel/get_cface2?lcid=" + msg_id+ "&guid=" + file_name + "&to=" + to_id + "&count=5&time=1&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();
        host_ = "d.web2.qq.com";
    }

private:
    virtual void triggerEvent(const QByteArray &data);

private:
    QString to_id_;
};

class LoadGroupImgJob : public LoadImgJob
{
    Q_OBJECT
public:
    LoadGroupImgJob(QString gid, const QString &file_name, QString uin, const QString &gcode, QString fid, QString rip, QString rport, const QString &time, ImgType type = IMGT_GroupImg) :
        LoadImgJob(file_name, type),
        gid_(gid)
    {
        request_url_ = "http://web.qq.com/cgi-bin/get_group_pic?type=0&gid=" + gcode + "&uin=" + uin + "&rip=" +rip + "&rport=" + rport + "&fid=" + fid + "&pic=" + file_name + "&vfwebqq="+ CaptchaInfo::instance()->vfwebqq() + "&t="+time;
        host_ = "web.qq.com";
    }

private slots:
    void onGetImgDone(bool err)
    {
        QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

        QByteArray data = reply->readAll();
        qDebug() << "Get Group Image response:\n"
            << reply->rawHeaderList() << "\ndata: " << data << endl;
        /*
        if ( response.statusCode() == 200 )
        {
            triggerEvent(data);
            return;
        }
        */

        QString location = reply->rawHeader("Location");
        qDebug() << location << endl;
        disconnect(reply, SIGNAL(finished()), this, SLOT(onGetImgDone()));
        reply->deleteLater();

        QNetworkRequest request(getRequestUrl(location));
        QString host = getHost(location);
        request.setRawHeader("Host", host.toLatin1());
        request.setRawHeader("Referer", "http://web.qq.com");
        request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

        qDebug() << "Image request:\n"
            << request.rawHeaderList() << endl;

        reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(onRedirectDone()));
    }

    void onRedirectDone(bool err)
    {
        QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
        disconnect(reply, SIGNAL(done(bool)), this, SLOT(onRedirectDone()));
        qDebug() << "Get Group Image redirect response header:\n"
            << reply->rawHeaderList() << endl;

        QByteArray data = reply->readAll();
        triggerEvent(data);
    }

private:
    virtual void run()
    {
        getImg();
    }

    virtual void triggerEvent(const QByteArray &data);

    virtual void getImg()
    {
        QNetworkRequest request(request_url_);
        request.setRawHeader("Host", host_.toLatin1());
        request.setRawHeader("Referer", "http://web.qq.com");
        request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

        qDebug() << "Image request:\n"
            << request.rawHeaderList() << endl;

        QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(onGetImgUrlDone()));
    }

private:
    QString gid_;
};

#endif //LOADIMG_JOB_H
