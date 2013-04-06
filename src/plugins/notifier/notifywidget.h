#ifndef NOTIFYWIDGET_H
#define NOTIFYWIDGET_H

#include <QMouseEvent>
#include <QTimer>

#include <QDesktopWidget>
#include "ui_notifywidget.h"

enum NotificationType
{ 
    NFT_Chat, 
    NFT_GroupChat, 
    NFT_SessChat,
    NFT_StatusChanged 
}; 

struct Notification 
{ 
    NotificationType type; 

    QString id;
    QString title;
    QString sender_name;
    QString content;
    int ms_timeout;
 
    QPixmap icon; 
    QVariant data;
};

class NotifyWidget :
			public QWidget
{
	Q_OBJECT;
public:
	NotifyWidget(const Notification &notification);
	~NotifyWidget();
	void appear();
	void animateTo(int AYPos);
	void setAnimated(bool AAnimated);
    void appendMessage(const QString &sender_name, const QString &msg);
    QString id() const
    { return id_; }

signals:
	void notifyActivated();
	void notifyRemoved();
	void windowDestroyed();

protected:
	virtual void resizeEvent(QResizeEvent *AEvent);
	virtual void mouseReleaseEvent(QMouseEvent *AEvent);
    virtual void enterEvent(QEvent *e);

protected slots:
	void onAnimateStep();
	void adjustHeight();
	void updateElidedText();

private:
	Ui::NotifyWidgetClass ui;

private:
	int FYPos;
	int ms_timeout_;
	int FAnimateStep;
    double window_opacity_;

    NotificationType type_;
    QString id_;
	QString title_;
	QString caption_;

    QTimer close_timer_;
    QTimer animate_timer_;

private:
	static void layoutWidgets();
	static QList<NotifyWidget *> FWidgets;
	static QRect FDisplay;
};

#endif // NOTIFYWIDGET_H
