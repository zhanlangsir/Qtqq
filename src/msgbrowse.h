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
signals:
    void linkClicked(const QUrl &url);
    void imageDoubleClicked(QString img_src);

public:
    enum MsgKind {kWord, kImg, kDateSeprate, kStatus, kError};
    explicit MsgBrowse(QWidget *parent = 0);
    void appendContent(QString content,const ShowOptions &options);
    void appendHtml(const QString &content, const ShowOptions &options);
    void replaceIdToName(const QString &id, const QString &name);

    int getStyleWindowWidth();
    int getStyleWindowHeight();

public slots:
    void replaceRealImg(const QString &id, const QString &local_path);

protected slots:
    void onImageDoubleClicked(QString img_src);

private slots:
    void onLinkClicked(const QUrl &url);

private:
    bool ifCombineMsg(const ShowOptions &options) const;
    void ecapseForScript(QString &html) const;
    QString loadFileData(QString name);
    void loadTemplate();
    void initUi();
    QString scriptForAppendContent(bool isCombine);
    void replaceKeyWord(QString &html, const ShowOptions &options);
    void converLink(QString &content);
    
private:
    QString last_send_id_;
    MsgKind last_kind_;

    static QString in_content_html_;
    static QString out_content_html_;
    static QString in_next_content_html_;
    static QString out_next_content_html_;
    static QString status_html_;
};

struct ShowOptions
{
    MsgBrowse::MsgKind type;
    QDateTime time;
    QString sender_uin;
    QString sender_name;
    QString sender_avatar_path;

    bool is_msg_in;
};

#endif // MSGBROWSE_H
