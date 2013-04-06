#include "notifywidget.h"

#include <QTimer>
#include <QScrollBar>
#include <Qt>

#define ANIMATE_STEPS             17
#define ANIMATE_TIME              700
#define ANIMATE_STEP_TIME         (ANIMATE_TIME/ANIMATE_STEPS)
#define ANIMATE_OPACITY_START     0.0
#define ANIMATE_OPACITY_END       0.9
#define ANIMATE_OPACITY_STEP      (ANIMATE_OPACITY_END - ANIMATE_OPACITY_START)/ANIMATE_STEPS

#define MAX_TEXT_LINES            5

QList<NotifyWidget *> NotifyWidget::FWidgets;

QRect NotifyWidget::FDisplay = QRect();

NotifyWidget::NotifyWidget(const Notification &notification)
#if defined(Q_OS_MAC)
	: QWidget(NULL, Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowStaysOnTopHint)
#else
	: QWidget(NULL, Qt::ToolTip|Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint)
#endif
{
	ui.setupUi(this);
	setFocusPolicy(Qt::NoFocus);
	setAttribute(Qt::WA_DeleteOnClose,true);
	setAttribute(Qt::WA_ShowWithoutActivating,true);

	QPalette pallete = ui.frmWindowFrame->palette();
	pallete.setColor(QPalette::Window, pallete.color(QPalette::Base));
	ui.frmWindowFrame->setPalette(pallete);
	ui.frmWindowFrame->setAutoFillBackground(true);
	ui.frmWindowFrame->setAttribute(Qt::WA_TransparentForMouseEvents,true);

	FYPos = -1;
	FAnimateStep = -1;
	ms_timeout_ = notification.ms_timeout;

    type_ = notification.type;
    id_ = notification.id;
	caption_ = tr("Notification");
	ui.lblCaption->setVisible(!caption_.isEmpty());

	title_ = notification.title;
	ui.lblTitle->setVisible(!title_.isEmpty());

	QIcon icon(notification.icon);
	if (!icon.isNull())
		ui.lblIcon->setPixmap(icon.pixmap(QSize(32,32)));
	else
		ui.lblIcon->setVisible(false);

    QString text;
    if ( notification.type == NFT_Chat || notification.type == NFT_SessChat )
    {
        text =  notification.content;
    }
    else
    {
        text = "<font color=\"blue\">" + notification.sender_name + "</font>: " +  notification.content;
    }

	if (!text.isEmpty())
    {
        ui.ntbText->setHtml(text);
        ui.ntbText->setContentsMargins(0,0,0,0);
        ui.ntbText->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        ui.ntbText->setMaxHeight(ui.ntbText->fontMetrics().height()*MAX_TEXT_LINES + (ui.ntbText->frameWidth() + qRound(ui.ntbText->document()->documentMargin()))*2);

        //QImage image = qvariant_cast<QImage>(ANotification.data.value(NDR_POPUP_IMAGE));
        QImage image;
        if (!image.isNull())
            ui.lblImage->setPixmap(QPixmap::fromImage(image.scaled(ui.lblImage->maximumSize(),Qt::KeepAspectRatio)));
        else
            ui.lblImage->setVisible(false);
    }
	else
	{
		ui.lblImage->setVisible(false);
		ui.ntbText->setVisible(false);
	}

	updateElidedText();
}

NotifyWidget::~NotifyWidget()
{
	FWidgets.removeAll(this);
	layoutWidgets();
	emit windowDestroyed();
}

void NotifyWidget::appear()
{
	if (!FWidgets.contains(this))
	{
		animate_timer_.setSingleShot(false);
		animate_timer_.setInterval(ANIMATE_STEP_TIME);
		animate_timer_.start();
		connect(&animate_timer_,SIGNAL(timeout()),SLOT(onAnimateStep()));

		if (ms_timeout_ > 0)
        {
            close_timer_.setSingleShot(true);
            connect(&close_timer_, SIGNAL(timeout()), this, SLOT(deleteLater()));
			close_timer_.start(ms_timeout_);
        }

		setWindowOpacity(ANIMATE_OPACITY_START);

		if (FWidgets.isEmpty())
        {
            int w = width();
            int h = height();
            int x = QApplication::desktop()->width() - w;
            int y = QApplication::desktop()->height() - h;
			FDisplay = QRect(x, y, w, h); 
        }
		FWidgets.prepend(this);
		layoutWidgets();
	}
}

void NotifyWidget::animateTo(int AYPos)
{
	if (FYPos != AYPos)
	{
		FYPos = AYPos;
		FAnimateStep = ANIMATE_STEPS;
	}
}

void NotifyWidget::setAnimated(bool AAnimated)
{
	ui.ntbText->setAnimated(AAnimated);
}

/*
void NotifyWidget::setMainWindow(IMainWindow *AMainWindow)
{
	FMainWindow = AMainWindow;
}
*/

void NotifyWidget::resizeEvent(QResizeEvent *AEvent)
{
	QWidget::resizeEvent(AEvent);
	ui.ntbText->verticalScrollBar()->setSliderPosition(0);
	updateElidedText();
	layoutWidgets();
}

void NotifyWidget::mouseReleaseEvent(QMouseEvent *AEvent)
{
	QWidget::mouseReleaseEvent(AEvent);
	if (AEvent->button() == Qt::LeftButton)
		emit notifyActivated();
	else if (AEvent->button() == Qt::RightButton)
		emit notifyRemoved();
}

void NotifyWidget::onAnimateStep()
{
	if (FAnimateStep > 0)
	{
		int ypos = y()+(FYPos-y())/(FAnimateStep);
		setWindowOpacity(qMin(windowOpacity()+ANIMATE_OPACITY_STEP, ANIMATE_OPACITY_END));
		move(x(),ypos);
		FAnimateStep--;
	}
	else if (FAnimateStep == 0)
	{
		move(x(),FYPos);
		setWindowOpacity(ANIMATE_OPACITY_END);
		FAnimateStep--;
	}
}

void NotifyWidget::layoutWidgets()
{
	int ypos = FDisplay.bottom();
	for (int i=0; ypos>0 && i<FWidgets.count(); i++)
	{
		NotifyWidget *widget = FWidgets.at(i);
		if (!widget->isVisible())
		{
			widget->show();
			widget->move(FDisplay.right() - widget->frameGeometry().width(), FDisplay.bottom());
			QTimer::singleShot(0,widget,SLOT(adjustHeight()));
			QTimer::singleShot(10,widget,SLOT(adjustHeight()));
		}
		ypos -=  widget->frameGeometry().height();
		widget->animateTo(ypos);
	}
}

void NotifyWidget::adjustHeight()
{
	resize(width(),sizeHint().height());
}

void NotifyWidget::updateElidedText()
{
	ui.lblCaption->setText(ui.lblCaption->fontMetrics().elidedText(caption_,Qt::ElideRight,ui.lblCaption->width() - ui.lblCaption->frameWidth()*2));
	ui.lblTitle->setText(ui.lblTitle->fontMetrics().elidedText(title_,Qt::ElideRight,ui.lblTitle->width() - ui.lblTitle->frameWidth()*2));
}

void NotifyWidget::appendMessage(const QString &sender_name, const QString &msg)
{
    if ( type_ == NFT_Chat || type_ == NFT_SessChat )
    {
        ui.ntbText->append(msg);
    }
    else
    {
        ui.ntbText->append("<font color=\"blue\">" + sender_name + "</font>: " + msg);
    }
    layoutWidgets();
    close_timer_.start(ms_timeout_);
}

void NotifyWidget::enterEvent(QEvent *e)
{
    hide();
}
