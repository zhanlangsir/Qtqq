#include "filerecive_job.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDateTime>
#include <QFile>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "core/captchainfo.h"
#include "protocol/qq_protocol.h"

QString FileReciveJob::save_dir_ = QDir::homePath() + "/QtqqDownload";

FileReciveJob::FileReciveJob(int session_id, QString file, QString to, JobType type) :
    __JobBase(NULL, type),
    session_id_(session_id),
    file_(file),
    to_(to)
{

}

void FileReciveJob::run()
{
    /*GET /channel/get_file2?lcid=23703&guid=新建文本文档.zip&to=3058254526&psessionid=8368046764001d636f6e6e7365727665725f77656271714031302e3133392e372e313630000029f000001f44026e0400949222916d0000000a40477a65444f31336f746d00000028ae6b342d2dc71f208a03f8cb07364d75dd334fc5643deff4f5443eece81ba0c58057b3170635b0ff&count=1&time=1360053787228&clientid=56345388 HTTP/1.1*/

	QString request_path = "/channel/get_file2?lcid=" + QString::number(session_id_) + "&guid=" + QUrl::toPercentEncoding(file_) + "&to=" + to_ + "&psessionid=" + CaptchaInfo::instance()->psessionid() + "&count=1&time=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) + "&clientid=5412354841";

    QNetworkRequest request(request_path);

	request.setRawHeader("Host", "d.web2.qq.com");
	request.setRawHeader("Referer", "http://web.qq.com");
	request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(requestLocationDone()));
	//http_.setHost("d.web2.qq.com");

	//connect(&http_, SIGNAL(done(bool)), this, SLOT(requestLocationDone(bool)));
    

    qDebug() << "header" << request.rawHeaderList() << endl;
	//http_.request(header);
}

void FileReciveJob::requestLocationDone(bool err)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QString location = reply->rawHeader("Location");
    qDebug() << "Response: " <<  reply->rawHeaderList() << endl;


    reply->deleteLater();

    int host_start_idx = QString("http://").length();
    int host_end_idx = location.indexOf(".com") + 4;
    QString host = location.mid(host_start_idx, host_end_idx - host_start_idx);
    qDebug() << "Host: " << host << endl;
    QString query = location.mid(host_end_idx);
    qDebug() << "Query: " << query << endl;

    QNetworkRequest request(QString(QUrl::toPercentEncoding(query, "/?")));

	request.setRawHeader("Host", host.toLatin1());
	request.setRawHeader("Referer", "http://web.qq.com");
	request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

    qDebug() << request.rawHeaderList() << endl;
	
    reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(requestFileDone()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDataReadProgress(int, int)));
}

void FileReciveJob::requestFileDone(bool err)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if ( !reply->error() )
    {
        QDir save_dir(save_dir_);
        if ( !save_dir.exists() )
            save_dir.mkdir(save_dir_);

        QFileInfo save_info(save_dir_ + '/' + file_);
        QString basename = save_info.baseName();
        QString suffix = save_info.suffix();
        if ( !suffix.isEmpty() )
            suffix = '.' + suffix;

        int i = 1;
        QString base_path = save_dir_ + '/' + basename;
        QString final_save_path = base_path + suffix;
        while ( save_dir.exists(final_save_path) )
        {
            final_save_path = base_path + QString::number(i) + suffix;
            ++i;
        }

        QFile save_to(final_save_path);
        save_to.open(QIODevice::WriteOnly);
        save_to.write(reply->readAll());
        save_to.close();

        qDebug() << reply->readAll() << endl;
    }
    else
        qDebug() << reply->error() << endl;
}

void FileReciveJob::onDataReadProgress(int done, int total)
{
    emit fileTransferProgress(session_id_, done, total);
}

void FileReciveJob::stop()
{
    emit sigJobDone(this, false);
}
