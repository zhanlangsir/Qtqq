#ifndef MSGBROWSE_H
#define MSGBROWSE_H

#include <QWebView>
#include <QDateTime>
#include <QUrl>

#include "core/qqmsg.h"

struct ShowOptions;

class MsgBrowse : public QWebView
{
    Q_OBJECT
public:
    enum MsgKind {kWord, kImg, kDateSeprate, kWorn, kError};
    explicit MsgBrowse(QWidget *parent = 0);
    
    void appendContent(QString content,const ShowOptions &options);
    void replaceIdToName(const QString &id, const QString &name);

public slots:
    void replaceRealImg(const QString &id, const QString &local_path);
    
private:
    bool     ifCombineMsg(const ShowOptions &options) const;
    void     ecapseForScript(QString &html) const;
    QString  loadFileData(QString name);
    void     loadTemplate();
    void     initUi();
    QString scriptForAppendContent(bool isCombine);
    void replaceKeyWord(QString &html, const ShowOptions &options);

private slots:
    void onLinkClicked(const QUrl &url);
    
private:
    QString last_send_id_;
    long last_time_;
    MsgKind last_kind_;

    QString style_path_;
    QString in_content_html_;
    QString out_content_html_;
    QString in_next_content_html_;
    QString out_next_content_html_;
    QString status_html_;
};

struct ShowOptions
{
    MsgBrowse::MsgKind type;
    QDateTime time;
    QString send_uin;
    QString send_name;
    QString send_avatar_path;

    bool is_msg_in;
};
#endif // MSGBROWSE_H
