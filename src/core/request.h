#ifndef CORE_REQUEST_H
#define CORE_REQUEST_H

#include <QString>
#include <QByteArray>

enum RequestModes { kPost, kGet };

class Request
{
public:
    Request();
    ~Request();

public:
    void addHeaderItem(QString key, QString value);
    void addRequestContent(QByteArray content);
    void create(RequestModes mode, QString url);

    void clear();
    QByteArray toByteArray();

private:
    void addDefaultHeader();

private:
    QByteArray req_str_; //The real request String;
    bool has_content_;
};

#endif //CORE_REQUEST
