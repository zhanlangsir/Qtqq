#include "sendfile_job.h"

#include <QHttpRequestHeader>
#include <QHttpResponseHeader>
#include <QDateTime>
#include <QTcpSocket>
#include <QDebug>

#include "core/talkable.h"
#include "core/captchainfo.h"
#include "core/curr_login_account.h"
#include "protocol/event.h"
#include "protocol/qq_protocol.h"
#include "protocol/event_center.h"
#include "protocol/filesender.h"

SendFileJob::SendFileJob(const QString &to_id,  const QString &file_path, const QByteArray &data, SendFileJobType filejob_type, JobType type) : 
	__JobBase(NULL, type),
	http_(this),
    filejob_type_(filejob_type),
    to_id_(to_id),
    file_path_(file_path),
    data_(data)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
    file_ = QFileInfo(file_path_).fileName();
}

void SendFileJob::requestDone(bool error)
{
	if ( !error )
	{
		QByteArray data = http_.readAll();
		http_.close();

        if ( filejob_type_ == kOffFile )
        {
            QString file_path = getFilePath(data);
            notifyServer(file_path);
        }
        qDebug() << "File send done response: " << http_.lastResponse().toString() << endl;
        qDebug() << "File send done body:\n" << data << endl;
        //Useless result, so no event
        //Protocol::Event *event = Protocol::EventCenter::instance()->createImgSendDoneEvent(for_, success, file_path_, img_id);
        //Protocol::EventCenter::instance()->triggerEvent(event);
	}
	else
	{
		qDebug() << "Send file failed " << endl;
		qDebug() << "Error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, error);
}

void SendFileJob::run()
{
    QString send_url;
    QString host;
    if ( filejob_type_ == kOffFile )
    {
        send_url = "/ftn_access/upload_offline_file?time=" + QString::number(QDateTime::currentMSecsSinceEpoch());
;
        host = "weboffline.ftn.qq.com";
    }
    else if ( filejob_type_ == kFile )
    {
        send_url = "/v2/" + CurrLoginAccount::id() + "/"+to_id_+"/"+ QString::number(QDateTime::currentMSecsSinceEpoch() % 4096) + "/"+ QString::number(CaptchaInfo::instance()->index()) + "/" + QString::number(CaptchaInfo::instance()->port()) + "/1/f/1/0/0?psessionid=" + CaptchaInfo::instance()->psessionid();
        host = "file1.web.qq.com";
    }

    QHttpRequestHeader header;

    header.setRequest("POST", send_url);
    header.addValue("Host", host);
    header.addValue("Connection", "keep-alive");
    header.addValue("Content-Length", QString::number(data_.length()));
    header.addValue("Cache-Control", "max-age=0");
    header.addValue("Origin", "http://web.qq.com");
    header.addValue("User-Agent", " User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.70 Safari/537.17");
    header.addValue("Content-Type", "multipart/form-data; boundary="+Protocol::QQProtocol::instance()->fileSender()->boundary());
    header.addValue("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    header.addValue("Referer", "http://web.qq.com/");
    header.addValue("Accept-Encoding", "gzip,deflate,sdch");
    header.addValue("Accept-Language", "zh-CN,zh;q=0.8");
    header.addValue("Accept-Charset", "UTF-8,*;q=0.5");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    http_.setHost(host);
    connect(&http_, SIGNAL(dataSendProgress(int, int)), this, SLOT(onDataSendProgress(int, int)));
    http_.request(header, data_);
}

void SendFileJob::onDataSendProgress(int done, int total)
{
    emit sendFileProgress(file_path_, done, total);
}

void SendFileJob::stop()
{
    http_.close();
    emit sigJobDone(this, false);
}

void SendFileJob::notifyServer(QString file_path)
{
    QString pse_id =  CaptchaInfo::instance()->psessionid();
    QByteArray data = "r={\"to\":\"" + to_id_.toAscii() + "\",\"file_path\":\"" + file_path.toAscii() + "\",\"filename\":\""+ file_.toAscii() +"\",\"to_uin\":\"" +to_id_.toAscii() + "\",\"clientid\":\"5412354841\",\"psessionid\":\"" + pse_id.toAscii() + "\"}&clientid=5412354841&psessionid=" + pse_id.toAscii();

    QString send_url = "/channel/send_offfile2";
    QString host = "d.web2.qq.com";

    QHttpRequestHeader header;
    header.setRequest("POST", send_url);
    header.addValue("Host", host);
    header.addValue("Pragma", "no-cache");
    header.addValue("Content-Length", QString::number(data.length()));
    header.addValue("Content-Type", "application/x-www-form-urlencoded");
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=3");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost(host, 80);
    fd.write(header.toString().toAscii() + data);

    fd.waitForReadyRead();
    qDebug() << fd.readAll() << endl;;
}

QString SendFileJob::getFilePath(const QByteArray &data)
{
    int path_s_idx = data.indexOf("filepath")+strlen("filepath")+3;
    int path_e_idx = data.indexOf('"', path_s_idx);
    QString file_path = data.mid(path_s_idx, path_e_idx-path_s_idx);

    return file_path;
}
