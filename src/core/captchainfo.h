#ifndef CAPTCHAINFO_H
#define CAPTCHAINFO_H

#include <QString>

class CaptchaInfo
{
public:
    static CaptchaInfo* instance()
    {
        if (!instance_)
            instance_ = new CaptchaInfo();
        return instance_;
    }

public:
    QString vfwebqq() const
    { return vfwebqq_; }
    void setVfwebqq(QString vfwebqq)
    { vfwebqq_ = vfwebqq; }

    QString psessionid() const
    { return psessionid_; }
    void setPsessionid(QString psessionid)
    { psessionid_ = psessionid; }

    QString skey() const
    { return skey_; }
    void setSkey(QString skey)
    { skey_ = skey; }

    QString ptwebqq() const
    { return ptwebqq_; }
    void setPtwebqq(const QString &ptwebqq) 
    {
        ptwebqq_ = ptwebqq;
    }

    QString cookie() const
    { return cookie_; }
    void setCookie(QString cookie)
    { cookie_= cookie; }

    int index() const
    { return index_; }
    void setIndex(int index) 
    { index_ = index; }

    int port() const
    { return port_; }
    void setPort(int port)
    { port_ = port; } 

protected:
    CaptchaInfo() {}

private:
    static CaptchaInfo* instance_;

    QString vfwebqq_;
    QString psessionid_;
    QString ptwebqq_;
    QString skey_;
    QString cookie_;
    int index_;
    int port_;
};

#endif // CAPTCHAINFO_H
