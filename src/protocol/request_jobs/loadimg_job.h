#ifndef LOADIMG_JOB_H
#define LOADIMG_JOB_H

#include <QHttp>
#include <QUrl>
#include <QDebug>

#include "protocol/request_jobs/job_base.h"
#include "protocol/request_jobs/img_type.h"
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
	QHttp http_;

    QString file_;
    ImgType img_type_;

private slots:
    void onRequestImgDone(bool err);
    void onGetImgUrlDone(bool err);

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
        request_url_ = "/channel/get_offpic2?file_path=" +file_name + "&f_uin=" + to_id + "&clientid=5412354841&psessionid="+
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
        request_url_ = "/channel/get_cface2?lcid=" + msg_id+ "&guid=" + file_name + "&to=" + to_id + "&count=5&time=1&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();
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
        request_url_ = "/cgi-bin/get_group_pic?type=0&gid=" + gcode + "&uin=" + uin + "&rip=" +rip + "&rport=" + rport + "&fid=" + fid + "&pic=" + file_name + "&vfwebqq="+ CaptchaInfo::instance()->vfwebqq() + "&t="+time;
        host_ = "web.qq.com";
    }

private slots:
    void onRequestLocationDone(bool err)
    {
        QHttpResponseHeader response = http_.lastResponse(); 
        qDebug() << "Get Group Image response header:\n"
            << response.toString() << endl;

        QString location = response.value("Location");
        qDebug() << location << endl;
        disconnect(&http_, SIGNAL(done(bool)), this, SLOT(onRequestLocationDone(bool)));

        QString host = getHost(location);
        QHttpRequestHeader header;
        header.setRequest("GET", getRequestUrl(location));
        header.addValue("Host", host);
        header.addValue("Referer", "http://web.qq.com");
        header.addValue("Cookie", CaptchaInfo::instance()->cookie());

        qDebug() << "Image request:\n"
            << header.toString() << endl;

        http_.setHost(host);
        connect(&http_, SIGNAL(done(bool)), this, SLOT(onRedirectDone(bool)));
        http_.request(header);
    }

    void onRedirectDone(bool err)
    {
        disconnect(&http_, SIGNAL(done(bool)), this, SLOT(onRedirectDone(bool)));
        QHttpResponseHeader response = http_.lastResponse(); 
        qDebug() << "Get Group Image redirect response header:\n"
            << response.toString() << endl;

        QString location = response.value("Location");

        reciveFile(location);
    }

private:
    virtual void triggerEvent(const QByteArray &data);

    virtual void getImgUrl()
    {
        QHttpRequestHeader header;
        header.setRequest("GET", request_url_);
        header.addValue("Host", host_);
        header.addValue("Referer", "http://web.qq.com");
        header.addValue("Cookie", CaptchaInfo::instance()->cookie());

        qDebug() << "Image request:\n"
            << header.toString() << endl;

        http_.setHost(host_);
        connect(&http_, SIGNAL(done(bool)), this, SLOT(onRequestLocationDone(bool)));
        http_.request(header);
    }

private:
    QString gid_;
};

#endif //LOADIMG_JOB_H
