#ifndef NOTIFYWIDGET_H
#define NOTIFYWIDGET_H

#include <QMouseEvent>
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
    QString title;
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

signals:
	void notifyActivated();
	void notifyRemoved();
	void windowDestroyed();
protected:
	virtual void resizeEvent(QResizeEvent *AEvent);
	virtual void mouseReleaseEvent(QMouseEvent *AEvent);
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
	QString title_;
	QString caption_;
private:
	static void layoutWidgets();
	static QList<NotifyWidget *> FWidgets;
	static QRect FDisplay;
};

#endif // NOTIFYWIDGET_H
