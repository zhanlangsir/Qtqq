#ifndef ANIMATEDTEXTBROWSER_H
#define ANIMATEDTEXTBROWSER_H

#include <QSet>
#include <QPair>
#include <QMovie>
#include <QTimer>
#include <QDateTime>
#include <QTextBrowser>
#include <QNetworkAccessManager>

class AnimatedTextBrowser : 
	public QTextBrowser
{
	Q_OBJECT;
public:
	AnimatedTextBrowser(QWidget *AParent = NULL);
	bool isAnimated() const;
	void setAnimated(bool AAnimated);
	QPair<int,int> visiblePositionBoundary() const;
	QNetworkAccessManager *networkAccessManager() const;
	void setNetworkAccessManager(QNetworkAccessManager *ANetworkAccessManager);
signals:
	void visiblePositionBoundaryChanged();
	void resourceLoaded(const QUrl &AName);
	void resourceUpdated(const QUrl &AName);
protected:
	QList<int> findUrlPositions(const QUrl &AName) const;
	QPixmap addAnimation(const QUrl &AName, const QVariant &AImageData);
protected:
	virtual void showEvent(QShowEvent *AEvent);
	virtual QVariant loadResource(int AType, const QUrl &AName);
protected slots:
	void onAnimationFrameChanged();
	void onResourceLoadFinished();
	void onUpdateDocumentAnimation();
	void onVerticalScrollBarChanged();
	void onMovieDestroyed(QObject *AObject);
	void onDocumentContentsChanged(int APosition, int ARemoved, int AAdded);
private:
	bool FAnimated;
	QTimer FUpdateTimer;
	QDateTime FLastUpdate;
	mutable bool FBoundaryChanged;
	mutable QPair<int,int> FBoundary;
	QSet<QMovie *> FChangedMovies;
	QHash<QMovie *, QUrl> FUrls;
	QHash<QUrl, QMovie *> FUrlMovies;
	QHash<QMovie *, QList<int> > FUrlPositions;
	QHash<QString, QVariant> FResources;
	QNetworkAccessManager *FNetworkAccessManager;
};

#endif // ANIMATEDTEXTBROWSER_H
