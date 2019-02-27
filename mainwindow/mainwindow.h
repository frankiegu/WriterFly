#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <basewindow.h>
#include <QAction>
#include <QLabel>
#include <QStatusBar>
#include <QProcess>
#include <QUrl>
#include "applicationupdatethread.h"
#include "fileutil.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);

private:
    void initWindow();
    void initData();
    void initFirstOpen();
    void toastPlatform();

public slots:
    void slotHasNewVersion(QString version, QString url);

private:

};

#endif // MAINWINDOW_H
