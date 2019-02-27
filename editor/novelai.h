#ifndef NOVELAI_H
#define NOVELAI_H

#include <QString>
#include <QTextCursor>
#include <QCursor>
#include <QTextEdit>
#include <QScrollBar>
#include <QRegExp>
#include <QStringList>
#include "novelaibase.h"
#include "pinyinutil.h"
#include "selectionitemlist.h"
#include "editorinputmanager.h"

#define INPUT_SHRESHOLD 20 // 超过20字则判断为粘贴

class NovelAI : public NovelAIBase
{
public:
    NovelAI();

    void initAnalyze(QTextEdit* edit);
    void textAnalyze();  // 文本改变时进行传值和初始化
    void isNotInput();   // 非用户输入标记
    void isNotMove();
    bool isMove();

    // 调用 NovelAIBase 的函数
    void activeSmartQuotes();
    void activeSmartQuotes2();
    void activeSmartSpace();
    void activeSmartEnter();
    void activeSmartBackspace();
    void activeNormalEnterInsertByFirstPara();
    void activeRetrevEnter();
    void activeNormalEnter();
    void activeSentFinish();
    void activeExpandSelection();
    void activeShrinkSelection();

    // 自身 AI 函数
    void advanceBackspace();
    void advanceDelete();
    void Typeset();
    void TypesetPaste(int x, QString text);
    void TypesetPart(int start, int end);
    bool activeParaSplit(int x);
    void activeWordReplace(int start, int end, QString word);
    void ExpandSelection(int start, int end, int &callback_start, int &callback_end);
    void ShrinkSelection(int start, int end, int &callback_start, int &callback_end);
    bool activeTabComplete();
    void activeTabSkip(int has_changed);
    void activeReverseTabSkip();
    bool activeHomonymCover();
    bool HomonymCover(int end_pos, int diff);
    bool PuncCover();      // 标点覆盖：输入特定标点触发
    bool PairMatch();      // 括号匹配：左半符号自动触发
    bool PairJump();       // 括号跳转：右半括号跳转到右边

    // 获取文本与光标事件
    int     getPreWordPos(int pos);                               // 上一个单词的位置
    int     getNextWordPos(int pos);                              // 下一个单词的位置
    int     getWordCount(QString text);                           // 字数统计
    int     getWordCount(QString text, int& ch, int& num, int& en, int& punc, int& blank, int& line);
    QString getWord(QString text, int pos, int& start, int& end); // 选择单词
    QString getSent(QString text, int pos, int& start, int& end); // 选择短句
    QString getSent2(QString text, int pos, int& start, int& end);// 选择长句
    QString getPair(QString text, int pos, int& start, int& end); // 选择成对符号（如果没有则返回空）
    QString getPara(QString text, int pos, int& start, int& end); // 选择段落

    // 自身光标处理事件
    void backspaceWord(int x);
    void deleteWord(int x);
    void moveToPreWord(int x);
    void moveToNextWord(int x);
    void moveToSentStart(int pos);
    void moveToSentEnd(int pos);
    void moveToParaStart(int x);
    void moveToParaStart2(int x); // 换行后面，或者0
    void moveToParaEnd(int x);
    void moveToParaPrevStart(int x);
    void moveToParaPrevEnd(int x);
    void moveToParaNextStart(int x);
    void moveToParaNextEnd(int x);
    void moveCursorFinished(int pos);

protected:
    virtual void recorderOperator();
    virtual void updateRecorderPosition(int x);
    virtual void updateRecorderScroll();
    virtual void updateRecorderSelection();
    virtual void cutRecorderPosition();
    void onlyDeleteText(int start, int end);
    virtual void scrollToEditEnd();
    virtual void scrollCursorFix();
    virtual void cursorRealChanged(int pos = -1);

    bool isAtEditEnd(); // 是否在编辑最后一行正文
    void makeInputAnimation();
    virtual void setTextInputColor(); // 设置输入动画的字体颜色

private:
    bool prepareAnalyze(); // 准备分析，预处理
    void finishAnalyze();  // 结束修改，设置新的文本或光标

    void moveCursor(int x);                 // 移动光标
    void insertText(int pos, QString text); // 插入文本
    void insertText(QString text);          // 插入文本
    void deleteText(int start, int end);    // 删除文本
    void setSelection(int start, int end, int pos = -1); // 设置光标

public:
    QTextEdit* _edit;
    QTextCursor _text_cursor;
    int _scroll_pos;
    bool _flag_is_not_move;
    bool _flag_is_not_scrolled;
    bool _change_text, _change_pos; // 标记是否改变
    bool is_editing_end, is_editing_end_backup;

    int pre_scroll_position, pre_scroll_bottom_delta;
    int pre_scroll_bottom_delta_by_analyze; // 文字改变前的位置，避免文字改变导致的变化
    int pre_scroll_viewpoer_top_by_analyze, pre_scroll_viewport_top;
    SelectionItemList selection_list;

    EditorInputManager* input_manager;
    bool _flag_color_changing; // 颜色改变期间无视任何 textchange
    bool _flag_is_cursor_change; // 是 setTextCursor 改变引起的 textChange 事件
    bool _flag_is_not_selection;
    int edit_range_start, edit_range_end;

private:
    int _pre_changed_pos; // 上一次改变的光标位置
    bool _flag_is_not_input;  // 非用户手动输入标记，如果为true，则本次不分析文本，并且置为false
};

#endif // NOVELAI_H
