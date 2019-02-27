#ifndef NOVELDIRLAYOUT_H
#define NOVELDIRLAYOUT_H

#include <QWidget>
#include <QPushButton>
#include <QListView>
#include <QVBoxLayout>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QTextStream>
#include <QCursor>
#include <QPropertyAnimation>
#include "defines.h"
#include "noveldirlistview.h"
#include "anifbutton.h"
#include "splitterwidget.h"
#include "settings.h"

class NovelDirGroup : public QWidget
{
    Q_OBJECT
public:
    NovelDirGroup(QWidget *parent = nullptr);
    ~NovelDirGroup();

public:
    void startShowSidebar(int distance);
    void startHideSidebar(int distance);
    bool isPacking();
    bool isAnimating();
    NovelDirListView *getListView();

protected:
    void paintEvent(QPaintEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void resizeEvent(QResizeEvent *event);

private:
    void initLayout();
    void initStyleSheet();
    void initDir();
    void initRecent();

public:
    void ensureDir(QString path); // 确保文件夹一定存在

signals:
    void signalShowInfo(QString novelName, QPoint pos);
    void signalRenameNovel(QString oldName, QString newName);
    void signalDeleteNovel(QString novelName);

public slots:
    void switchBook(int index);
    void createBook();
    void slotRenameNovel();
    void slotDeleteNovel();
    void slotNeedLineGuide();

    void slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void slotFixedWidthChanged(int x);

    void slotInfoClicked();
    void slotAddRollButtonClicked();
    void slotAddChapterButtonClicked();

    void slotImportFinished(QString name);

private:
    QComboBox *book_switch_combo;
    NovelDirListView *novel_dir_listview;
    AniFButton *add_roll_button;
    AniFButton *add_chapter_button;
    AniFButton *info_button;
    QHBoxLayout *btn_layout;
    SplitterWidget* splitter_widget;

    int fixed_max_width; // 最大值
    bool is_packing; // 目录是否折叠起来
    bool is_animating;
    QString novel_new_name_temp;
    bool novel_had_deleted_temp;
};

#endif // NOVELDIRECTORYLAYOUT_H
