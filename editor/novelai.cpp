#include "novelai.h"

NovelAI::NovelAI()
{
    _text = "";
    _pos = 0;
    _pre_changed_pos = -1;
    _flag_is_not_scrolled = false;
    _change_text = _change_pos = false;

    input_manager = new EditorInputManager();
    _flag_is_cursor_change = false;
    _flag_is_not_selection = false; // 这两个暂时没什么用（可以删掉）
}

void NovelAI::initAnalyze(QTextEdit *edit)
{
    _edit = edit;
    _text = edit->toPlainText();
    _pos = edit->textCursor().position();
    _text_cursor = _edit->textCursor();
    input_manager->setEditor(edit);
    input_manager->setFlag(&_flag_is_cursor_change);

    _flag_is_not_input = false; // 默认是true，会导致第一次修改时不准备文本和光标，导致光标乱跳……
    _flag_is_not_move = false;
}

void NovelAI::isNotInput()
{
    _flag_is_not_input = true;
}

void NovelAI::isNotMove()
{
    _flag_is_not_move = true;
}

bool NovelAI::isMove()
{
    if (_flag_is_not_move)
    {
        _flag_is_not_move = false;
        return false;
    }
    else
        return true;
}

void NovelAI::activeSmartQuotes()
{
    prepareAnalyze();
    SmartQuotes();
    finishAnalyze();
}

void NovelAI::activeSmartQuotes2()
{
    prepareAnalyze();
    SmartQuotes2(_text_cursor.selectionStart(), _text_cursor.selectionEnd());
    finishAnalyze();
}

void NovelAI::activeSmartSpace()
{
    prepareAnalyze();
    SmartSpace();
    finishAnalyze();
}

void NovelAI::activeSmartEnter()
{
    prepareAnalyze();
    SmartEnter();
    finishAnalyze();
}

void NovelAI::activeSmartBackspace()
{
    prepareAnalyze();
    SmartBackspace();
    finishAnalyze();
}

void NovelAI::activeNormalEnterInsertByFirstPara()
{
    prepareAnalyze(); // _pos == 0

    QString ins_text = "";
    int blank_line = us->indent_line;
    if (_right1 == "\n") blank_line--;
    for (int i = 0; i < us->indent_blank; i++)
        ins_text += "　";
    int end_pos = ins_text.length();
    for (int i = 0; i <= blank_line; i++)
        ins_text += "\n";
    insertText(ins_text);
    moveCursor(end_pos-_pos);

    finishAnalyze();
}

void NovelAI::activeRetrevEnter()
{
    prepareAnalyze();

    QString ins_text = "";
    int blank_line = us->indent_line;
    //if (_right1 == "\n") blankLine--;
    for (int i = 0; i < us->indent_blank; i++)
        ins_text += "　";
    for (int i = 0; i <= blank_line; i++)
        ins_text += "\n";
    insertText(ins_text);
    moveCursor(-blank_line-1);

    finishAnalyze();
}

void NovelAI::activeNormalEnter()
{
    prepareAnalyze();

    QString ins_text = "";
    int blank_line = us->indent_line;
    if (_left1 == "\n" && _left2 != "\n") blank_line--;
    for (int i = 0; i <= blank_line; i++)
        ins_text += "\n";
    for (int i = 0; i < us->indent_blank; i++)
        ins_text += "　";
    insertText(ins_text);

    finishAnalyze();
}

bool NovelAI::activeTabComplete()
{
    int changed = false;
    prepareAnalyze();

    if (isSymPairRight(_right1) && _right1 != "”") // 成对分符号右边，例如 《成对|》，不进行标点填充
        return false;
    if (isSymPairLeft(_left1)) // 《|成对》
        return false;
    if (isSymPairLeft(_right1)) // |《成对》
        return false;

    changed = SentFinish();
    finishAnalyze();
    return changed;
}

void NovelAI::activeTabSkip(int has_changed)
{
    int len = _text.length();
    _pos = _edit->textCursor().position(); // 因为没有 prepareAnalyze()，所以光标位置不确定啊……

    // 最后一个位置，相当于回车
    if (_pos >= len)
    {
        if (us->smart_enter)
            activeSmartEnter();
        else
            activeNormalEnter();
        return ;
    }

    // 在全文首
    if (_pos == 0)
    {
        // 跳过空格
        if (_text.mid(_pos, 1) == " " || _text.mid(_pos, 1) == "　")
        {
            while (_text.mid(_pos, 1) == " " || _text.mid(_pos, 1) == "　")
                _pos++;
            moveCursorFinished(_pos);
        }
        // 自动缩进（除了空格以其它都缩进）
        else /*if (_text.mid(_pos, 1) == "\n")*/
        {
            prepareAnalyze();
            QString insText = "";
            for (int i = 0; i < us->indent_blank; i++)
                insText += "　";
            insertText(insText);
            finishAnalyze();
        }
        return ;
    }

    // pos < len && pos > 0
    QString l1 = _text.mid(_pos-1, 1);
    QString r1 = _text.mid(_pos, 1);

    // 成对符号右边（右引号、右括号）
    if (isSymPairRight(r1))
    {
        _pos++;
        moveCursorFinished(_pos);
        return ;
    }

    // 是否在一个段落开头的位置
    bool is_para_start = false;
    if (isBlankChar(l1))
    {
        int pos = _pos;
        while (pos > 0 && isBlankChar2(_text.mid(pos-1, 1)))
            pos--;
        if (pos == 0 || _text.mid(pos-1, 1) == "\n" || _text.mid(pos, 1) == "\r")
            is_para_start = true;
    }

    bool is_artic_end = false;
    is_artic_end = isAtEditEnd();

    // 段首自动调整缩进
    if (is_para_start && !is_artic_end)
    {
        prepareAnalyze();
        QString old_text = _text;
        int old_pos = _pos;
        // 删除原有的空白符号
        int start_pos = _pos, end_pos = _pos;
        while (start_pos > 0 && isBlankChar2(_text.mid(start_pos-1, 1)))
            start_pos--;
        while (end_pos < len && isBlankChar2(_text.mid(end_pos, 1)))
            end_pos++;
        deleteText(start_pos, end_pos);
        // 插入新的空白符号
        QString insText = "";
        for (int i = 0; i < us->indent_blank; i++)
            insText += "　";
        insertText(insText);
        if (old_text != _text || old_pos != _pos) // 文本有变化（否则本来就是刚好缩进OK的）
            return finishAnalyze();
    }

    // 文本没有改变，硬性跳转
    if (!has_changed)
    {
        // 成对符号判断
        /* 没必要判断，让用户多次手动点击tab键吧 */

        // 空行自动缩进
        if ((l1 == "\n" || l1 == "\r") && (r1 == "\n" || r1 == "\r"))
        {
            prepareAnalyze();
            QString insText = "";
            for (int i = 0; i < us->indent_blank; i++)
                insText += "　";
            insertText(insText);
            finishAnalyze();
            return ;
        }

        // 跳过空白
        if (isBlankChar(r1)) // 右1是空白，全部跳过
        {
            while (_pos < len && isBlankChar(_text.mid(_pos, 1)))
                _pos++;
            moveCursorFinished(_pos);
            return ;
        }
    }

    // 跳过成对符号左边
    if (isSymPairLeft(r1))
    {
        return moveCursorFinished(++_pos);
    }

    // 连续符号从左边跳到右边
    // 右1 必定不是右括号（前面已经判断过了）
    if (isSymPairLeft(l1))
    {
        QString pair_right = getSymPairRightByLeft(l1);
        int pos = _text.indexOf(pair_right, _pos);
        if (pos == -1)
            pos = _text.length();
        while (pos > 0 && isBlankChar(_text.mid(pos-1, 1)))
            pos--;
        return moveCursorFinished(pos);
    }

    // 跳过连续的标点
    if (isSentSplitPunc(r1))
    {
        while (_pos < len && isSentSplitPunc(_text.mid(_pos, 1)))
            _pos++;
        moveCursorFinished(_pos);
        return ;
    }

    // 跳过一样的文字
    int pos = _pos;
    while (pos < len && _text.mid(pos, 1) == r1)
        pos++;
    if (pos > _pos + 1)
        return moveCursorFinished(pos);


    // 跳过同一类文字
    pos = _pos;
    if (isChinese(r1))
    {
        while (pos < len && isChinese(_text.mid(pos,1)))
            pos++;
    }
    else if (isEnglish(r1))
    {
        while (pos < len && isEnglish(_text.mid(pos,1)))
            pos++;
    }
    else if (isNumber(r1))
    {
        while (pos < len && isNumber(_text.mid(pos,1)))
            pos++;
    }
    else if (isBlankChar2(r1))
    {
        while (pos < len && isBlankChar2(_text.mid(pos,1)))
            pos++;
    }
    else if (isBlankChar(r1))
    {
        while (pos < len && isBlankChar(_text.mid(pos,1)))
            pos++;
    }
    else if (isSentPunc(r1))
    {
        while (pos < len && isSentPunc(_text.mid(pos,1)))
            pos++;
    }
    else if (isSentSplit(r1))
    {
        while (pos < len && isSentSplit(_text.mid(pos,1)))
            pos++;
    }
    else // 移动到下一个空格或者换行
    {
        while (pos < len && !isBlankChar(_text.mid(pos,1)))
            pos++;
    }

    return moveCursorFinished(pos);
}

void NovelAI::activeReverseTabSkip()
{
    int len = _text.length();
    _pos = _edit->textCursor().position(); // 因为没有 prepareAnalyze()，所以光标位置不确定啊……

    if (_pos == 0) return ;

    QString l1 = _text.mid(_pos-1, 1);
    QString r1 = "";
    if (_pos < len) r1 = _text.mid(_pos, 1);

    // 成对符号右边  |）
    if (isSymPairRight(r1))
    {
        QString pair_right = r1;
        QString pair_left = getSymPairLeftByRight(pair_right);

        int find = _text.lastIndexOf(pair_left, _pos);
        if (find > -1)
        {
            return moveCursorFinished(find+1);
        }
    }

    // 成对符号右边 ）|
    if (isSymPairRight(l1))
    {
        QString pair_right = l1;
        QString pair_left = getSymPairLeftByRight(pair_right);

        int find = _text.lastIndexOf(pair_left, _pos-1);
        if (find > -1)
        {
            return moveCursorFinished(find+1);
        }
    }

    // 标点
    if (isSentSplitPunc(l1))
    {
        return moveCursorFinished(_pos-1);
    }

    int pos = _pos;
    // 跳过同一类文字
    if (isChinese(l1))
        {
            while (pos > 0 && isChinese(_text.mid(pos-1,1)))
                pos--;
        }
        else if (isEnglish(l1))
        {
            while (pos > 0 && isEnglish(_text.mid(pos-1,1)))
                pos--;
        }
        else if (isNumber(l1))
        {
            while (pos > 0 && isNumber(_text.mid(pos-1,1)))
                pos--;
        }
        else if (isBlankChar2(l1))
        {
            while (pos > 0 && isBlankChar2(_text.mid(pos-1,1)))
                pos--;
        }
        else if (isBlankChar(l1))
        {
            while (pos > 0 && isBlankChar(_text.mid(pos-1,1)))
                pos--;
        }
        else if (isSentPunc(l1))
        {
            while (pos > 0 && isSentPunc(_text.mid(pos-1,1)))
                pos--;
        }
        else if (isSentSplit(l1))
        {
            while (pos > 0 && isSentSplit(_text.mid(pos-1,1)))
                pos--;
        }
        else // 移动到下一个空格或者换行
        {
            while (pos > 0 && !isBlankChar(_text.mid(pos-1,1)))
                pos--;
        }

    return moveCursorFinished(pos);



}

/**
 * 字数统计
 * 单词算一个、省略号/破折号算一个、空白符不算
 * @param  text 需要统计的字数
 * @return      字数
 */
int NovelAI::getWordCount(QString text)
{
    int count = 0;

    // 非空白符的字数
    int len = text.length();
    QString ch;
    for (int i = 0; i < len; i++)
    {
        ch = text.mid(i, 1);
        if (!isBlankChar(ch))
            count++;
    }

    // 去掉单词/数字


    // 去掉 破折号、省略号


    return count;
}

/**
 * 字数统计
 * @param  text  要统计的文本
 * @param  ch    中文字数
 * @param  num   数字字数
 * @param  en    单词字数
 * @param  punc  标点字符
 * @param  blank 空白字数（不含换行）
 * @param  line  段落字数
 * @return       有效字数
 */
int NovelAI::getWordCount(QString text, int& ch, int& num, int& en, int& punc, int& blank, int& line)
{
    Q_UNUSED(text);
    Q_UNUSED(ch);
    Q_UNUSED(num);
    Q_UNUSED(en);
    Q_UNUSED(punc);
    Q_UNUSED(blank);
    Q_UNUSED(line);

    // TODO

    return 0;
}


/**
 * 选词
 * @param  text  总文本
 * @param  pos   要判断的位置
 * @param  start 起始位置（引用）（返回值）
 * @param  end   结束位置（引用）（返回值）
 * @return       选中的文本
 */
QString NovelAI::getWord(QString text, int pos, int &start, int &end)
{
    QString l1 = "", r1 = "";
    int len = text.length();
    start = end = pos;
    if (pos > 0) l1 = text.mid(pos-1, 1);
    if (pos < text.length()) r1 = text.mid(pos, 1);

    QString judge_c = l1;
    // 是空格
    if (isBlankChar2(l1) && isBlankChar2(r1))
    {
        while (start > 0 && isBlankChar2(text.mid(start-1,1)))
            start--;
        while (end < len && isBlankChar2(text.mid(end, 1)))
            end++;
        return text.mid(start, end - start);
    }

    // 是空白符（包括换行）
    if (isBlankChar(l1) && isBlankChar(r1))
    {
        while (start > 0 && isBlankChar(text.mid(start-1,1)))
            start--;
        while (end < len && isBlankChar(text.mid(end, 1)))
            end++;
        return text.mid(start, end - start);
    }

    // 分词算法
    int sent_start = start, sent_end = end;
    while (sent_start > 0 && !isSentSplit(text.mid(sent_start-1, 1)))
        sent_start--;
    while (sent_end < len && !isSentSplit(text.mid(sent_end, 1)))
        sent_end++;
    QString str = text.mid(sent_start, sent_end);
    QStringList list = friso_AI->WordSegment(str);
    int check_pos = 0, in_sent_pos = pos - sent_start;
    for (int i = 0; i < list.size(); i++)
    {
        check_pos = str.indexOf(list[i], check_pos);
        if (check_pos == -1) return "";
        if (check_pos < in_sent_pos && check_pos+list[i].length() >= in_sent_pos)
        {
            start = check_pos + sent_start;
            end = check_pos + sent_start + list[i].length();
            return text.mid(start, end-start);
        }
        else if (check_pos <= in_sent_pos && check_pos+list[i].length() > in_sent_pos)
        {
            start = check_pos + sent_start;
            end = check_pos + sent_start + list[i].length();
            return text.mid(start, end-start);
        }
        check_pos += list[i].length();
    }

    /*int l, r;
    l = r = pos;
    if (isChinese(judge_c))
    {
        while (l > 0)
        {
            if (isChinese(text.mid(l - 1, 1)))
                l--;
            else
                break;
        }
        while (r < len)
        {
            if (isChinese(text.mid(r + 1, 1)))
                r++;
            else
                break;
        }
    }
    else if (isEnglish(judge_c))
    {
        while (l > 0)
        {
            if (isEnglish(text.mid(l - 1, 1)))
                l--;
            else
                break;
        }
        while (r < len)
        {
            if (isEnglish(text.mid(r + 1, 1)))
                r++;
            else
                break;
        }
    }
    else if (isNumber(judge_c))
    {
        while (l > 0)
        {
            if (isNumber(text.mid(l - 1, 1)))
                l--;
            else
                break;
        }
        while (r < len)
        {
            if (isNumber(text.mid(r + 1, 1)))
                r++;
            else
                break;
        }
    }
    else if (judge_c == " " || judge_c == "　")
    {
        while (l > 0)
        {
            if (text.mid(l - 1, 1) == " " || text.mid(l - 1, 1) == "　")
                l--;
            else
                break;
        }
        while (r < len)
        {
            if (text.mid(r + 1, 1) == " " || text.mid(r + 1, 1) == "　")
                r++;
            else
                break;
        }
    }
    else if (judge_c == "\n")
    {
        while (l > 0)
        {
            if (text.mid(l - 1, 1) == "\n")
                l--;
            else
                break;
        }
        while (r < len)
        {
            if (text.mid(r + 1, 1) == "\n")
                r++;
            else
                break;
        }
    }

    start = l;
    end = r + 1; // 这个结尾是到end前面的那个字符的*/

    return text.mid(start, end - start);
}

/**
 * 获取上一个单词的位置
 * @param  pos 当前光标位置
 * @return     上一个单词的位置
 */
int NovelAI::getPreWordPos(int pos)
{
    if (pos <= 1) return 0;

    // 左边是空白的情况
    if (pos > 1 && isBlankChar2(_text.mid(pos-1, 1)) && isBlankChar(_text.mid(pos-2, 1)))
    {
        while (pos > 0 && isBlankChar2(_text.mid(pos-1, 1)))
            pos--;
        return pos;
    }
    else if (pos > 1 && isBlankChar(_text.mid(pos-1, 1)) && isBlankChar(_text.mid(pos-2, 1)))
    {
        while (pos > 0 && isBlankChar(_text.mid(pos-1, 1)))
            pos--;
        return pos;
    }

    int p = pos;
    // 找到句子的首部（包括空白）
    while (p > 0 && !isSentSplit(_text.mid(p-1, 1)))
        p--;
    if (p >= pos-1) return pos-1;

    // 分词算法
    QString str = _text.mid(p, pos-p);
    QStringList list = friso_AI->WordSegment(str);
    if (list.size() == 0) return pos-1;
    QString last_word = list.at(list.size()-1);
    int len = last_word.length();
    if (_text.mid(pos-len, len) != last_word)
    {
        p = _text.lastIndexOf(last_word, pos-len);
        return p>-1?p:pos-1;
    }
    return pos-len;
}

/**
 * 获取下一个单词的位置
 * @param  pos 当前光标位置
 * @return     下一个单词的位置
 */
int NovelAI::getNextWordPos(int pos)
{
    int len = _text.length();
    if (pos >= len) return pos;

    // 右边是空白的情况
    if (pos < len-1 && isBlankChar2(_text.mid(pos, 1)) && isBlankChar(_text.mid(pos+1, 1)))
    {
        while (pos < len && isBlankChar2(_text.mid(pos, 1)))
            pos++;
        return pos;
    }
    else if (pos < len-1 && isBlankChar(_text.mid(pos, 1)) && isBlankChar(_text.mid(pos+1, 1)))
    {
        while (pos < len && isBlankChar(_text.mid(pos, 1)))
            pos++;
        return pos;
    }

    int p = pos;
    while (p < _text.length() && !isSentSplit(_text.mid(p, 1)))
        p++;
    if (p <= pos+1) return pos+1;

    QString str= _text.mid(pos, p-pos);
    QStringList list = friso_AI->WordSegment(str);
    if (list.size() == 0) return pos+1;
    QString first_word = list.at(0);
    int word_len = first_word.length();
    if (_text.mid(pos, word_len) != first_word)
    {
        p = _text.indexOf(first_word, pos);
        return p>-1?p+word_len:pos+1;
    }
    return pos+word_len;
}

/**
 * 选句
 * @param  text  总文本
 * @param  pos   要判断的位置
 * @param  start 起始位置（引用）
 * @param  end   结束位置（引用）
 * @return       句子文本
 */
QString NovelAI::getSent(QString text, int pos, int &start, int &end)
{
    if (pos > 0 && isSymPairRight(text.mid(pos-1,1))) // 去掉成对符号（左1是右括号）
    {
        start = end = pos;
        QString pair_right = text.mid(pos-1, 1);
        QString pair_left = getSymPairLeftByRight(text.mid(pos-1, 1));
        // 使用队列删除成对符号
        int queue = 0;
        while (start > 0)
        {
            QString c = text.mid(start-1, 1);
            if (c == "\n" || c == "\r")
                break;
            else if (c == pair_right)
                queue++;
            else if (c == pair_left)
            {
                queue--;
                if (queue <= 0)
                {
                    start--;
                    break;
                }
            }
            start--;
        }
        if (pair_left == "“" && start > 0 && (text.mid(start-1, 1) == "：" || text.mid(start-1, 1) == "，")) // 前引号
            start--; // 还有删除前引号前面的冒号
        return text.mid(start, end-start);
    }
    else if (pos < text.length() && isSymPairRight(text.mid(pos, 1))) // 去掉成对符号（右1是右括号）
    {
        start = end = pos;
        QString pair_right = text.mid(pos, 1);
        QString pair_left = getSymPairLeftByRight(text.mid(pos, 1));
        // 使用队列删除成对符号
        int queue = 0;
        while (start > 0)
        {
            QString c = text.mid(start-1, 1);
            if (c == "\n" || c == "\r")
                break;
            else if (c == pair_right)
                queue++;
            else if (c == pair_left)
            {
                queue--;
                if (queue <= 0)
                {
                    break;
                }
            }
            start--;
        }
        if (start == end) // 里面是空内容……
        {
            if (start > 0)
                start--;
            if (end < text.length())
                end++;
        }
        return text.mid(start, end-start);
    }
    else if (pos > 0 && isSentSplit(text.mid(pos-1,1)))   // 句末标点后面选中左边整句话
    {
        pos--;
    }

    // 根据换行和句末标点来判断
    int len = text.length();
    if (pos > len) pos = len;
    int l = pos, r = pos;
    while (l > 0)
    {
        if (isSentSplit(text.mid(l - 1, 1)) || text.mid(l - 1, 1) == "\n" || text.mid(l - 1, 1) == "“" || text.mid(l - 1, 1) == "”")
            break;
        else
            l--;
    }
    while (r < len)
    {
        if (isSentSplit(text.mid(r, 1)) || text.mid(r, 1) == "\n" || text.mid(r, 1) == "“" || text.mid(r, 1) == "”")
            break;
        else
            r++;
    }
    if (isSentSplit(text.mid(r, 1)))
    {
        r++;
    }

    // 去掉左右的空白符
    while(l < pos && isBlankChar(text.mid(l, 1)))
    {
        l++;
    }
    while (r > pos && isBlankChar(text.mid(r - 1, 1)))
    {
        r--;
    }

    start = l;
    end = r;

    return text.mid(start, end - start);
}

QString NovelAI::getSent2(QString text, int pos, int &start, int &end)
{
    if (pos > 0 && isSentPunc(text.mid(pos-1,1)))   // 句末标点后面选中左边整句话
    {
        pos--;
    }
    else if (pos > 0 && isSymPairRight(text.mid(pos-1,1))) // 去掉成对符号
    {
        start = end = pos;
        QString pair_right = text.mid(pos-1, 1);
        QString pair_left = getSymPairLeftByRight(text.mid(pos-1, 1));
        // 使用队列删除成对符号
        int queue = 0;
        while (start > 0)
        {
            QString c = text.mid(start-1, 1);
            if (c == "\n" || c == "\r")
                break;
            else if (c == pair_right)
                queue++;
            else if (c == pair_left)
            {
                queue--;
                if (queue <= 0)
                {
                    start--;
                    break;
                }
            }
            start--;
        }
        if (pair_left == "“" && start > 0 && (text.mid(start-1, 1) == "：" || text.mid(start-1, 1) == "，")) // 前引号
            start--; // 还有删除前引号前面的冒号
        return text.mid(start, end-start);
    }

    // 根据换行和句末标点来判断
    int l = pos, r = pos;
    int len = text.length();
    while (l > 0)
    {
        if (isSentPunc(text.mid(l - 1, 1)) || text.mid(l - 1, 1) == "\n" || text.mid(l - 1, 1) == "“" || text.mid(l - 1, 1) == "”")
            break;
        else
            l--;
    }
    while (r < len)
    {
        if (isSentPunc(text.mid(r, 1)) || text.mid(r, 1) == "\n" || text.mid(r, 1) == "“" || text.mid(r, 1) == "”")
            break;
        else
            r++;
    }
    if (isSentPunc(text.mid(r, 1)))
    {
        r++;
    }

    // 去掉左右的空白符
    while(l < pos && isBlankChar(text.mid(l, 1)))
    {
        l++;
    }
    while (r > pos && isBlankChar(text.mid(r - 1, 1)))
    {
        r--;
    }

    start = l;
    end = r;

    return text.mid(start, end - start);
}

QString NovelAI::getPair(QString text, int pos, int &start, int &end)
{
    QString l1 = "", r1 = "";
    if (pos > 0)
        l1 = text.mid(pos-1, 1);
    if (pos < text.length())
        r1 = text.mid(pos, 1);

    if (l1 != "" && isSymPairRight(l1)) // 括号外围
    {
        QString pair_right = l1;
        QString pair_left = getSymPairLeftByRight(l1);
        // 使用队列找到成对符号
        int queue = 0;
        while (start > 0)
        {
            QString c = text.mid(start-1, 1);
            if (c == "\n" || c == "\r")
                break;
            else if (c == pair_right)
                queue++;
            else if (c == pair_left)
            {
                queue--;
                if (queue <= 0)
                {
                    start--;
                    break;
                }
            }
            start--;
        }
        end = pos;
        return text.mid(start, end-start);
    }
    else if (r1 != "" && isSymPairRight(r1)) // 括号里面
    {
        QString pair_right = r1;
        QString pair_left = getSymPairLeftByRight(r1);
        // 使用队列找到成对符号
        int queue = 0;
        while (start > 0)
        {
            QString c = text.mid(start-1, 1);
            if (c == "\n" || c == "\r")
                break;
            else if (c == pair_right)
                queue++;
            else if (c == pair_left)
            {
                queue--;
                if (queue <= 0)
                {
                    break;
                }
            }
            start--;
        }
        end = pos;
        return text.mid(start, end-start);
    }
    else
        return "";
}

/**
 * 选段，根据换行来判断
 * @param  text  总文本
 * @param  pos   要判断的位置
 * @param  start 起始位置（引用）
 * @param  end   结束位置（引用）
 * @return       段落文本
 */
QString NovelAI::getPara(QString text, int pos, int &start, int &end)
{
    if (pos < text.length() && text.mid(pos, 1) == "\n")
    {
        end = pos;
        start = text.lastIndexOf("\n", pos-1) + 1;
        while (start < end && isBlankChar(text.mid(start, 1)))
            start++;
    }
    else if (pos > 0 && text.mid(pos-1, 1) == "\n")
    {
        start = pos;
        end = text.indexOf("\n", pos);
        if (end == -1)
            end = text.length();
    }
    else
    {
        if (pos == text.length()) pos--; // 最后一个位置搜索会出问题的……
        start = text.lastIndexOf("\n", pos) + 1;
        end = text.indexOf("\n", pos);
        if (end == -1)
            end = text.length();
        while (start < end && isBlankChar(text.mid(start, 1)))
            start++;
    }
    return text.mid(start, end - start);
}

void NovelAI::activeSentFinish()
{
    prepareAnalyze();
    SentFinish();
    finishAnalyze();
}

void NovelAI::activeExpandSelection()
{
    int start = -1, end = -1;
    ExpandSelection(_edit->textCursor().selectionStart(), _edit->textCursor().selectionEnd(), start, end);
    if (start == end || start== -1 || end == -1) return ; // 没有选择

    QTextCursor tc = _edit->textCursor();
    tc.setPosition(start, QTextCursor::MoveAnchor);
    tc.setPosition(end, QTextCursor::KeepAnchor);
    _edit->setTextCursor(tc);
}

void NovelAI::activeShrinkSelection()
{
    int start = -1, end = -1;
    ShrinkSelection(_edit->textCursor().selectionStart(), _edit->textCursor().selectionEnd(), start, end);
    if (start == 0 && end == _text.length()) // 如果回退一次还是全选，则再次回退
        ShrinkSelection(_edit->textCursor().selectionStart(), _edit->textCursor().selectionEnd(), start, end);
    if (start == end || start== -1 || end == -1) return ; // 没有选择

    QTextCursor tc = _edit->textCursor();
    tc.setPosition(start, QTextCursor::MoveAnchor);
    tc.setPosition(end, QTextCursor::KeepAnchor);
    _edit->setTextCursor(tc);
}

bool NovelAI::activeHomonymCover()
{
    prepareAnalyze();
    int current_pos = _edit->textCursor().position();
    if (_pre_changed_pos != current_pos)
        return false;

    bool result = HomonymCover(_pos, _dif);
    finishAnalyze();
    return result;
}

bool NovelAI::HomonymCover(int end_pos, int diff)
{
    if (diff > 5) return false;
    if (end_pos < diff * 2) return false;
    int pos = end_pos - diff;
    QString s_new = _text.mid(pos, diff);
    QString s_old = _text.mid(pos-diff, diff);

    // 判断是否满足条件
    if (s_new == s_old) return false;
    for (int i = 0; i < s_new.length(); i++)
        if (!isChinese(s_new.mid(i, 1)))
            return false;
    for (int i = 0; i < s_old.length(); i++)
        if (!isChinese(s_old.mid(i, 1)))
            return false;

    // 判断黑名单
    for (int i = 1; i < s_new.length(); i++)
    {
        QString pat = s_new.mid(i, i);
        pat = "\\b" + pat + "\\b";
        if (canRegExp(us->homonym_cover_blanklist, pat))
            return false;
    }
    for (int i = 1; i < s_old.length(); i++)
    {
        QString pat = s_old.mid(i, i);
        pat = "\\b" + pat + "\\b";
        if (canRegExp(us->homonym_cover_blanklist, pat))
            return false;
    }
    QStringList pys_new = PinyinUtil::ChineseConvertPinyins(s_new),
            pys_old = PinyinUtil::ChineseConvertPinyins(s_old);

    int size;
    if ( (size = pys_new.size()) != pys_old.size() )
        return false;

    for (int i = 0; i < size; i++)
    {
        if (pys_new.at(i) != pys_old.at(i))
        {
            // TODO 判断模糊音
            return false;
        }
    }

    deleteText(pos-diff, pos);
    edit_range_start = pos-diff;
    edit_range_end = pos;

    return true;
}

/**
 * 标点覆盖标点，不判断特定标点之外的内容
 * @return 是否覆盖标点
 */
bool NovelAI::PuncCover()
{
    if (isChinese(_left1) || isChinese(_left2)) return false; // 中文
    if (_left1 != "，" && !isSentPunc(_left1)) return false; // 左1 不是句末标点
    if (_left2 != "，" && !isSentPunc(_left2)) return false;  // 左2 不是句末标点

    if (_left1 == _left2) return false; // 重复的标点

    if (_left2 == "？" && _left1 == "！") return false; // 特殊情况：？！ 可以连续

    deleteText(_pos - 2, _pos - 1);

    edit_range_start = _pos-1;
    edit_range_end = _pos;

    return true;
}

/**
 * 括号匹配：输入左半括号，自动补全右半括号
 * @return 是否补全
 */
bool NovelAI::PairMatch()
{
    if (_right1 != "" && _right1 != "\n" && _right1 != "”" && _right1 != "　" && _right1 != " ") return false; // 只有空白处才自动添加

    /*int pair_pos = _symbol_pair_lefts.indexOf(_left1);
    if (pair_pos == -1) return false; // 成对符号那里没有找到标点
    const QString pair_right = _symbol_pair_rights.mid(pair_pos, 1);*/

    if (!isSymPairLeft(_left1)) return false;
    const QString pair_right = getSymPairRightByLeft(_left1);

    // 当前段落
    int nl_pos = _text.lastIndexOf("\n", _pos > 0 ? _pos - 1 : _pos);
    int nr_pos = _text.indexOf("\n", _pos);
    if (nl_pos == -1) nl_pos = 0;
    if (nr_pos == -1) nr_pos = _text.length();
    QString para = _text.mid(nl_pos + 1, nr_pos - nl_pos - 1); // 段落文本，用来判断括号堆栈
    int para_pos = _pos - nl_pos - 1;                     // 光标相对于段落的位置

    // 左边括号多余的数量（包括刚加上去的那个）
    int l_count = 0;
    for (int i = 0; i < para_pos; i++)
        if (para.mid(i, 1) == _left1)
            l_count++;
        else if (para.mid(i, 1) == _right1)
            l_count--;

    // 右边括号剩余的数量
    int r_count = 0, len = para.length();
    for (int i = para_pos; i < len; i++)
        if (para.mid(i, 1) == _left1)
            r_count--;
        else if (para.mid(i, 1) == _right1)
            r_count++;

    if (l_count > r_count)   // 这个段落右括号的数量比左括号多，或者数量一样，则一起删除
    {
        insertText(pair_right);
        moveCursor(-1);
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * 括号跳转：在右半括号中输入同样的括号，如果多余则调到右边
 * @return 是否括号跳转
 */
bool NovelAI::PairJump()
{
    if (isBlankChar(_left1) || isChinese(_left1) || _left1 != _right1) return false; // 空白符、中文、右边两个字符不一样
    /*if (_symbol_pair_rights.indexOf(_left1) == -1) return false; // 不是右括号
    QString pair_left = _symbol_pair_lefts.mid( _symbol_pair_rights.indexOf(_left1), 1 );
    QString pair_right = _left1;*/
    if (!isSymPairRight(_left1)) return false;
    QString pair_left = getSymPairLeftByRight(_left1);
    QString pair_right = _left1;

    // 当前段落
    int nl_pos = _text.lastIndexOf("\n", _pos > 0 ? _pos - 1 : _pos);
    int nr_pos = _text.indexOf("\n", _pos);
    if (nl_pos == -1) nl_pos = 0;
    if (nr_pos == -1) nr_pos = _text.length();
    QString para = _text.mid(nl_pos + 1, nr_pos - nl_pos - 1); // 段落文本，用来判断括号堆栈
    int para_pos = _pos - nl_pos - 1;                     // 光标相对于段落的位置

    // 左边括号多余的数量（包括刚加上去的那个）
    int l_count = 0;
    for (int i = 0; i < para_pos; i++)
        if (para.mid(i, 1) == pair_left)
            l_count++;
        else if (para.mid(i, 1) == pair_right)
            l_count--;

    // 右边括号剩余的数量
    int r_count = 0, len = para.length();
    for (int i = para_pos; i < len; i++)
        if (para.mid(i, 1) == pair_left)
            r_count--;
        else if (para.mid(i, 1) == pair_right)
            r_count++;

    if (r_count > l_count)   // 这个段落右括号的数量比左括号多，或者数量一样，则一起删除
    {
        deleteText(_pos, _pos + 1);
        // moveCursor(1);
        return true;
    }
    else
    {
        return false;
    }
}

void NovelAI::advanceBackspace()
{
    prepareAnalyze();

    ;

    finishAnalyze();
}

void NovelAI::advanceDelete()
{
    prepareAnalyze();

    ;

    finishAnalyze();
}

void NovelAI::Typeset()
{
    prepareAnalyze();

    // ==== 存储之前的状态 ====
    bool is_all_end = false;   // 是否在全文最后
    bool is_para_end = false;  // 是否在段落最后
    //int scroll_slide = _edit->verticalScrollBar()->sliderPosition(); // 滚动进度
    int viewport_slide = _edit->cursorRect().top();

    // 判断是不是在全文尾
    if (isBlankChar(_left1))
    {
        int pos = _pos, len = _text.length();
        while (pos < len && isBlankChar(_text.mid(pos, 1)))
            pos++;
        if (pos == len) // 光标后面全是空白的，表示是编辑末尾
        {
            if (isBlankString(_text)) // 全是空白的……
            {
                int  num_blank = us->indent_blank;
                QString new_blank = "";
                while (num_blank--)
                    new_blank += "　";
                deleteText(0, _text.length());
                insertText(0, new_blank);
                return finishAnalyze();
            }
            else
            {
                is_all_end = true;
            }
        }
    }
    else if (_right1 == "")
    {
        if (isSentPunc(_left1))
        {
            is_all_end = true;
        }
    }
    else if (_right1 == "\n") // 左1 有非空白符，右1 是段落结尾
    {
        is_para_end = true;
    }

    // ==== 删除多余的连续空格 ====
    QRegExp re1("[ 　]{2,}");
    int pos = 0;
    while ( (pos = re1.indexIn(_text, pos)) != -1 )
    {
        deleteText(pos, pos+re1.matchedLength());
    }

    // ==== 删除行间空白 ====
    QRegExp re2("[\\s　]*\\n[\\s　]*");
    pos = 0;
    while ( (pos = re2.indexIn(_text, pos)) != -1 )
    {
        deleteText(pos, pos+re2.matchedLength());
        insertText(pos, "\n");
        pos++;
    }
    if (is_para_end)
        moveCursor(-1);

    // ==== 删除首尾空白 ====
    if (_text.startsWith("\n"))
        deleteText(0, 1);
    if (_text.endsWith("\n"))
        deleteText(_text.length()-1, _text.length());

    // ==== 长段落分段 ====
    if (us->typeset_split)
    {
        pos = 0;
        int prev_pos = 0, para_max = us->para_max_length;
        while ( (pos = _text.indexOf("\n", pos+1)) != -1)
        {
            if (pos - prev_pos > para_max)
            {
                paraSplit(pos, false);
            }
            prev_pos = pos;
        }
    }

    // ==== 设置空格和换行的数量 ====
    int  num_blank = us->indent_blank, num_line = us->indent_line;
    QString new_blank = "", new_line = "";
    while (num_blank--)
        new_blank += "　";
    while (num_line--)
        new_line += "\n";
    int new_delta = new_line.length()+new_blank.length();

    // ==== 每段之间的分隔 ====
    insertText(pos = 0, new_blank); // 第一行前面加上缩进
    pos++;
    while ( (pos = _text.indexOf("\n", pos)) != -1 )
    {
        insertText(++pos, new_line+new_blank);
        pos += new_delta;
    }

    // ==== 智能判断最后是否需要再次插入空行 ====
    if (is_all_end)
    {
        insertText(_text.length(), "\n" + new_line + new_blank);
    }

    // ==== 数字字母和中文之间的空格 ====
    // 这个是匹配中文的，但是不知道为什么居然也能匹配字母数字 [\u4e00-\u9fa5]  不需要两个反斜杠转义
    // 匹配双字节的所有字符（排除全角空格）  [^\\x00-\\xff　]  这个需要两个反斜杠转义
    if (us->typeset_blank)
    {
        QRegExp re3("[\u4e00-\u9fa5][a-zA-Z0-9]");
        pos = 0;
        while ( (pos = re3.indexIn(_text, pos)) != -1 )
        {
            insertText(pos+1, " ");
            pos += re3.matchedLength()+1;
        }
        QRegExp re4("[a-zA-Z0-9][\u4e00-\u9fa5]");
        pos = 0;
        while ( (pos = re4.indexIn(_text, pos)) != -1 )
        {
            insertText(pos+1, " ");
            pos += re4.matchedLength()+1;
        }
    }

    // ==== 英文标点格式化 ====
    // 因为要保留光标位置，所以就只能用循环一个个换过去了……不能直接全部替换
    if (us->typeset_english)
    {
        // 排版英文句尾
        QRegExp re5("[a-zA-Z]+ [a-zA-Z]+[,.!?:]([a-zA-Z])");
        pos = 0;
        while ( (pos = re5.indexIn(_text, pos)) != -1 )
        {
            insertText(pos+re5.matchedLength()-1, " ");
            pos += re5.matchedLength()+1;
        }

        // 空出单词开头（略）
        // 空出负数开头（略）
        // 空出单词结尾（略）
        // 取消小数点空格（略）

        // 句尾后首字母大写
        QRegExp re6("[a-zA-Z][.?!] +[a-z]");
        pos = 0;
        while ( (pos = re6.indexIn(_text, pos)) != -1 )
        {
            pos += re6.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 全文首字母大写
        QRegExp re7("^[ 　\t]*[a-z]");
        pos = 0;
        while ( (pos = re7.indexIn(_text, pos)) != -1 )
        {
            pos += re7.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 段首字母大写
        QRegExp re9("\n[ 　\t]*[a-z]");
        pos = 0;
        while ( (pos = re9.indexIn(_text, pos)) != -1 )
        {
            pos += re9.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 全文首引号后首字母大写
        QRegExp re8("^[ 　\t]*\"[a-z]");
        pos = 0;
        while ( (pos = re8.indexIn(_text, pos)) != -1 )
        {
            pos += re8.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 段首引号后首字母大写
        QRegExp re10("\n[ 　\t]*\"[a-z]");
        pos = 0;
        while ( (pos = re10.indexIn(_text, pos)) != -1 )
        {
            pos += re10.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 说话引号后首字母大写
        QRegExp re11("[,:?!.][ 　\t]*\"[a-z]");
        pos = 0;
        while ( (pos = re11.indexIn(_text, pos)) != -1 )
        {
            pos += re11.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }
    }

    finishAnalyze();

    int new_scroll_slide = _edit->verticalScrollBar()->sliderPosition();
    int new_viewprot_slide = _edit->cursorRect().top();
    int delta = viewport_slide - new_viewprot_slide; // 光标位置的差距
    new_scroll_slide -= delta;
    _edit->verticalScrollBar()->setSliderPosition(new_scroll_slide);
}

/**
 * 粘贴排版，需要手动开启
 * // 此处不进行英文排版？
 * ctrl+V 或者 Ctrl+Shift+V 触发
 * @param x    粘贴位置
 * @param text 粘贴的原文本
 */
void NovelAI::TypesetPaste(int x, QString text)
{
    prepareAnalyze();

    // TODO
    insertText(x, text);

    finishAnalyze();
}

void NovelAI::TypesetPart(int start_pos, int end_pos)
{
    QString origin_text = _text; // 保存原来的文本

    prepareAnalyze();
    _pos = end_pos;
    int &_end_pos = _pos; // 因为 end_pos 一直在变，所以使用引用重新造了一个 end_pos 实例来表示结尾

    // === 先删除首尾空白 ====
    int front_lines = 0, back_lines = 0;
    int pos = start_pos;
    while (pos < _end_pos && isBlankChar(_text.mid(pos, 1)))
    {
    	if (_text.mid(pos, 1) == "\n")
    		front_lines++; // 开头的换行数
    	pos++;
    }
    deleteText(start_pos, pos);
    pos = _end_pos;
    while (pos > start_pos && isBlankChar(_text.mid(pos-1, 1)))
    {
    	if (_text.mid(pos-1, 1) == "\n")
    		back_lines++; // 末尾的换行数
    	pos--;
    }
    deleteText(pos, _end_pos);


    // ==== 删除多余的连续空格 ====
    QRegExp re1("[ 　]{2,}");
    pos = start_pos;
    while ( (pos = re1.indexIn(_text, pos)) != -1 && pos < _end_pos )
    {
        deleteText(pos, pos+re1.matchedLength());
    }

    // ==== 删除行间空白 ====
    QRegExp re2("[\\s　]*\\n[\\s　]*");
    pos = start_pos;
    while ( (pos = re2.indexIn(_text, pos)) != -1 && pos < _end_pos )
    {
        deleteText(pos, pos+re2.matchedLength());
        insertText(pos, "\n");
        pos++;
    }

    // ==== 长段落分段 ====
    if (us->typeset_split)
    {
        pos = start_pos;
        int prev_pos = start_pos, para_max = us->para_max_length;
        while ( (pos = _text.indexOf("\n", pos+1)) != -1 && pos < _end_pos )
        {
            if (pos - prev_pos > para_max)
            {
                paraSplit(pos, false);
            }
            prev_pos = pos;
        }
    }

    // ==== 设置空格和换行的数量 ====
    int  num_blank = us->indent_blank, num_line = us->indent_line;
    QString new_blank = "", new_line = "";
    while (num_blank--)
        new_blank += "　";
    while (num_line--)
        new_line += "\n";
    int new_delta = new_line.length()+new_blank.length();

    // ==== 每段之间的分隔 ====
    if ((start_pos > 0 && _text.mid(start_pos-1, 1) == "\n") || front_lines>0 || start_pos == 0)
	    insertText(pos = start_pos, new_blank); // 第一行前面加上缩进
    pos = start_pos + 1;
    while ( (pos = _text.indexOf("\n", pos)) != -1 && pos < _end_pos )
    {
        insertText(++pos, new_line+new_blank);
        pos += new_delta;
    }


    // === 计算前后的空白行 ====
    // 计算 start_pos 前面的换行数量，以及 _end_pos 后面的换行数量，直至中文位置
    int exist_front_lines = 0, exist_back_lines = 0;
    pos = start_pos;
    while (pos > 0 && isBlankChar(_text.mid(pos-1, 1)))
    {
        if (_text.mid(pos-1, 1) == "\n")
            exist_front_lines++;
        pos--;
    }
    pos = _end_pos;
    while(pos < _text.length() && isBlankChar(_text.mid(pos, 1)))
    {
        if (_text.mid(pos, 1) == "\n")
            exist_back_lines++;
        pos++;
    }
    // 在前面插入空行
    if (exist_front_lines || front_lines)
    {
        QString new_line = "";
        for (int  i = 0; i <= us->indent_line - exist_front_lines; i++)
            new_line += "\n";
        if (!new_line.isEmpty())
            insertText(start_pos, new_line);
    }
    // 在后面插入空行
    if (exist_back_lines || back_lines)
    {
        QString new_line = "";
        for (int i = 0; i <= us->indent_line - exist_back_lines; ++i)
            new_line += "\n";
        if (!new_line.isEmpty())
            insertText(_end_pos, new_line);
    }


    // ==== 数字字母和中文之间的空格 ====
    // 这个是匹配中文的，但是不知道为什么居然也能匹配字母数字 [\u4e00-\u9fa5]  不需要两个反斜杠转义
    // 匹配双字节的所有字符（排除全角空格）  [^\\x00-\\xff　]  这个需要两个反斜杠转义
    if (us->typeset_blank)
    {
        QRegExp re3("[\u4e00-\u9fa5][a-zA-Z0-9]");
        pos = start_pos;
        while ( (pos = re3.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            insertText(pos+1, " ");
            pos += re3.matchedLength()+1;
        }
        QRegExp re4("[a-zA-Z0-9][\u4e00-\u9fa5]");
        pos = start_pos;
        while ( (pos = re4.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            insertText(pos+1, " ");
            pos += re4.matchedLength()+1;
        }
    }

    // ==== 英文标点格式化 ====
    // 因为要保留光标位置，所以就只能用循环一个个换过去了……不能直接全部替换
    if (us->typeset_english)
    {
        // 排版英文句尾
        QRegExp re5("[a-zA-Z]+ [a-zA-Z]+[,.!?:]([a-zA-Z])");
        pos = start_pos;
        while ( (pos = re5.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            insertText(pos+re5.matchedLength()-1, " ");
            pos += re5.matchedLength()+1;
        }

        // 空出单词开头（略）
        // 空出负数开头（略）
        // 空出单词结尾（略）
        // 取消小数点空格（略）

        // 句尾后首字母大写
        QRegExp re6("[a-zA-Z][.?!] +[a-z]");
        pos = start_pos;
        while ( (pos = re6.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            pos += re6.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 全文首字母大写
        QRegExp re7("^[ 　\t]*[a-z]");
        pos = start_pos;
        while ( (pos = re7.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            pos += re7.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 段首字母大写
        QRegExp re9("\n[ 　\t]*[a-z]");
        pos = start_pos;
        while ( (pos = re9.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            pos += re9.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 全文首引号后首字母大写
        QRegExp re8("^[ 　\t]*\"[a-z]");
        pos = start_pos;
        while ( (pos = re8.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            pos += re8.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 段首引号后首字母大写
        QRegExp re10("\n[ 　\t]*\"[a-z]");
        pos = start_pos;
        while ( (pos = re10.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            pos += re10.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }

        // 说话引号后首字母大写
        QRegExp re11("[,:?!.][ 　\t]*\"[a-z]");
        pos = start_pos;
        while ( (pos = re11.indexIn(_text, pos)) != -1 && pos < _end_pos )
        {
            pos += re11.matchedLength();
            QString c = _text.mid(pos-1, 1);
            c = c.toUpper();
            deleteText(pos-1, pos);
            insertText(pos-1, c);
        }
    }

    // 光标聚焦到最后非空白符的位置
    if (exist_back_lines || back_lines)
    {
        while (_pos > 0 && isBlankChar(_text.mid(_pos-1, 1)))
            _pos--;
    }

    finishAnalyze();

    if (_text == origin_text) // 如果排版之后和父类相比没什么差别，则取消本次保存
    {
        cutRecorderPosition();
    }
}


/**
 * 手动分割过长段落
 * @param  x [description]
 * @return   [description]
 */
bool NovelAI::activeParaSplit(int x)
{
    prepareAnalyze();
    bool rst = paraSplit(x, true);
    finishAnalyze();
    return rst;
}

/**
 * 配合自动提示用，删除一部分的文字，用来替换某个词语
 * @param start
 * @param end
 * @param word
 */
void NovelAI::activeWordReplace(int start, int end, QString word)
{
    prepareAnalyze();
    deleteText(start, end);
    insertText(start, word);
    finishAnalyze();
}


void NovelAI::ExpandSelection(int start, int end, int& callback_start, int& callback_end)
{
    _pos = _edit->textCursor().position();

    // 保存当前选择状态（用来给收缩选择返回）
    if (!_edit->textCursor().hasSelection())
        selection_list.clear();
    // warning：保存的是上次选择的位置，而不是扩展选择后的位置！！！
    // 如果要返回上一次，get成功后再back即可
    selection_list.append(_edit->textCursor());


    // 处理预定义的位置
    if (start == -1)
        start = _pos;
    if (end == -1)
        end = _pos;
    if (start > end)
    {
        int temp = end;
        end = start;
        start = temp;
    }

    // ==== 选词语 ====
    getWord(_text, end, callback_start, callback_end);
    if ( (callback_start <= start && callback_end > end)
        || (callback_start < start && callback_end >= end) )
        return ;

    // ==== 选句子 ====
    getSent(_text, end, callback_start, callback_end);
    if ( (callback_start <= start && callback_end > end)
        || (callback_start < start && callback_end >= end) )
        return ;

    // ==== 选句子2 ====
    getSent2(_text, end, callback_start, callback_end);
    if ( (callback_start <= start && callback_end > end)
        || (callback_start < start && callback_end >= end) )
        return ;

    // ==== 选括号 ====
    if (getPair(_text, end, callback_start, callback_end) != "")
        if ( (callback_start <= start && callback_end > end)
            || (callback_start < start && callback_end >= end) )
            return ;

    // ==== 选段落 ====
    getPara(_text, end, callback_start, callback_end);
    if ( (callback_start <= start && callback_end > end)
        || (callback_start < start && callback_end >= end) )
        return ;

    // ==== 选全文 ====
    callback_start = 0;
    callback_end = _text.length();
    return ;
}


void NovelAI::ShrinkSelection(int start, int end, int& callback_start, int& callback_end)
{
    _pos = _edit->textCursor().position();

    // 检查上一次的收缩状态
    if (selection_list.getCallback(callback_start, callback_end))
    {
        selection_list.back();
        return ;
    }

    // 处理预定义的位置
    if (start == -1)
        start = _pos;
    if (end == -1)
        end = _pos;
    if (start > end)
    {
        int temp = end;
        end = start;
        start = temp;
    }

    // ==== 选全文 ====
    callback_start = 0;
    callback_end = _text.length();
    if ( (callback_start > start && callback_end <= end)
        || (callback_start >= start && callback_end > end) )
        return ;

    // ==== 选段落 ====
    getPara(_text, end, callback_start, callback_end);
    if ( (callback_start > start && callback_end <= end)
        || (callback_start >= start && callback_end > end) )
        return ;

    // ==== 选括号 ====
    if (getPair(_text, end, callback_start, callback_end) != "")
        if ( (callback_start > start && callback_end <= end)
             || (callback_start >= start && callback_end > end) )
            return ;

    // ==== 选句子2 ====
    getSent2(_text, end, callback_start, callback_end);
    if ( (callback_start > start && callback_end <= end)
        || (callback_start >= start && callback_end > end) )
        return ;


    // ==== 选句子 ====
    getSent(_text, end, callback_start, callback_end);
    if ( (callback_start > start && callback_end <= end)
        || (callback_start >= start && callback_end > end) )
        return ;

    // ==== 选词语 ====
    getWord(_text, end, callback_start, callback_end);
    if ( (callback_start > start && callback_end <= end)
        || (callback_start >= start && callback_end > end) )
        return ;

    // ==== 没得选 ====
    callback_start = callback_end = end;
    return ;
}


void NovelAI::backspaceWord(int x)
{
    prepareAnalyze();
    // 空白符 特判
    if (x > 0 && isBlankChar(_text.mid(x-1, 1)))
    {
        if (_left1 == " " || _left1 == "　" || _left1 == "\t")
        {
            int pos = _pos-1;
            while (pos > 0 && (_text.mid(pos-1, 1) == " " || _text.mid(pos-1, 1) == "　" || _text.mid(pos-1, 1) == "\t"))
                pos--;
            deleteText(pos, _pos);
        }
        else if (_left1 == "\n")
        {
            int pos = _pos-1;
            while (pos > 0 && isBlankChar(_text.mid(pos-1, 1)))
                pos--;
            deleteText(pos, _pos);
        }
        else
        {
            deleteText(_pos-1, _pos);
        }
        finishAnalyze();
        return ;
    }
    // 成对符号 特判
    if (canDeletePairPunc())
    {
        if ( (_left1 == "“" && (_left2 == "：" || _left2 == "，"))
            || (_left2 == "“" && _left1 == "”" && (_left3 == "：" || _left3 == "，") ) )
            deleteText(_pos-1, _pos);
        finishAnalyze();
        return ;
    }

    int pos = getPreWordPos(x);
    if (pos>=0 && pos<x)
    {
        deleteText(pos, x);
        _pos = pos;
        finishAnalyze();
    }
}

void NovelAI::deleteWord(int x)
{
    int pos = getNextWordPos(x);
    if (pos>=0 && pos>x && pos<=_text.length())
    {
        deleteText(pos, x);
        _pos = x;
        finishAnalyze();
    }
}

void NovelAI::moveToPreWord(int x)
{
    int pos = getPreWordPos(x);
    if (pos < 0 || pos >= x)
    {
        if (pos > 0)
            pos = x - 1;
        else
            pos = x;
    }

    moveCursorFinished(pos);
}

void NovelAI::moveToNextWord(int x)
{
    int pos = getNextWordPos(x);
    if (pos <= x || pos > _text.length())
    {
        if (pos < _text.length())
            pos = x + 1;
        else
            pos = _text.length();
    }

    moveCursorFinished(pos);
}

void NovelAI::moveToSentStart(int pos)
{
    int org_pos = pos;
    bool skip_line = false;

    // 跳过前面的前引号
    while (pos > 0 && isSymPairLeft(_text.mid(pos-1, 1)))
        pos--;

    // 跳过空白或者右边的括号
    while (pos > 0 && (isBlankChar(_text.mid(pos-1,1)) || isSymPairRight(_text.mid(pos-1, 1))))
    {
        if (_text.mid(pos, 1) == "\n")
            skip_line = true;
        pos--;
    }

    if (skip_line) // 跳过了换行，移动到上一段的末尾
    {
        if (pos > 0 && isBlankChar(_text.mid(pos-1, 1)))
            pos--;
    }
    else // 移动到句首
    {
        // 左1是标点，跳过
        if (pos > 0 && isSentSplit(_text.mid(pos-1,1)))
            pos--;

        // 遍历左边，如果是标点则停下
        while (pos > 0 && !isSentSplit(_text.mid(pos-1, 1)))
            pos--;
    }

    // 去掉前方空白
    while (pos < org_pos-1 && isBlankChar2(_text.mid(pos, 1)))
        pos++;

    moveCursorFinished(pos);
}

void NovelAI::moveToSentEnd(int pos)
{
    int org_pos = pos, len = _text.length();
    Q_UNUSED(org_pos);
    bool skip_line = false;

    // 右1就是标点时，跳过
    while (pos < len && isSentSplit(_text.mid(pos, 1)) && _text.mid(pos, 1) != "\n" && _text.mid(pos, 1) != "\r")
        pos++;

    // 右1就是后引号时，跳过
    while (pos < len && isSymPairRight(_text.mid(pos, 1)))
        pos++;

    // 右1就是前引号，跳过
    while (pos < len && isSymPairLeft(_text.mid(pos, 1)))
        pos++;

    // 跳过右边的空白
    while (pos < len && (isBlankChar(_text.mid(pos, 1))))
    {
        if (_text.mid(pos, 1) == "\n")
            skip_line = true;
        pos++;
    }

    if (skip_line) // 跳过了换行，移到下一段的段首
    {
        while (pos < len && isBlankChar2(_text.mid(pos, 1)))
               pos++;
    }
    else // 移到句尾
    {
        if (pos < len && isSentSplit(_text.mid(pos, 1)))
            pos++;
        else
            // 遍历右边，遇到句末标点停下
            while (pos < len && !isSentSplit(_text.mid(pos, 1)))
                pos++;
    }

    moveCursorFinished(pos);
}

void NovelAI::moveToParaStart(int x)
{
    if (x == 0) return ;
    int pos = _text.lastIndexOf("\n", x-1)+1;
    int org_pos = pos; // 换行符后面
    // 跳过空格
    while (pos < _text.length() && isBlankChar(_text.mid(pos, 1)))
        pos++;
    if (org_pos != pos && x != org_pos && x <= pos) // 原位置在段首前面，则放到换行符后面
    {
        pos = _text.lastIndexOf("\n", x)+1;
    }
    else if (x == org_pos) // 光标本来就在段首，切换到上一段
    {
        pos = x;
        while (pos > 0 && isBlankChar(_text.mid(pos-1, 1)))
            pos--;
        if (pos > 0)
            pos = _text.lastIndexOf("\n", pos-1)+1;
    }

    moveCursorFinished(pos);
}

void NovelAI::moveToParaStart2(int x)
{
    if (x == 0) return ;
    int pos = _text.lastIndexOf("\n", x-1) + 1;

    _pos = pos;
    _text_cursor.setPosition(_pos);
    _edit->setTextCursor(_text_cursor);
    //updateRecorderPosition(_pos);
}

void NovelAI::moveToParaEnd(int x)
{
    int len = _text.length();
    int pos = _text.indexOf("\n", x);
    if (pos == -1)
        pos = len;
    else if (pos == x) // 光标目前就在段尾，跳到下一段的段尾
    {
        while (pos < len && isBlankChar(_text.mid(pos, 1)))
            pos++;
        pos = _text.indexOf("\n", pos);
        if (pos == -1)
            pos = len;
    }

    moveCursorFinished(pos);
}

void NovelAI::moveToParaPrevStart(int x)
{
    if (x == 0) return ;
    int pos = _text.lastIndexOf("\n", x-1)+1;
    while (pos > 0 && isBlankChar(_text.mid(pos-1, 1)))
        pos--;
    if (pos > 0)
        pos = _text.lastIndexOf("\n", pos-1)+1;

    moveCursorFinished(pos);
}

void NovelAI::moveToParaPrevEnd(int x)
{
    if (x == 0) return ;
    int pos = _text.lastIndexOf("\n", x-1);
    if (pos == -1)
        pos = 0;
    while (pos > 0 && isBlankChar(_text.mid(pos-1, 1)))
        pos--;

    if (pos > 0)
        pos = _text.indexOf("\n", pos);
    // TODO : pos == 0时，表示是第一段，需要特殊判断

    moveCursorFinished(pos);
}

void NovelAI::moveToParaNextStart(int x)
{
    int len = _text.length();
    int pos = _text.indexOf("\n", x);
    if (pos == -1) pos = len;
    while (pos < len && isBlankChar(_text.mid(pos, 1)))
        pos++;

    moveCursorFinished(pos);
}

void NovelAI::moveToParaNextEnd(int x)
{
    int len = _text.length();
    int pos = _text.indexOf("\n", x);
    if (pos == -1) pos = len;
    while (pos < len && isBlankChar(_text.mid(pos, 1)))
        pos++;
    pos = _text.indexOf("\n", pos);
    if (pos == -1)
        pos = len;

   moveCursorFinished(pos);
}

void NovelAI::moveCursorFinished(int pos)
{
    _pos = pos;
    _text_cursor.setPosition(_pos);
    _edit->setTextCursor(_text_cursor);
    if (isMove())
        updateRecorderPosition(_pos);
}

void NovelAI::recorderOperator()
{
    ; // 自类继承
}


void NovelAI::updateRecorderPosition(int x)
{
    Q_UNUSED(x);
    ; // 子类继承
}

void NovelAI::updateRecorderScroll()
{
    ; // 子类继承
}

void NovelAI::updateRecorderSelection()
{
    ; // 子类继承
}

void NovelAI::cutRecorderPosition()
{
    ; // 子类继承
}

void NovelAI::onlyDeleteText(int start, int end)
{
    prepareAnalyze();
    deleteText(start, end);
    finishAnalyze();
}

void NovelAI::scrollToEditEnd()
{
    ; // 子类继承，滚动到底部上次的位置
}

void NovelAI::scrollCursorFix()
{
    ; // 子类继承，固定光标所在行的位置
}

void NovelAI::cursorRealChanged(int pos)
{
    ; // 子类继承，移动光标动画
}

bool NovelAI::isAtEditEnd()
{
    int pos = _edit->textCursor().position();
    QString text = _edit->toPlainText();
    int len = text.length();
    while (pos < len)
    {
        if (isBlankChar(text.mid(pos, 1)) || isSymPairRight(text.mid(pos, 1)))
            pos++;
        else break;
    }
    if (pos == len)
        return true;
    return false;
}

void NovelAI::makeInputAnimation()
{
    if (!us->input_animation || _dif <= 0 || _dif > 20) return ;
    if (isBlankChar(_left1)) return ;
    int diff = _dif;
    int position = _pos - diff;


    // 分开设置每个文字的动画
    for (int i = 0; i < diff; i++)
    {
        int pos = position + i;
        QTextCursor cursor = _edit->textCursor();
        cursor.setPosition(pos);
        QRect rect = _edit->cursorRect(cursor);
        QString str = _text.mid(pos, 1);
        // isNotInput(); // 针对 setTextCursor 做出的 // 不过现在没必要了
        input_manager->addInputAnimation(rect.topLeft(), str, pos, 0, 100+i*20);
    }
}

void NovelAI::setTextInputColor()
{
    // 子类继承
}

bool NovelAI::prepareAnalyze()
{
    if (_flag_is_not_input) { // 非用户改变，不做任何修改
        _flag_is_not_input = false;
        _text = _edit->toPlainText();
        _text_cursor = _edit->textCursor();
        return false;
    }

    // 判断有没有改变的标记
    _change_text = _change_pos = false;
    is_editing_end_backup = is_editing_end;

    // 上一次的内容，用来和这一次的内容进行对比，判断修改了什么地方
    _pre_text = _text;
    _pre_pos = _pos;

    // 设置本次分析所用全局变量的值
    _text = _edit->toPlainText();
    _dif = _text.length() - _pre_text.length();
    _text_cursor = _edit->textCursor();
    _pos = _text_cursor.position();
    if (_dif < 0) return false; // 如果内容是删除的则取消分析
    QScrollBar* scrollbar = _edit->verticalScrollBar();
    _scroll_pos = scrollbar->sliderPosition();

    // 光标附近位置的汉字
    updateCursorChars();
    isInQuotes = isCursorInQuote(_text, _pos);

    // 动画效果
    makeInputAnimation();
    edit_range_start = edit_range_end = -1;

    return true;
}

void NovelAI::textAnalyze()
{
    // prepare 失败是因为 _flag_is_not_input 标记，表示已经准备过一次了，不需要再次准备
    if (!prepareAnalyze()) return ;

    if (_dif > 20 || _dif <= 0) return ;

    if (_dif == 1) { // 只相差一个字
        if (!isChinese(_left1)) { // 是标点
            if (us->punc_cover) { // 标点覆盖
                if (PuncCover()) {
                    return finishAnalyze();
                }
            }
            if (us->pair_match) { // 括号匹配
                if (PairMatch()) {
                    return finishAnalyze();
                }
            }
            if (us->pair_jump) { // 括号跳转
                if (PairJump()) {
                    return finishAnalyze();
                }
            }
        }
    }

    if (isChinese(_left1)) {
        if (us->auto_punc) { // 自动添加标点
            if (AutoPunc()) {
                return finishAnalyze();
            }
        }
        if (us->homonym_cover) { // 同音词覆盖
            if (HomonymCover(_pos, _dif)) {
                return finishAnalyze();
            }
        }
    }

    finishAnalyze();
}

void NovelAI::finishAnalyze()
{
    pre_scroll_bottom_delta_by_analyze = pre_scroll_bottom_delta;
    pre_scroll_viewpoer_top_by_analyze = pre_scroll_viewport_top;
    // 文本改变，保存文本
    if (_change_text) {
        //isNotInput(); // 避免死循环或者重复处理？ // 注释掉这句话，因为输入动画产生的无法analyze可以被强制取消掉
        _flag_is_not_scrolled = true;
        _edit->setPlainText(_text);
        setTextInputColor(); // 能够避免一些 input_animation 的白屏现象
        is_editing_end = is_editing_end_backup;
        _pre_changed_pos = _pos;
        recorderOperator(); // textChanged() 里面不判断，则手动进行判断
    }
    // 光标位置改变
    if (_change_text || _change_pos) {
        _text_cursor.setPosition(_pos);
        _flag_is_cursor_change = true; // FLAG 守护……
        _edit->setTextCursor(_text_cursor);
        _flag_is_cursor_change = false;
        updateRecorderPosition(_pos);
    }

    // 界面滚动的位置
    if (us->scroll_bottom_fixed && is_editing_end_backup) {
        pre_scroll_bottom_delta = pre_scroll_bottom_delta_by_analyze;
        scrollToEditEnd();
    } else if (us->scroll_cursor_fixed) {
        pre_scroll_viewport_top = pre_scroll_viewpoer_top_by_analyze;
        scrollCursorFix();
    }
    else {
        QScrollBar* scrollbar = _edit->verticalScrollBar();
        scrollbar->setSliderPosition(_scroll_pos);
        _edit->ensureCursorVisible();
    }
    updateRecorderScroll();

    // 一些收尾工作，比如动画
    if (_change_text || _change_pos) {
        cursorRealChanged(); // 放到后面是因为界面要滚动，得先固定
    }
    if (_change_text) {
        if (us->input_animation)
            input_manager->updateRect(edit_range_start, edit_range_end);
    }

    _change_text = _change_text = false;
}

void NovelAI::moveCursor(int x)
{
    _pos += x;
    if (_pos < 0) _pos = 0;
    if (_pos > _text.length()) _pos = _text.length();
    _change_pos = true;
}

void NovelAI::insertText(int pos, QString text)
{
    if (pos < 0) pos = 0;
    if (pos > _text.length()) pos = _text.length();

    _text = _text.left(pos) + text + _text.right(_text.length()-pos);

    if (_pos >= pos) {
        _pos += text.length();
        _change_pos = true;
    }

    _change_text = true;

    if (us->input_animation)
        input_manager->textChanged(pos, text.length());
}

void NovelAI::insertText(QString text)
{
    insertText(_pos, text);
}

void NovelAI::deleteText(int start, int end)
{
    if (start < 0) start = 0;
    if (end > _text.length()) end = _text.length();
    if (end < start) {
        start = start + end;
        end = start - end;
        start = start - end;
    }

    _text = _text.left(start) + _text.right(_text.length()-end);

    if (start < _pos)
    {
        if (end <= _pos) _pos -= (end-start);
        else /*if (end > _pos)*/ _pos = start;
        _change_pos = true;
    }

    _change_text = true;
    if (us->input_animation)
        input_manager->textChanged(end, start-end);
}

void NovelAI::setSelection(int start, int end, int pos)
{
    if (start == -1 || end == -1) return ;
    _text_cursor = _edit->textCursor();
    if (pos == start) // pos != -1
        start = end;
    if (pos == -1) // pos==-1时,start=start
        pos = end;
    _text_cursor.setPosition(start, QTextCursor::MoveAnchor);
    _text_cursor.setPosition(pos, QTextCursor::KeepAnchor);
    _edit->setTextCursor(_text_cursor);
}
