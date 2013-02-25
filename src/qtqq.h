#ifndef QTQQ_QTQQ_H
#define QTQQ_QTQQ_H

#include <QObject>

#include "core/qqlogincore.h"

class LoginWin;
class MainWindow;
class QAction;

class Qtqq : public QObject
{
    Q_OBJECT
public:
    ~Qtqq();
	static Qtqq *instance()
	{
		if ( !instance_ )
			instance_ = new Qtqq();

		return instance_;
	}

	MainWindow *mainWin() const
	{ return main_win_; }

    void start();
    MainWindow *mainWindow() const
    { return main_win_; }


public slots:
    void aboutQtqq();

private slots:
    void showMainPanel();

	void onMinimize();
	void onRestore();
    void onLogout();
	void onQuit();

private:
    LoginWin *login_dlg_;
    MainWindow *main_win_;

	QAction *minimize_;
	QAction *restore_;
	//QAction *logout_;
	QAction *quit_;

private:
    Qtqq();
	Qtqq(const Qtqq &);
	Qtqq &operator=(const Qtqq &);

	static Qtqq *instance_;
};

#endif // QTQQ_QTQQ_H
