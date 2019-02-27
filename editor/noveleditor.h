#ifndef NOVELEDITOR_H
#define NOVELEDITOR_H

#include <QWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <novelai.h>
#include <QPropertyAnimation>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QInputMethodEvent>
#include <QFont>
#include <QMimeData>
#include <QTextBlockFormat>
#include <QTextBlock>
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>
#include <QStringListModel>
#include <QListView>
#include <QClipboard>
#include <QApplication>
#include <QDesktopWidget>
#include "settings.h"
#include "defines.h"
#include "globalvar.h"
#include "fileutil.h"
#include "noveleditinginfo.h"
#include "editorrecorderitem.h"
#include "editorcursor.h"

// 用户最近使用的操作（结合 last_operator）
#define OPERATOR_NULL 0
#define OPERATOR_KEY 1
#define OPERATOR_MOUSE 2
#define OPERATOR_CN 3
#define OPERATOR_WHEEL 4 // 这个用不到
#define OPERATOR_NOT_COMPLETER 11 // 专门用来不提示的
#define OPERATOR_IMD_COMPLETER 12 // 专门用来马上提示

#define UNDOREDO_CASE_NONE 0
#define UNDOREDO_CASE_INPUTONE 1
#define UNDOREDO_CASE_BACKSPACEONE 2
#define UNDOREDO_CASE_DELETEONE 3


class NovelEditor : public QTextEdit, public NovelAI
{
    Q_OBJECT
public:
    NovelEditor(QWidget *parent);
    NovelEditor(QWidget *parent, QString novel_name, QString chapter_name, QString full_chapter_name);
    void initEditor();

    bool isSaved();
    void readChapter();
    bool save();
    NovelEditingInfo &getInfo();

    void surroundWordSearch();
    void closeCompleteTimerOnce();

signals:
    void signalWordCount(int x);
    void signalUpdateAllEditor();

public slots:
    void openChapter(QString novel_name, QString chapter_name, QString full_chapter_name); // 单纯的打开章节
    void beforeClose();
    void textChange();   // 文本改变槽
    void cursorChange(); // 光标位置改变槽
    void copyAva();
    void cursorChangedByUser();
    void scrollChange();
    void resetScrollBarRange();

    void initUndoRedo();
    void recorderOperator();
    void undoOperator();
    void redoOperator();
    void readRecorder(int x);
    void updateRecorderPosition(int x);
    void updateRecorderScroll();
    void updateRecorderSelection();

    void updateUI();
    void updateWordCount();
    void slotEditorBottomSettingsChanged();
    void updateEditorMarked();
    void updateEditor();

    void slotCompleterShowed();
    void slotOnCompleterActived(const QString& completion);

protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void inputMethodEvent(QInputMethodEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent* e);
    void showEvent(QShowEvent* e);
    void resizeEvent(QResizeEvent* e);
    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);
    void insertFromMimeData(const QMimeData *source);
    void cursorRealChanged(int pos = -1);

    void cutRecorderPosition();

    void setTextInputColor();

private:
    void startOpenFile(); // 重新打开章节期间，文本会改变，但是不保存
    void endOpenFIle();
    void delayCompleter(int ms);
    void highlightCurrentLine();
    void scrollToEditEnd();
    void scrollCursorFix();

protected:
    NovelEditingInfo editing;

private:
    // 撤销重做
    QList<EditorRecorderItem> recorders;
    int recorders_pos; //  索引从1开始
    int last_operator, completer_case;
    bool changed_by_redo_undo;

    // 按键交互
    bool ctrling, alting, shifting;
    QTimer* mouse_press_timer, *completer_timer;
    bool mouse_press_timer_timeout, completer_timer_timeout;
    bool focusing;
    QTimer* document_height_timer;

    // 自动补全
    QCompleter* completer;
    QListView* completer_listview;

    // 光标行固定
    int editor_height, document_height;
    int viewport_margin_bottom;
    int pre_scrollbar_max_range;     // 上一次的最大值，判断有没有改变高度

    // 各种标识
    bool _flag_dont_save; // 本次修改是否不保存：打开新章导致文本改变
    bool _flag_is_not_range_changed; // rangeChanged 事件中，避免死循环
    bool _flag_should_update;
    bool _flag_user_change_cursor;

    // 动画
    EditorCursor* editor_cursor;

};

#endif // NOVELEDITOR_H
