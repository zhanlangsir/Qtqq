/*
 *  从vacuum抽出来的adium风格消息浏览器,有些东西是原项目中的,这里没用,先留着
 *
 */

#include "msgbrowse.h"

#include <QByteArray>
#include <QWebFrame>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QWebPage>
#include <QRegExp>
#include <QDesktopServices>

#include "core/qqsetting.h"
#include "core/captchainfo.h"

#define APPEND_MESSAGE_WITH_SCROLL          "checkIfScrollToBottomIsNeeded(); appendMessage(\"%1\"); scrollToBottomIfNeeded();"
#define APPEND_NEXT_MESSAGE_WITH_SCROLL     "checkIfScrollToBottomIsNeeded(); appendNextMessage(\"%1\"); scrollToBottomIfNeeded();"
#define APPEND_MESSAGE                      "appendMessage(\"%1\");"
#define APPEND_NEXT_MESSAGE                 "appendNextMessage(\"%1\");"
#define APPEND_MESSAGE_NO_SCROLL            "appendMessageNoScroll(\"%1\");"
#define APPEND_NEXT_MESSAGE_NO_SCROLL       "appendNextMessageNoScroll(\"%1\");"
#define REPLACE_LAST_MESSAGE                "replaceLastMessage(\"%1\");"
static const QString  kReplaceIdToName =    "replaceIdToName(\"%1\", \"%2\");";
static const QString  kReplaceRealImg  =    "replaceRealImg(\"%1\", \"%2\");";

#define TOPIC_MAIN_DIV	                    "<div id=\"topic\"></div>"
#define TOPIC_INDIVIDUAL_WRAPPER            "<span id=\"topicEdit\" ondblclick=\"this.setAttribute('contentEditable', true); this.focus();\">%1</span>"

MsgBrowse::MsgBrowse(QWidget *parent) :
    QWebView(parent),
    last_send_id_("")
{
    loadTemplate();

    initUi();

    setAcceptDrops(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    this->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(page(), SIGNAL(linkClicked(const QUrl &)), this, SLOT(onLinkClicked(const QUrl &)));
}

void MsgBrowse::onLinkClicked(const QUrl &url)
{
    QRegExp sender_reg("\\[(.*)\\]");
    if ( sender_reg.indexIn(url.toString()) != -1 )
    {
        emit senderLinkClicked(sender_reg.cap(1));
    }
    else
        QDesktopServices::openUrl(url);
}

void MsgBrowse::initUi()
{
      QString template_file = QQSettings::instance()->messageStylePath() + "/adium/Template.html";
      QFile fd(template_file);
      fd.open(QIODevice::ReadOnly);
      QByteArray file_content_arr = fd.readAll();

      QString html = QString::fromUtf8(file_content_arr.data(), file_content_arr.size());

      QString header = "";

      html.replace("%msg_style_path%", QUrl::fromLocalFile(QQSettings::instance()->messageStylePath()).toString());
      html.replace(html.indexOf("%@"), 2, header);
      html.replace(html.indexOf("%@"), 2, "");
      html.replace("==bodyBackground==", "margin-top: 5px; ");

      setHtml(html);

      page()->settings()->setFontSize(QWebSettings::DefaultFontSize, 7);
      page()->settings()->setFontFamily(QWebSettings::StandardFont,  "Monospace");
}

void MsgBrowse::appendContent(QString content, const ShowOptions &options)
{
    content.replace("\n", "<br>");

    if ( options.type != kImg )
    {
        QRegExp link_reg("((http|https)://|www\\.)[0-9A-Za-z:/\\.?=\\-_&{}#]*");
        QString a_templace = "<a href=\"";

        int pos = 0;
        while ( (pos = link_reg.indexIn(content, pos)) != -1 )
        {
            if ( link_reg.cap(2).isEmpty() )
                a_templace += "http://";

            QString after = a_templace  + link_reg.cap(0) + "\">" + link_reg.cap(0)+ "</a>";
            qDebug()<<link_reg.cap(0)<<endl;
            qDebug()<<link_reg.cap(0).length()<<endl;
            qDebug()<<after.length()<<endl;
            content.replace(link_reg.cap(0), after);
            qDebug()<<content.length()<<endl;

            pos += after.length();
        }
    }

    bool is_combine = ifCombineMsg(options);
    QString html;
    if ( options.type == kDateSeprate )
    {
        html = status_html_;
    }
    else if ( options.is_msg_in )
        html = is_combine ? in_next_content_html_ : in_content_html_;
    else
        html = is_combine ? out_next_content_html_ : out_content_html_;

    replaceKeyWord(html, options);
    html.replace("%message%", content);

    ecapseForScript(html);
    QString script = scriptForAppendContent(is_combine).arg(html);

    this->page()->mainFrame()->evaluateJavaScript(script);

    last_send_id_ = options.send_uin;
    last_kind_ = options.type;
}

void MsgBrowse::replaceIdToName(const QString &id, const QString &name)
{
    QString script = kReplaceIdToName.arg(id).arg(name);
    page()->mainFrame()->evaluateJavaScript(script);
}

void MsgBrowse::replaceRealImg(const QString &id, const QString &local_path)
{
    QString script = kReplaceRealImg.arg(id).arg(local_path);
    page()->mainFrame()->evaluateJavaScript(script);
}

QString MsgBrowse::scriptForAppendContent(bool is_combine)
{
    return is_combine ? APPEND_NEXT_MESSAGE: APPEND_MESSAGE ;
}

void MsgBrowse::replaceKeyWord(QString &html, const ShowOptions &options)
{
    html.replace("%senderStatusIcon%","null");
    html.replace("%shortTime%", "20:30");
    html.replace("%service%",QString::null);

    QString avatar = options.send_avatar_path;
    /*
    if (!QFile::exists(avatar))
    {
        avatar = isDirectionIn ? "Incoming/buddy_icon.png" : "Outgoing/buddy_icon.png";
        if (!isDirectionIn && !QFile::exists(avatar))
            avatar = "Incoming/buddy_icon.png";
    }
    */
    html.replace("%userIconPath%",avatar);
    html.replace("%sender%", options.send_name);
    html.replace("%sender_id%", options.send_uin);


    QString time =  options.time.toString("hh:mm:ss");
    html.replace("%time%", time);

    /*
    QRegExp timeRegExp("%time\\{([^}]*)\\}%");
    for (int pos=0; pos!=-1; pos = timeRegExp.indexIn(html, pos))
        if (!timeRegExp.cap(0).isEmpty())
            html.replace(pos, timeRegExp.cap(0).length(), time);

    QString sColor = !AOptions.senderColor.isEmpty() ? AOptions.senderColor : senderColor(AOptions.senderId);
    html.replace("%senderColor%",sColor);

    QRegExp scolorRegExp("%senderColor\\{([^}]*)\\}%");
    for (int pos=0; pos!=-1; pos = scolorRegExp.indexIn(html, pos))
        if (!scolorRegExp.cap(0).isEmpty())
            html.replace(pos, scolorRegExp.cap(0).length(), sColor);

    if (AOptions.kind == IMessageContentOptions::KindStatus)
    {
        html.replace("%status%",messageStatus);
        html.replace("%statusSender%",AOptions.senderName);
    }
    else
    {
        html.replace("%senderScreenName%",AOptions.senderId);
        html.replace("%sender%",AOptions.senderName);
        html.replace("%senderDisplayName%",AOptions.senderName);
        html.replace("%senderPrefix%",QString::null);

        QString rgbaColor;
        QColor bgColor(AOptions.textBGColor);
        QRegExp colorRegExp("%textbackgroundcolor\\{([^}]*)\\}%");
        for (int pos=0; pos!=-1; pos = colorRegExp.indexIn(html, pos))
        {
            if (!colorRegExp.cap(0).isEmpty())
            {
                if (bgColor.isValid())
                {
                    int r,g,b;
                    bool ok = false;
                    qreal a = colorRegExp.cap(1).toDouble(&ok);
                    bgColor.setAlphaF(ok ? a : 1.0);
                    bgColor.getRgb(&r,&g,&b);
                    rgbaColor = QString("rgba(%1, %2, %3, %4)").arg(r).arg(g).arg(b).arg(a);
                }
                else if (rgbaColor.isEmpty())
                {
                    rgbaColor = "inherit";
                }
                html.replace(pos, colorRegExp.cap(0).length(), rgbaColor);
            }
        }
    }
    */
}

inline
bool MsgBrowse::ifCombineMsg(const ShowOptions &options) const
{
    bool is_combine = false;
    if ( options.send_uin == last_send_id_ && options.type == last_kind_)
        is_combine = true;

    return is_combine;
}

void MsgBrowse::ecapseForScript(QString &html) const
{
    html.replace("\\","\\\\");
    html.replace("\"","\\\"");
    html.replace("\n","");
    html.replace("\r","<br>");
}

QString MsgBrowse::loadFileData(QString name)
{
    QFile fd(name);
    fd.open(QIODevice::ReadOnly);
    QByteArray file_content_arr = fd.readAll();

    return QString::fromUtf8(file_content_arr.data(), file_content_arr.size());
}

void MsgBrowse::loadTemplate()
{
    in_content_html_ =      loadFileData( QQSettings::instance()->messageStylePath() + "/adium/Incoming/Content.html");
    in_next_content_html_ =  loadFileData(QQSettings::instance()->messageStylePath()  + "/adium/Incoming/NextContent.html");

    out_content_html_ =     loadFileData(QQSettings::instance()->messageStylePath()  + "/adium/Outgoing/Content.html");
    out_next_content_html_ = loadFileData(QQSettings::instance()->messageStylePath()  + "/adium/Outgoing/NextContent.html");
    status_html_ = loadFileData(QQSettings::instance()->messageStylePath()  + "/adium/Status.html");
}
