#include "pollthread.h"

#include <QTcpSocket>

#include "core/captchainfo.h"

Protocol::PollThread::PollThread(QObject *parent) :
	QThread(parent),
	is_stop_(true)
{
    QString poll_path = "/channel/poll2";
    QByteArray msg = "r={\"clientid\":\"5412354841\",\"psessionid\":\"" + CaptchaInfo::instance()->psessionid().toAscii() + "\","
        "\"key\":0,\"ids\":[]}&clientid=" + "5412354841" + "&psessionid=" + CaptchaInfo::instance()->psessionid().toAscii();

    req_.create(kPost, poll_path);
    req_.addHeaderItem("Host", "d.web2.qq.com");
    req_.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
    req_.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html");
    req_.addHeaderItem("Content-Length", QString::number(msg.length()));
    req_.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
    req_.addRequestContent(msg);

    setTerminationEnabled();
}

void Protocol::PollThread::run()
{
    QTcpSocket fd;
	is_stop_ = false;

    while ( !is_stop_ )
    {
        fd.connectToHost("d.web2.qq.com", 80);
        fd.write(req_.toByteArray());

        QByteArray result;
        while(fd.waitForReadyRead(45000))
        {
            result.append(fd.readAll());
        }

        int retcode_idx = result.indexOf("retcode") + 9;
        int retcode_end_idx = result.indexOf(",", retcode_idx);
        int retcode = result.mid(retcode_idx, retcode_end_idx - retcode_idx).toInt();

        if (retcode == 116)
        {
            int ptwebqq_idx = result.indexOf("p", retcode_end_idx) + 4;
            int ptwebqq_end_idx = result.indexOf("\"", ptwebqq_idx);
            QString ptwebqq = result.mid(ptwebqq_idx, ptwebqq_end_idx - ptwebqq_idx);
            //CaptchaInfo::singleton()->set_vfwebqq(ptwebqq);
        }

        emit newMsgArrive(result);

        result.clear();
        fd.abort();
        fd.close();
    }
}


void Protocol::PollThread::stop()
{
	is_stop_ = true;
}
