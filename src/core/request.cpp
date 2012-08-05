#include "request.h"

void Request::addDefaultHeader()
{
    addHeaderItem("User-Agent", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.3 Safari/535.19");
    addHeaderItem("Accept", "text/html, application/xml;q=0.9, "
                  "application/xhtml+xml, image/png, "
                  "image/jpeg, image/gif, "
                  "image/x-xbitmap, */*;q=0.1");
    addHeaderItem("Accept-Language", "en-US,zh-CN,zh;q=0.9,en;q=0.8");
    addHeaderItem("Accept-Charset", "GBK, utf-8, utf-16, *;q=0.1");
    addHeaderItem("Accept-Encoding", "deflate, gzip, x-gzip, identity, *;q=0");
}

void Request::addHeaderItem(QString key, QString value)
{
    req_str_ = req_str_ + key.toAscii() + ": " + value.toAscii() + "\r\n";
}

void Request::addRequestContent(QByteArray content)
{
    req_str_ = req_str_ + "\r\n" + content;
    has_content_ = true;
}

void Request::create(RequestModes mode, QString url)
{
    clear();
    switch(mode)
    {
    case kGet:
        req_str_ += "GET "; 
        break;
    case kPost:
        req_str_ += "POST ";
        break;
    defaule:
            break;
    }

    req_str_ = req_str_ + url.toAscii() + " HTTP/1.1\r\n";

    addDefaultHeader();
}

void Request::clear()
{
    req_str_.clear();
}

QByteArray Request::toByteArray()
{
    if (has_content_)
        return req_str_;

    return (req_str_+"\r\n");
}

Request::Request() : req_str_(""), has_content_(false)
{
}

Request::~Request()
{
}
