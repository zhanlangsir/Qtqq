#include "filerecive_job.h"

#include <QDateTime>
#include <QFile>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QHttpResponseHeader>
#include <QDebug>

#include "core/captchainfo.h"

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

	QHttpRequestHeader header;
	QString request_path = "/channel/get_file2?lcid=" + QString::number(session_id_) + "&guid=" + QUrl::toPercentEncoding(file_) + "&to=" + to_ + "&psessionid=" + CaptchaInfo::instance()->psessionid() + "&count=1&time=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) + "&clientid=5412354841";

	header.setRequest("GET", request_path);
	header.addValue("Host", "d.web2.qq.com");
	header.addValue("Referer", "http://web.qq.com");
	header.addValue("Cookie", CaptchaInfo::instance()->cookie());

	http_.setHost("d.web2.qq.com");

	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestLocationDone(bool)));

    qDebug() << "header" << header.toString() << endl;
	http_.request(header);
}

void FileReciveJob::requestLocationDone(bool err)
{
    QHttpResponseHeader response = http_.lastResponse();
    qDebug() << "Response: " <<  response.toString() << endl;

    QString location = response.value("Location");

	disconnect(&http_, SIGNAL(done(bool)), this, SLOT(requestLocationDone(bool)));

	QHttpRequestHeader header;

    int host_start_idx = QString("http://").length();
    int host_end_idx = location.indexOf(".com") + 4;
    QString host = location.mid(host_start_idx, host_end_idx - host_start_idx);
    qDebug() << "Host: " << host << endl;
    QString query = location.mid(host_end_idx);
    qDebug() << "Query: " << query << endl;

	header.setRequest("GET", QUrl::toPercentEncoding(query, "/?"));
	header.addValue("Host", host);
	header.addValue("Referer", "http://web.qq.com");
	header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    qDebug() << header.toString() << endl;
	
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

    save_to_ = new QFile(final_save_path); 
	http_.setHost(host);

	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestFileDone(bool)));
    connect(&http_, SIGNAL(dataReadProgress(int, int)), this, SLOT(onDataReadProgress(int, int)));
	http_.request(header, NULL, save_to_);
}

void FileReciveJob::requestFileDone(bool err)
{
    if ( !err )
    {
        save_to_->flush();
        save_to_->close();
        delete save_to_;

        qDebug() << http_.readAll() << endl;
    }
    else
        qDebug() << http_.errorString() << endl;
}

void FileReciveJob::onDataReadProgress(int done, int total)
{
    emit fileTransferProgress(session_id_, done, total);
}

void FileReciveJob::stop()
{
    http_.close();
    emit sigJobDone(this, false);
}
