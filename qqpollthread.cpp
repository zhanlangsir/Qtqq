#include "qqpollthread.h"

//#include <QHttp>

void QQPollThread::run()
{
    while (true)
    {
        poll_fd_ = new QTcpSocket();
        poll_fd_->connectToHost("d.web2.qq.com", 80);
        poll_fd_->write(req_.toByteArray());

        QByteArray result;
        while(poll_fd_->waitForReadyRead(45000))
        {
            result.append(poll_fd_->readAll()); 
        }

        if (result.isEmpty())
            continue;

        int retcode_fir_idx = result.indexOf("retcode") + 9;
        int retcode_sec_idx = result.indexOf(",", retcode_fir_idx);
        int retcode = result.mid(retcode_fir_idx, retcode_sec_idx - retcode_fir_idx).toInt();

        if (retcode == 116)
        {
            int ptwebqq_fir_idx = result.indexOf("p", retcode_sec_idx) + 3;
            int ptwebqq_sec_idx = result.indexOf("\"", ptwebqq_fir_idx);
            QString ptwebqq = result.mid(ptwebqq_fir_idx, ptwebqq_sec_idx - ptwebqq_fir_idx);
            captcha_info_.vfwebqq_ = ptwebqq;

        }
        message_queue_->enqueue(result);
        poll_fd_->close();
        delete poll_fd_;
        semaphore_->release();
    }
}

QQPollThread::QQPollThread(CaptchaInfo captcha_info, QQueue<QByteArray> *message_queue,
                           QSemaphore *semaphore) : 
    captcha_info_(captcha_info),
    semaphore_(semaphore),
    message_queue_(message_queue)
{
    QString poll_path = "/channel/poll2";
    msg_ = "r={\"clientid\":\"5412354841\",\"psessionid\":\"" + captcha_info_.psessionid_.toAscii() + "\","
        "\"key\":0,\"ids\":[]}&clientid=" + "5412354841" + "&psessionid=" + captcha_info_.psessionid_.toAscii();

    req_.create(kPost, poll_path);
    req_.addHeaderItem("Host", "d.web2.qq.com");
    req_.addHeaderItem("Cookie", captcha_info_.cookie_);
    req_.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html");
    req_.addHeaderItem("Content-Length", QString::number(msg_.length()));
    req_.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
    req_.addRequestContent(msg_);
}
