#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QStatusBar>
#include "mytitlebar.h"
#include "stackwidget.h"
#include "noveldirgroup.h"
#include "noveleditor.h"
#include "splitterwidget.h"
#include "defines.h"
#include "globalvar.h"
#include "settings.h"
#include "dragsizewidget.h"
#include "usersettingswindow.h"
#include "transparentcontainer.h"
#include "frisothread.h"
#include "lexiconsthread.h"
#include "novelschedulewidget.h"
#include "im_ex_window.h"

class BaseWindow : public QMainWindow
{
    Q_OBJECT

public:
    BaseWindow(QMainWindow *parent = nullptr);
    ~BaseWindow();

    void setSidebarShowed(bool show);

protected:
    void initDataFiles();
    void initLayout();
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void closeEvent(QCloseEvent *);
    void showWindowAnimation();
    void closeWindowAnimation();
    void toWinMaxAnimation();
    void toWinMinAnimation();
    void toWinRestoreAnimation();

public slots:
    //void slotMainSplitterMoved(int x);
    void onButtonWinMinClicked();
    void onButtonWinRestoreClicked();
    void onButtonWinMaxClicked();
    void onButtonWinCloseClicked();
    void onButtonSidebarClicked();
    void onButtonSettingsClicked();
    void toWinMax();
    void toWinMin();
    void toWinRestore();

    void saveWinLayout(QPoint point, QSize size);
    void slotWindowSizeChanged(int dw, int dh);

    void slotSetWindowTitleBarContent(QString title);
    void slotSetTitleNovel(QString t);
    void slotSetTitleChapter(QString t);
    void slotSetTitleFull(QString t);
    // void slotSetTitleWc(int x);
    void slotChapterWordCount(int x);

    void slotTransContainerToClose(int kind);
    void slotTransContainerClosed(int kind);
    void slotShowNovelInfo(QString novel_name, QPoint point);
    void slotShowImExWindow(int kind, QString def_name);

protected:
    MyTitleBar* m_titlebar;
    QHBoxLayout *main_layout;
    NovelDirGroup *dir_layout;
    SplitterWidget* main_splitter;
    StackWidget *tab_widget;
    DragSizeWidget* drag_size_widget;
    UserSettingsWindow* user_settings_window;
    TransparentContainer* trans_ctn;
    QRect callback_rect;
    QPoint callback_point;
    NovelScheduleWidget* novel_schedule_widget;
    ImExWindow* im_ex_window;

private:
    //Settings win_settings;
    QString title_novel, title_chapter, title_full;
    int title_wc;
};

#endif // BASEWINDOW_H
