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
    void set_vfwebqq(QString vfwebqq)
    { vfwebqq_ = vfwebqq; }

    QString psessionid() const
    { return psessionid_; }
    void set_psessionid(QString psessionid)
    { psessionid_ = psessionid; }

    QString skey() const
    { return skey_; }
    void set_skey(QString skey)
    { skey_ = skey; }

    QString cookie() const
    { return cookie_; }
    void set_cookie(QString cookie)
    { cookie_= cookie; }

protected:
    CaptchaInfo() {}

private:
    static CaptchaInfo* instance_;

    QString vfwebqq_;
    QString psessionid_;
    QString skey_;
    QString cookie_;
};

#endif // CAPTCHAINFO_H
