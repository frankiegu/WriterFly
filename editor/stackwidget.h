#ifndef MYTABWIDGET_H
#define MYTABWIDGET_H

#include <QTabWidget>
#include <noveleditor.h>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QList>
#include <QObject>
#include <QGraphicsDropShadowEffect>
#include <QPalette>
#include <QTimer>
#include <QDateTime>
#include "stackwidgetanimation.h"
#include "noveleditinginfo.h"
#include "globalvar.h"
#include "defines.h"
#include "zoomgeometryanimationwidget.h"

class StackWidget : public QStackedWidget
{
    Q_OBJECT
public:
    StackWidget(QWidget *parent);

public:
    void switchEditor(int index);
    void createEditor(QString novelName, QString chapterName, QString fullChapterName);

protected:
    void resizeEvent(QResizeEvent *);

signals:
    void signalChapterClosed(QString novelName, QString chapterName);
    void signalChangeTitleByChapter(QString title);
    void signalEditorWordCount(int x);
    void signalSetTitleNovel(QString s);
    void signalSetTitleChapter(QString s);
    void signalSetTitleFull(QString s);
    // void signalSetTitleWc(int x);
    void signalOpenNewEditor(QPoint p);

public slots:
    void slotRenameNovel(QString oldName, QString newName);
    void slotDeleteNovel(QString novelName);
    void slotOpenChapter(QString novelName, QString chapterName, QString fullChapterName);
    void slotOpenAnimation(QPoint point);
    void slotOpenAnimationFinished(int x);
    void slotDeleteChapter(QString novelName, QString chapterName);
    void slotDeleteChapters(QString novelName, QStringList chapterNames);
    void slotChangeName(bool isChapter, QString novelName, QString oldName, QString newName);
    void slotCloseEditor(int index);
    void slotTextChanged();
    void slotUpdateUI();
    void slotUpdateAllEditor();
    void slotEditorBottomSettingsChanged();
    void slotEditorWordCount(int x);
    void slotSavePosition();
    void slotResotrePosition();
    void slotDelayFocus();
    void slotDelayScroll();

private:
    QWidget* parent;
    QList<NovelEditor*>editors;
    bool flag_isNew; // 是否是本次第一次打开这个章节，用来显示打开动画
    int flag_last;   // 上次（其实是本次）打开的图片
    int flag_aim;    //
    QPixmap* open_pixmap; // 打开的图片，切换完毕消失
};

#endif // MYTABWIDGET_H
