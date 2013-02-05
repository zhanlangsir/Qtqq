#ifndef NOTIFYTEXTBROWSER_H
#define NOTIFYTEXTBROWSER_H

#include <QTextBrowser>

#include "animatedtextbrowser.h"

class NotifyTextBrowser : 
	public AnimatedTextBrowser
{
	Q_OBJECT;
public:
	NotifyTextBrowser(QWidget *AParent);
	~NotifyTextBrowser();
	void setMaxHeight(int AMax);
public:
	virtual void mouseReleaseEvent(QMouseEvent *AEvent);
protected slots:
	void onTextChanged();
private:
	int FMaxHeight;
};

#endif // NOTIFYTEXTBROWSER_H
