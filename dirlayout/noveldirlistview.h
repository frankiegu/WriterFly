#ifndef NOVELDIR_H
#define NOVELDIR_H

#include <QWidget>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QAction>
#include <QLineEdit>
#include <QMimeData>
#include <QDrag>
#include <QApplication>
#include <QTimer>
#include <QPixmap>
#include <QKeyEvent>
#include <QFileDialog>
#include <QScrollBar>
#include "defines.h"
#include "fileutil.h"
#include "stringutil.h"
#include "globalvar.h"
#include "noveldirmodel.h"
#include "noveldirdelegate.h"
#include "noveldiritem.h"
#include "noveldirsettings.h"
#include "stackwidgetanimation.h"
#include "dirsettingswidget.h"

class NovelDirListView : public QListView
{
    Q_OBJECT
public:
    NovelDirListView(QWidget *parent);

    void initStyle();
    void initRecent();
    void initMenu();
    void initRowHidden();
    void initRowHidden(QModelIndex index);

    void addRoll(QString newName);
    void addChapter(int index, QString newName);

    NovelDirModel* getModel();
    NovelDirDelegate* getDelegate();
    QString getNovelName();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyboardSearch(const QString &search);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void currentChange(const QModelIndex &current, const QModelIndex &previous);
    void contextMenuEvent(QContextMenuEvent *event);
    bool exportNovel(QString path);
    QString simplifyChapter(QString chpt);

private:
    bool isBlankChar(QString c);

signals:
    void signalCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void signalOpenChapter(QString novel_name, QString chapterName, QString fullChapterName);
    void signalOpenAnimationStartPos(QPoint point);

    void signalAddItemRect(QRect rect, QPixmap pixmap);
    void signalDeleteChapter(QString novel_name, QString chapterName);
    void signalDeleteChapters(QString novel_name, QStringList chapterNames);

    void signalExportFinished();
    void signalImportFinished(QString novel_name);

    void signalNeedLineGuide();

public slots:
    void readNovel(QString novel_name); // Delegate 读取 settings，Model 读取 ChapterList
    void slotOpenChapter(int index);
    void slotOpenCurrentChapter();
    void slotOpenChapter(QString name);

    void slotInsertRoll();
    void slotDeleteRoll();
    void slotRenameRoll();

    void slotInsertChapter();
    void slotDeleteChapter();
    void slotRenameChapter();

    void slotAddRoll();
    void slotAddChapter();
    void slotInitRowHidden();

    void slotExternRect(QRect r);
    void slotExternPoint(QPoint p);
    void slotAddAnimationFinished(int index);

    void slotReedit();

    void emitItemEvent(int index);
    void slotExport();
    void slotImport(QString file_path, bool split_roll, int index);
    void slotDirSettings();

private:
    NovelDirModel *novel_dir_model;
    NovelDirDelegate *novel_dir_delegate;
    QList<NovelDirItem> roll_position_list; // 记录分组的位置

    QMenu *blank_menu;
    QMenu *roll_menu;
    QMenu *chapter_menu;

    NovelDirItem *current_item;
    QPoint drag_start_pos, drag_end_pos;
    QRect extern_rect;
    QPoint extern_point;
    int pre_index;
    qint64 pressed_time1, pressed_time2;
    qint64 released_time;

    QString novel_name;
};

#endif // NOVELDIR_H
