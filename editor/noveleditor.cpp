#include "noveleditor.h"

NovelEditor::NovelEditor(QWidget * parent)
        : QTextEdit(parent)
{
    initEditor();
}

NovelEditor::NovelEditor(QWidget* parent, QString novel_name, QString chapter_name, QString full_chapter_name)
    : QTextEdit(parent)
{
    initEditor();

    openChapter(novel_name, chapter_name, full_chapter_name);
}

void NovelEditor::initEditor()
{
    // ==== 初始化按键和内容的flag ====
    ctrling = false;
    shifting = false;
    alting = false;
    changed_by_redo_undo = false;
    _flag_dont_save = true;
    last_operator = OPERATOR_NULL;
    completer_case = 0;
    is_editing_end = false;
    pre_scroll_position = 0;
    pre_scroll_bottom_delta = 0; // 没必要
    pre_scroll_bottom_delta_by_analyze = -1;
    pre_scroll_viewpoer_top_by_analyze = -1;
    document_height = -1;
    editor_height = -2;  // textChange 里面有个 document_height+viewport_margin_bottom <= editor_height，不能一开始成立
    viewport_margin_bottom = 0;
    _flag_is_not_range_changed = false;
    pre_scrollbar_max_range = 0;
    pre_scroll_viewport_top = 0;
    _flag_should_update = false;  // 不需要更新编辑器样式
    _flag_user_change_cursor = false; // 用户修改光标为假

    // ==== 初始化编辑器 ====
    setUndoRedoEnabled(false);
    setAcceptRichText(false);


    // ==== 初始化界面 ====
    QString trans("QTextEditor, NovelEditor{ border:none;}"); // background-color: transparent;
    trans += "";
            /*NovelEditor QScrollBar::vertical {margin: 15px 0px 15px 0px;background-color: rgb(150, 150, 150, 0); border: 0px; width: 15px;}\
            //       NovelEditor QScrollBar::handle:vertical {background-color: rgb(128,128,128,80);}\
            //       NovelEditor QScrollBar::handle:vertical:hover {background-color: rgb(128,128,128,150);}\
            //       NovelEditor QScrollBar::handle:vertical:pressed {background-color: rgb(128,128,128,200); }\
            //NovelEditor QScrollBar::sub-page:vertical {background-color: orange; }
            //NovelEditor QScrollBar::add-page:vertical {background-color: blue; }
            //NovelEditor QScrollBar::sub-line:vertical { border: 0px solid orange; height:15px}
            //NovelEditor QScrollBar::add-line:vertical { margin:30px 0px 30px 0px; border: 0px solid blue; height:15px}
            //NovelEditor QScrollBar::up-arrow:vertical { background-color: rgb(200,20,220,100); }
            //NovelEditor QScrollBar::down-arrow:vertical { image: url(:/resource/down_arrow.PNG); }*/
    setStyleSheet(trans);

    if (isFileExist(rt->STYLE_PATH + "edit.qss"))
        setStyleSheet(readTextFile(rt->STYLE_PATH + "edit.qss"));

    if (isFileExist(rt->STYLE_PATH + "scrollbar.qss"))
        verticalScrollBar()->setStyleSheet(readTextFile(rt->STYLE_PATH + "scrollbar.qss"));

    // ==== 设置信号槽 ====
    connect(this, SIGNAL(textChanged()), this, SLOT(textChange()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorChange()));
    connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(copyAva()));

    // ==== 初始化撤销重做 ====
    initUndoRedo();
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollChange()));
    connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(resetScrollBarRange()));

    // ==== 设置自动提示 ====
    completer = new QCompleter(this);
    completer_listview = new QListView(this);
    completer->setPopup(completer_listview); // 设置自己的ListView，以便可以设置palette
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWidget(this);
    completer->popup()->setMinimumWidth(50);
    completer->popup()->setMaximumWidth(QApplication::desktop()->screenGeometry().width());
    completer->popup()->hide();
    connect(completer, SIGNAL(activated(QString)), this, SLOT(slotOnCompleterActived(QString)));

    completer_timer_timeout = true;
    completer_timer = new QTimer(this);
    completer_timer->setInterval(1000);
    completer_timer->setSingleShot(true);
    connect(completer_timer, SIGNAL(timeout()), this, SLOT(slotCompleterShowed()));

    // ==== 长按左键显示光标所在位置（不含空白符） ====
    mouse_press_timer = new QTimer(this);
    mouse_press_timer->setInterval(500);
    mouse_press_timer->setSingleShot(true);
    connect(mouse_press_timer, &QTimer::timeout, [this]{
        mouse_press_timer_timeout = true;
        if (textCursor().hasSelection()) return ;
        int pos = textCursor().position();
        int real_pos = pos;
        for (int i = 0; i < pos; i++)
            if (isBlankChar(_text.mid(i, 1)))
                real_pos--;
        emit signalWordCount(real_pos);
    });

    document_height_timer = new QTimer(this);
    document_height_timer->setInterval(200);
    document_height_timer->setSingleShot(true);
    connect(document_height_timer, &QTimer::timeout, [=]{
        document_height = static_cast<int>(document()->size().height());
        resetScrollBarRange();
    });

    // ==== 初始化用户自定义UI ====
    editor_cursor = new EditorCursor(this, this); // 必须要在 updateUI 之前

    updateUI();
}

void NovelEditor::updateUI()
{
    // 设置颜色 edit
    QPalette palette = this->palette();
    if (us->editor_font_color.alpha() > 0) // 只有在第一次的时候有效诶……后面就不知道为什么无效了 // BUG
    {
        palette.setColor(QPalette::Text, us->editor_font_color); // 虽然这句话并没有什么用……但是可以设置光标颜色……

        /* 内牛满面！！！研究了一整天的 bug，终于好了！！！当我把 QPlainTextEdit 换成 QTextEdit 时……
        int cursor_pos = textCursor().position();
        int select_start = -1, select_end = -1;
        // int scroll_pos = verticalScrollBar()->sliderPosition(); // 暂时用不到
        if (this->textCursor().hasSelection())
        {
            select_start = this->textCursor().selectionStart();
            select_end = this->textCursor().selectionEnd();
        }
        selectAll();
        palette.setColor(QPalette::Text, us->editor_font_color); // 虽然这句话并没有什么用……但是可以设置光标颜色……
        // palette.setColor(QPalette::WindowText, us->editor_font_color);
        QTextCharFormat f;// = textCursor.charFormat();
        f.setForeground(QBrush(us->editor_font_color));
        QTextCursor textCursor = this->textCursor();
        textCursor.setCharFormat(f);
        textCursor.setPosition(cursor_pos);
        if (select_start > -1 && select_end > -1)
        {
            if (cursor_pos == select_start)
                select_start = select_end;
            textCursor.setPosition(select_start, QTextCursor::MoveAnchor);
            textCursor.setPosition(cursor_pos, QTextCursor::KeepAnchor);
        }
        setTextCursor(textCursor);
        // verticalScrollBar()->setSliderPosition(scroll_pos); // 不需要也行的*/
    }
    palette.setColor(QPalette::Base, us->editor_bg_color);
    if (us->editor_font_selection.alpha() > 0)
        palette.setColor(QPalette::HighlightedText, us->editor_font_selection);
    palette.setColor(QPalette::Highlight, us->editor_bg_selection);
    this->setPalette(palette);

    // 如果开启了输入动画，则要重新设置一下所有的文字颜色
    if (us->input_animation)
    {
        int ori_position = this->textCursor().position();
        this->selectAll();
        QTextCursor tc = this->textCursor();
        QTextCharFormat fm = tc.charFormat();
        fm.setForeground(QBrush(us->editor_font_color));
        tc.setCharFormat(fm);
        tc.setPosition(ori_position);
        this->setTextCursor(tc);
    }

    // 设置自动提示的颜色 completer
    QPalette palette2 = completer_listview->palette();
    if (us->editor_font_color.alpha() > 0)
        palette2.setColor(QPalette::Text, us->editor_font_color);
    palette2.setColor(QPalette::Base, us->editor_bg_color);
    if (us->editor_font_selection.alpha() > 0)
        palette2.setColor(QPalette::HighlightedText, us->editor_font_selection);
    palette2.setColor(QPalette::Highlight, us->editor_bg_selection);
    completer_listview->setPalette(palette2);
    completer->setMaxVisibleItems(us->completer_maxnum);

    // 设置字体
    QFont font(this->font());
    //font.setFamily("Courier");
    font.setPointSize(us->editor_font_size);
    //font.setFixedPitch(true);
    this->setFont(font);

    // 设置光标粗细
    this->setCursorWidth(us->editor_cursor_width);
    editor_cursor->styleChanged(cursorWidth(), fontMetrics().height(), us->editor_font_color);
    input_manager->setColor(us->editor_font_color);

    /* // 设置行间字体（无效 或者 闪退） formt
    QTextCursor cursor = this->textCursor();
    QTextBlockFormat textBlockFormat;
    textBlockFormat.setLineHeight(70, QTextBlockFormat::FixedHeight); // 设置固定行高
    textBlockFormat.setBottomMargin(10); // 设置两行之间的空白高度
    textBlockFormat.setProperty(QTextFormat::OutlinePen, 10);
    cursor.setBlockFormat(textBlockFormat);
    this->setTextCursor(cursor);*/

    // 设置光标所在行的背景颜色 line
    QList<QTextEdit::ExtraSelection> es;
    QTextEdit::ExtraSelection s;
    QColor line_color = us->editor_bg_line;
    s.format.setBackground(line_color);
    s.format.setProperty(QTextFormat::FullWidthSelection, true);
    s.cursor = this->textCursor();
    s.cursor.clearSelection();
    es.append(s);
    this->setExtraSelections(es);
}

void NovelEditor::updateWordCount()
{
    emit signalWordCount(getWordCount(_text));
}

void NovelEditor::slotEditorBottomSettingsChanged()
{
    if (us->scroll_bottom_blank <= 0)
        ;
    else if (us->scroll_bottom_blank <= 10)
        viewport_margin_bottom = editor_height/us->scroll_bottom_blank;
    else
        viewport_margin_bottom = us->scroll_bottom_blank;

    resetScrollBarRange();
}

void NovelEditor::updateEditorMarked()
{
    _flag_should_update = true;
}

/**
 * 更新一些来自其他editor的修改信息，比如字体大小、名片库颜色等
 * @brief NovelEditor::updateEditor
 */
void NovelEditor::updateEditor()
{
    if (_flag_should_update == false)
        return ;
    _flag_should_update = false;

    QFont font = this->font();
    font.setPointSize(us->editor_font_size);
    this->setFont(font);
}

bool NovelEditor::isSaved()
{
    return editing.getSavedText() == this->toPlainText();
}

bool NovelEditor::save()
{
    if (editing.getPath() == "") return false;

    writeTextFile(editing.getPath(), this->toPlainText());

    return true;
}

void NovelEditor::openChapter(QString novel_name, QString chapter_name, QString full_chapter_name)
{
    if (!isSaved()) save();

    startOpenFile();

    // 保存章节信息
    // QString path = NOVEL_PATH + fnEncode(novel_name) + "/chapters/" + fnEncode(chapter_name) + ".txt";
    editing.setNames(novel_name, chapter_name);
    //editing.setPath(path);
    editing.setFullChapterName(full_chapter_name);

    // 读取文本内容
    readChapter();

    endOpenFIle();

    // 先进行撤销重做
    initUndoRedo();
    initAnalyze(static_cast<QTextEdit*>(this));

    // emit signalWordCount(getWordCount(_text)); // 在 stackWidget 中 createEdtor 后，此时还没进行 connect
    completer_timer_timeout = true;

    // 第一次cursorchanged事件：打开章节
    // 第二次cursorchanged事件：读取光标位置（启动后第一次打开）
    // 所以需要这个定时器来关闭，或者在光标改变后 closeCompleteOnce
    /*QTimer* timer = new QTimer(this);
    timer->setInterval(100);
    timer->setSingleShot(true);
    connect(timer, QTimer::timeout, [this]{
        completer_timer_timeout = true;
    });
    timer->start();*/

    // 不满一页的时候把光标聚焦到最后面（全空除外）
    // 还有各种延时操作
    QTimer* open_cursor_timer = new QTimer(this);
    open_cursor_timer->setInterval(10);
    open_cursor_timer->setSingleShot(true);
    connect(open_cursor_timer, &QTimer::timeout, [=]{
        QTextDocument* document = this->document();
        if (document)
        {
            document_height = static_cast<int>(document->size().height());
            if (document_height < editor_height) // 可以聚焦到末尾
            {
                QTextCursor cursor = textCursor();
                QString text = toPlainText();
                int len = text.length();
                int pos = len;
                while (pos > 0 && isBlankChar(text.mid(pos-1, 1)))
                    pos--;
                if (pos <= 0 || text.mid(pos-1, 1) == "\n" || text.mid(pos-1, 1) == "\r") // 行首
                {
                    while (pos < len && isBlankChar(text.mid(pos, 1)))
                        pos++;
                }
                cursor.setPosition(pos);
                setTextCursor(cursor); // 带有当前行高亮
                updateRecorderPosition(pos);
                cursorChangedByUser();
                pre_scroll_bottom_delta_by_analyze = -1;
                pre_scroll_viewpoer_top_by_analyze = -1;
                resetScrollBarRange(); // 判断一开始需不需要滚动条
                completer_timer_timeout = true;
            }
            else
            {
                highlightCurrentLine();
                resetScrollBarRange();
            }
            cursorChangedByUser();
            cursorRealChanged();
        }
        else
        {
            highlightCurrentLine();
        }

        delete open_cursor_timer;
    });
    open_cursor_timer->start();

}

void NovelEditor::readChapter()
{
    if (editing.getPath() == "") return ;

    //QTextCursor cursor = this->textCursor();
    QString text = readExistedTextFile(editing.getPath());

    // 判断是不是全空的，如果是，添加段首空行
    if (text.isEmpty())
    {
        int x = us->indent_blank;
        QString ins = "";
        while (x--)
            ins += "　";
        text = ins;
    }

    editing.setSavedText(text);
    this->setPlainText(text);
    //cursor.insertText(text);
    //this->setTextCursor(cursor);
}

void NovelEditor::beforeClose()
{
    if (!isSaved()) save();
}

NovelEditingInfo& NovelEditor::getInfo()
{
    return this->editing;
}

void NovelEditor::surroundWordSearch()
{
    if (!focusing) return ;
    int pos = textCursor().position();
    QString text = toPlainText();
    int len = text.length();
    int left = pos, right = pos;
    while (left > 0 && (isChinese(text.mid(left-1, 1)) || isEnglish(text.mid(left-1, 1)) ))
        left--;
    while (right < len && (isChinese(text.mid(right, 1)) || isEnglish(text.mid(right, 1))) )
        right++;
    if (left == right) // 如果是空的
    {
        completer->popup()->hide();
        return ;
    }
    QString sent = text.mid(left, right-left);

    if (lexicon_AI->surroundSearch(sent, pos-left))
    {
        QStringList list = lexicon_AI->getResult();
        QString key = lexicon_AI->getMatched();

        while (list.removeOne(key)); // 移除当前已经输入的
        QStringListModel* model = new QStringListModel(list);
        completer->setModel(model);
        int width = completer->popup()->sizeHintForColumn(0)+completer->popup()->verticalScrollBar()->sizeHint().width();
        QRect cr = cursorRect();
        cr.setWidth(width+10);
        completer->complete(cr);
        completer->popup()->scrollToTop();
        completer_case = COMPLETER_CONTENT_WORD;
    }
    else
    {
        completer->popup()->hide();
    }
}

void NovelEditor::closeCompleteTimerOnce()
{
    completer_timer_timeout = true;
}

void NovelEditor::operatorHomeKey()
{
    QString text = toPlainText();
    QTextCursor cursor = textCursor();
    int ori_pos = cursor.position();
    int len = text.length();
    bool block_start = false;  // 行首（真正没有空字符的开头）
    bool blank_start = false; // 段落开头（段落空字符开头）

    // 判断当前光标前面是不是空行或者空字符串
    if (ori_pos == 0 || text.mid(ori_pos-1, 1) == "\n")
        block_start = true;

    // 是否是段落
    int temp_pos = ori_pos;
    while (temp_pos > 0 && isBlankChar2(text.mid(temp_pos-1, 1)))
        temp_pos--;
    if (temp_pos == 0 || text.mid(temp_pos-1, 1) == "\n")
        blank_start = true;

    // 移动到开头
    cursor.movePosition(QTextCursor::StartOfLine);

    // 如果本来就在行首，则移动到后面非空白符的位置
    if (!blank_start || block_start)
    {
        int pos = cursor.position();
        while (pos < len && isBlankChar2(text.mid(pos, 1)))
            pos++;
        cursor.setPosition(pos);
    }
    if (cursor.position() == ori_pos) // 位置没变，表示本身就是开头
    {
        int temp_pos = cursor.position();
        while (temp_pos > 0 && text.mid(temp_pos-1, 1) != "\n")
            temp_pos--;
        while (temp_pos < len && isBlankChar2(text.mid(temp_pos, 1)))
            temp_pos++;
        cursor.setPosition(temp_pos);
    }

    setTextCursor(cursor);
}

void NovelEditor::slotCompleterShowed()
{
    if (!completer_timer_timeout)
    {
        completer_timer_timeout = true;
        if (textCursor().hasSelection())
        {
            completer->popup()->hide();
            return ;
        }
        surroundWordSearch();
    }
}

void NovelEditor::slotOnCompleterActived(const QString &completion)
{
    if ( lexicon_AI->getMatchedCase() == COMPLETER_CONTENT_SENT )//completer_case == COMPLETER_CONTENT_SENT)
    {
        if (!us->insert_sentence)
        {
            QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
            clipboard->setText(completion);		             //设置剪贴板内容</span>
            return ;
        }
    }


    if (completion == "-->") // 找到句子
    {
        //completer_case = COMPLETER_CONTENT_SENT;
        QStringList list = lexicon_AI->getSentence();
        QStringListModel* model = new QStringListModel(list);
        completer->setModel(model);
        int width = completer->popup()->sizeHintForColumn(0)+completer->popup()->verticalScrollBar()->sizeHint().width();
        QRect cr = cursorRect();
        cr.setWidth(width+10);
        completer->complete(cr);
        completer->popup()->scrollToTop();
        return ;
    }

    QString key = lexicon_AI->getMatched(); // 匹配到的字符，把这个字符替换成completion
    int pos = textCursor().position();
    QString text = toPlainText();
    int len = text.length();
    int left = pos, right = pos;
    while (left > 0 && (isChinese(text.mid(left-1, 1)) || isEnglish(text.mid(left-1, 1)) ))
        left--;
    while (right < len && (isChinese(text.mid(right, 1)) || isEnglish(text.mid(right, 1))) )
        right++;
    if (left == right) return ;
    int start_pos = pos - key.length();
    if (start_pos < left) start_pos = left;
    int find_pos = text.indexOf(key, start_pos);
    if (find_pos == -1) return ;
    operatorWordReplace(find_pos, find_pos+key.length(), completion);

    surroundWordSearch();

    // 改变文本后光标会再次改变……所以需要抵消下一次的显示提示框
    completer_timer_timeout = true;
}

void NovelEditor::keyPressEvent(QKeyEvent *event)
{
    last_operator = OPERATOR_KEY;
    // deb(event->key(), "editor key press event"); // 判断按下了什么键
    auto modifiers = event->modifiers();
    bool ctrl = modifiers & Qt::ControlModifier,
         shift = modifiers & Qt::ShiftModifier,
         alt = modifiers & Qt::AltModifier;

    if (/*completer &&*/ completer->popup()->isVisible())
    {
        if (modifiers == 0)
        {
            switch (event->key())
            {
            case Qt::Key_Escape:
                event->ignore();
                completer->popup()->hide();
                return ;
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Tab:
                if (completer->currentIndex().isValid() && completer->popup()->currentIndex().row()>-1)
                {
                    event->ignore();
                    return ;
                }
                break;
            case Qt::Key_Home :
            case Qt::Key_End :
                completer->popup()->hide();
                break;
            default:
                break;
            }
        }
        else
        {
            completer->popup()->hide();
        }
    }

    switch (event->key())
    {
    // ========== 方向键 ==========
    case Qt::Key_Up : // 上方向键
        if (ctrl && !shift && !alt) // 屏幕向上滚1行
        {
            int slider_position = verticalScrollBar()->sliderPosition() - verticalScrollBar()->singleStep()*3;
            if (slider_position < 0)
                slider_position = 0;
            verticalScrollBar()->setSliderPosition(slider_position);
            if (us->cursor_animation)
                editor_cursor->setStartPosition(-1);
            return ;
        }
        else if (!ctrl && !shift && alt) // 屏幕向上滚5行
        {
            int slider_position = verticalScrollBar()->sliderPosition() - editor_height/3;
            if (slider_position < 0)
                slider_position = 0;
            verticalScrollBar()->setSliderPosition(slider_position);
            if (us->cursor_animation)
                editor_cursor->setStartPosition(-1);
            return ;
        }
        else if (ctrl && shift && !alt) // 扩选
        {
            activeExpandSelection();
            return ;
        }
        else if (ctrl && alt) // 上一段
        {
            int select_start = -1, select_end = -1;
            select_start = select_end = textCursor().position();
            if (select_start == -1)
                select_start = select_end = _pos;
            if (textCursor().hasSelection())
            {
                select_start = textCursor().selectionStart();
                select_end = textCursor().selectionEnd();
            }
            if (shift && select_end == -1)
                select_end = textCursor().position();
            moveToParaPrevStart(this->textCursor().position());
            if (shift)
            {
                QTextCursor tc = textCursor();
                if (_pos < select_start) // 选区左边移动
                {
                    tc.setPosition(select_end, QTextCursor::MoveAnchor);
                    tc.setPosition(_pos, QTextCursor::KeepAnchor);
                }
                else // 选区右边移动
                {
                    tc.setPosition(select_start, QTextCursor::MoveAnchor);
                    tc.setPosition(_pos, QTextCursor::KeepAnchor);
                }
                setTextCursor(tc);
            }
            return ;
        }
        _flag_user_change_cursor = true;

        break;
    case Qt::Key_Down : // 下方向键
        if (ctrl && !shift && !alt) // 屏幕向下滚1行
        {
            int slider_position = verticalScrollBar()->sliderPosition() + verticalScrollBar()->singleStep()*3;
            if (slider_position > verticalScrollBar()->maximum())
                slider_position = verticalScrollBar()->maximum();
            verticalScrollBar()->setSliderPosition(slider_position);
            if (us->cursor_animation)
                editor_cursor->setStartPosition(-1);
            return ;
        }
        else if (!ctrl && !shift && alt) // 屏幕向下滚5行
        {
            int slider_position = verticalScrollBar()->sliderPosition() + editor_height/3;
            if (slider_position > verticalScrollBar()->maximum())
                slider_position = verticalScrollBar()->maximum();
            verticalScrollBar()->setSliderPosition(slider_position);
            if (us->cursor_animation)
                editor_cursor->setStartPosition(-1);
            return ;
        }
        else if (ctrl && shift && !alt) // 收缩选择
        {
            activeShrinkSelection();
            return ;
        }
        else if (ctrl && alt) // 下一段
        {
            int select_start = -1, select_end = -1;
            select_start = select_end = textCursor().position();
            if (select_start == -1)
                select_start = select_end = _pos;
            if (textCursor().hasSelection())
            {
                select_start = textCursor().selectionStart();
                select_end = textCursor().selectionEnd();
                if (_pos == select_start) // 如果需要从右边选到左边
                {
                    select_start = select_end;
                }
            }
            if (shift && select_start == -1)
                select_start = textCursor().position();
            moveToParaPrevStart(this->textCursor().position());
            if (shift)
            {
                QTextCursor tc = textCursor();
                if (_pos < select_start) // 选区左边移动
                {
                    tc.setPosition(select_end, QTextCursor::MoveAnchor);
                    tc.setPosition(_pos, QTextCursor::KeepAnchor);
                }
                else // 选区右边移动
                {
                    tc.setPosition(select_start, QTextCursor::MoveAnchor);
                    tc.setPosition(_pos, QTextCursor::KeepAnchor);
                }
                setTextCursor(tc);
            }
            return ;
        }
        _flag_user_change_cursor = true;
        break;
    case Qt::Key_Left : // 左方向键
    {
        if (ctrl || alt) // 向左移动一个单位
        {
            int select_start = -1, select_end = -1;
            select_start = select_end = textCursor().position();
            if (select_start == -1)
                select_start = select_end = _pos;
            if (textCursor().hasSelection())
            {
                select_start = textCursor().selectionStart();
                select_end = textCursor().selectionEnd();
            }
            if (shift && select_end == -1)
                select_end = textCursor().position();

            if (ctrl && !alt) // 左移一个单词
                moveToPreWord(this->textCursor().position());
            else if (!ctrl && alt) // 左移一个句子
                moveToSentStart(this->textCursor().position());
            else // 移动到段首
                moveToParaStart(this->textCursor().position());

            if (shift)
            {
                QTextCursor tc = textCursor();
                if (_pos < select_start) // 选区左边移动
                {
                    tc.setPosition(select_end, QTextCursor::MoveAnchor);
                    tc.setPosition(_pos, QTextCursor::KeepAnchor);
                }
                else // 选区右边移动
                {
                    tc.setPosition(select_start, QTextCursor::MoveAnchor);
                    tc.setPosition(_pos, QTextCursor::KeepAnchor);
                }
                setTextCursor(tc);
            }
            return ;
        }
        if (completer->popup()->isVisible())
        {
            if (completer->popup()->currentIndex().row() == -1)
                completer->popup()->hide();
            surroundWordSearch();
            last_operator = OPERATOR_IMD_COMPLETER;
        }
        _flag_user_change_cursor = true;
        break;
    }
    case Qt::Key_Right : // 右方向键
        if (ctrl || alt) // 向右移动一个单位
        {
            int select_start = -1, select_end = -1;
            select_start = select_end = textCursor().position();
            if (select_start == -1)
                select_start = select_end = _pos;
            if (textCursor().hasSelection())
            {
                select_start = textCursor().selectionStart();
                select_end = textCursor().selectionEnd();
                if (_pos == select_start) // 从结尾开始
                {
                    select_start = select_end;
                }
            }
            if (shift && select_end == -1)
                select_end = textCursor().position();

            if (ctrl && !alt) // 右移一个单词
                moveToNextWord(this->textCursor().position());
            else if (!ctrl && alt) // 右移一个句子
                moveToSentEnd(this->textCursor().position());
            else // 移动到段尾
                moveToParaEnd(this->textCursor().position());

            if (shift)
            {
                QTextCursor tc = textCursor();

                if (_pos < select_start) // 选区左边移动
                {
                    tc.setPosition(select_end, QTextCursor::MoveAnchor);
                    tc.setPosition(_pos, QTextCursor::KeepAnchor);
                }
                else // 选区右边移动
                {
                    tc.setPosition(select_start, QTextCursor::MoveAnchor);
                    tc.setPosition(_pos, QTextCursor::KeepAnchor);
                }
                setTextCursor(tc);
            }
            return ;
        }
        if (completer->popup()->isVisible())
        {
            if (completer->popup()->currentIndex().row() == -1)
                completer->popup()->hide();
            surroundWordSearch();
            last_operator = OPERATOR_IMD_COMPLETER;
        }
        _flag_user_change_cursor = true;
        break;
    case Qt::Key_Home :
        _flag_user_change_cursor = true;
        operatorHomeKey();
        return ;
        break;
    case Qt::Key_End :
        _flag_user_change_cursor = true;
        break;
    case Qt::Key_PageUp :
        _flag_user_change_cursor = true;
        break;
    case Qt::Key_PageDown :
        _flag_user_change_cursor = true;
        break;
    // ========== 智能标点增强键 =========
    case Qt::Key_Space : // 32 空格键
        if (us->smart_space /*&& !this->textCursor().hasSelection()*/) {
            if (this->textCursor().hasSelection()) // 用空格键删除选中文本
                this->textCursor().removeSelectedText();
            else
                activeSmartSpace();
            return ;
        }
        break;
    case Qt::Key_Enter :
    case Qt::Key_Return : // 16777220 回车键
        if (ctrl && shift) { // 添加到上一段
            if (alt) {
                if (textCursor().position() > 0 && isChinese(toPlainText().mid(textCursor().position()-1, 1)))
                    activeSentFinish();
            }
            isNotMove();
            moveToParaStart2(this->textCursor().position());
            activeRetrevEnter();
            /*moveToParaPrevEnd(this->textCursor().position());
            if (_pos == 0) // 表示当前的是第一段
                activeNormalEnterInsertByFirstPara();
            else
                activeNormalEnter();*/
            return ;
        }
        else if (ctrl) { // 行末添加到下一段
            if (alt)
                activeSentFinish();
            if (textCursor().position() < toPlainText().length() && toPlainText().mid(textCursor().position(), 1) != "\n")
            {
                isNotMove();
                moveToParaEnd(this->textCursor().position());
            }
            activeNormalEnter();
            activeParaSplit(this->textCursor().position());
            return ;
        }
        else if (shift) { // 单纯换行
            break;
        }
        else if (us->smart_enter) {
            if (this->textCursor().hasSelection()) // 智能回车前删除选中文本
                this->textCursor().removeSelectedText();
            activeSmartEnter();
            activeParaSplit(this->textCursor().position());
            return ;
        }
        break;
    case Qt::Key_Backspace : // 16777219
        if (textCursor().hasSelection() || shift) break;
        if (us->smart_backspace) {
            if (ctrl && !alt)
                backspaceWord(textCursor().position());
            else if (!ctrl && alt) // 删除句子
            {
                int select_start = -1, select_end = -1;
                getSent(_text, textCursor().position(), select_start, select_end);
                if (select_end > select_start)
                    onlyDeleteText(select_start, select_end);
                else // 无法选中句子
                    backspaceWord(textCursor().position());
            }
            else if (ctrl && alt) // 删除段落
            {
                int select_start = -1, select_end = -1;
                getPara(_text, textCursor().position(), select_start, select_end);
                if (select_end > select_start)
                    onlyDeleteText(select_start, select_end);
                else // 无法选中段落
                    backspaceWord(textCursor().position());
            }
            else
            {
                activeSmartBackspace();
            }
            return ;
        }
        break;
    case Qt::Key_Delete : // 向右删除
        if (textCursor().hasSelection() || shift) break;
        if (us->smart_backspace) {
            if (ctrl && !alt)
            {
                deleteWord(textCursor().position());
                return ;
            }
            else if (!ctrl && alt) // 删除句子
            {
                int select_start = -1, select_end = -1;
                getSent(_text, textCursor().position()+1, select_start, select_end);
                if (select_end > select_start)
                    onlyDeleteText(select_start, select_end);
                else // 无法选中句子
                    deleteWord(textCursor().position());
                return ;
            }
            else if (ctrl && alt) // 删除段落
            {
                int select_start = -1, select_end = -1;
                getPara(_text, textCursor().position()+1, select_start, select_end);
                if (select_end > select_start)
                    onlyDeleteText(select_start, select_end);
                else // 无法选中段落
                    deleteWord(textCursor().position());
                return ;
            }
            else if (!ctrl && !shift && !alt)
            {
                operatorSmartDelete();
            }
        }
        break;
    case Qt::Key_Tab : // tab补全、tab跳过
        if (!shift)
        {
            if (us->tab_complete && us->tab_skip)
            {
                operatorTabSkip(operatorTabComplete());
                return ;
            }
            if (us->tab_skip)
            {
                operatorTabSkip(false);
                return ;
            }
        }
        else // shift
        {
            if (us->tab_skip)
            {
                operatorReverseTabSkip();
                return ;
            }
        }
        break;
    case Qt::Key_Backtab : // shift + tab
        if (us->tab_skip)
        {
            operatorReverseTabSkip();
            return ;
        }
        break;
    // ========== 快捷键 ===========
    case Qt::Key_T :
        if (ctrl) { // ctrl+T 排版 typography
            operatorTypeset();
            return ;
        }
        break;
    case Qt::Key_D :
        if (ctrl) { // ctrl+D 强制覆盖 homonym
            activeHomonymCover();
            return ;
        }
        break;
    case Qt::Key_Escape : // ESC
    case Qt::Key_Close :
        break;
    case Qt::Key_New :
        break;
    case Qt::Key_Open :
        break;
    case Qt::Key_S :
    case Qt::Key_Save :
        save();
        break;
    case Qt::Key_Undo :
        undoOperator();
        return ;
    case Qt::Key_Redo :
        redoOperator();
        return ;
    case Qt::Key_Z :
        if (ctrl) {
            undoOperator();
            return ;
        }
        break;
    case Qt::Key_Y :
        if (ctrl) {
            redoOperator();
            return ;
        }
        break;
    case 16777248 : // shift
        shifting = true;
        break;
    case 16777249 : // ctrl
        ctrling = true;
        break;
    case 16777251 : // alt
        alting = true;
        break;
    }

    QTextEdit::keyPressEvent(event);
}

void NovelEditor::keyReleaseEvent(QKeyEvent *event)
{
    last_operator = OPERATOR_KEY;
    switch (event->key())
    {
    case Qt::Key_Up :
    case Qt::Key_Down :
    case Qt::Key_Left :
    case Qt::Key_Right :
    case Qt::Key_Home :
    case Qt::Key_End :
    case Qt::Key_PageUp :
    case Qt::Key_PageDown :
        updateRecorderPosition(textCursor().position());
        cursorChangedByUser();
        _flag_user_change_cursor = false;
        break;
    case Qt::Key_Return :
    case Qt::Key_Enter :
        is_editing_end = isAtEditEnd();
        //if (us->cursor_animation)
            //cursorRealChanged(); // 在这里用，会导致撤销时光标位置出问题……
        break;
    case 16777248 : // shift
        shifting = false;
        break;
    case 16777249 : // ctrl
        ctrling = false;
        break;
    case 16777251 : // alt
        alting = false;
        break;
    }

    QTextEdit::keyReleaseEvent(event);
}

/**
 * 输入中文事件
 * 也只能检测到中文或者中文标点，数字、字母、半角标点、换行等全都检测不到
 */
void NovelEditor::inputMethodEvent(QInputMethodEvent *event)
{
    last_operator = OPERATOR_CN;
    QString in = event->commitString();

    if (in == "“" || in == "”") {
        if (us->smart_quotes) {
            if (this->textCursor().hasSelection())
                activeSmartQuotes2();
            else
                activeSmartQuotes();
            return ;
        }
    }
    else if (in == "‘" || in == "’") {
        if (us->change_apos) {
            if (this->textCursor().hasSelection())
                activeSmartQuotes2();
            else
                activeSmartQuotes();
            return ;
        }
    }

    // 输入文字提示
    delayCompleter(us->completer_input_delay);
    QTextEdit::inputMethodEvent(event);
}

void NovelEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
    last_operator = OPERATOR_MOUSE;

    QTextEdit::mouseDoubleClickEvent(e);
}

void NovelEditor::mouseMoveEvent(QMouseEvent *e)
{

    QTextEdit::mouseMoveEvent(e);
}

void NovelEditor::mousePressEvent(QMouseEvent *e)
{
    last_operator = OPERATOR_MOUSE;
    _flag_user_change_cursor = true;

    if (ctrling)
    {

    }
    mouse_press_timer_timeout = false;
    mouse_press_timer->start();
    QTextEdit::mousePressEvent(e);
}

void NovelEditor::mouseReleaseEvent(QMouseEvent *e)
{
    last_operator = OPERATOR_MOUSE;
    _flag_user_change_cursor = false;

    QTextCursor textCursor = this->textCursor();
    int position = textCursor.position();
    updateRecorderPosition(position);
    cursorChangedByUser();

    if (mouse_press_timer_timeout)
    {
        mouse_press_timer_timeout = false;
        if (!textCursor.hasSelection())
        {
            emit signalWordCount(getWordCount(_text));
        }
    }
    else
    {
        mouse_press_timer->stop();
    }

    QTextEdit::mouseReleaseEvent(e);
}

void NovelEditor::wheelEvent(QWheelEvent *e)
{
    // 隐藏自动提示
    if (completer->popup()->isVisible() && completer->popup()->currentIndex().row() < 0)
    {
        completer->popup()->hide();
    }

    if (ctrling && !shifting) // 修改字体大小
    {
        if (e->delta() > 0) // 滚轮往前转
        {
            if (us->editor_font_size > 1)
            {
                if (us->editor_font_size > 30)
                    us->editor_font_size *= 1.1;
                else
                    us->editor_font_size++;
                us->setVal("editor_font_size", us->editor_font_size);
                this->updateUI();
                emit signalUpdateAllEditor();
                //this->zoomIn();
            }
        }
        else if (e->delta() < 0)
        {
            if (us->editor_font_size < 200)
            {
                if (us->editor_font_size > 20)
                    us->editor_font_size *= 0.8;
                else if (us->editor_font_size > 3)
                    us->editor_font_size--;
                us->setVal("editor_font_size", us->editor_font_size);
                this->updateUI();
                emit signalUpdateAllEditor();
                //this->zoomOut();
            }
        }
    }
    else if (!ctrling && shifting) // 扩大选择、收缩选择
    {
        if (e->delta() > 0) // 滚轮往前转
        {
            activeExpandSelection();
        }
        else if (e->delta() < 0)
        {
            activeShrinkSelection();
        }
    }
    else if (!ctrling)
    {
        QTextEdit::wheelEvent(e);
        if (us->cursor_animation)
            editor_cursor->setStartPosition(-1);
    }

}

void NovelEditor::showEvent(QShowEvent *e)
{
    updateEditor();

    return QTextEdit::showEvent(e);
}

void NovelEditor::resizeEvent(QResizeEvent *e)
{
    editor_height = e->size().height();
    document_height = static_cast<int>(document()->size().height());

    //document_height_timer->stop();
    //document_height_timer->start();
    if (us->scroll_bottom_blank <= 0)
        ;
    else if (us->scroll_bottom_blank == 1) // 滚动超过屏幕高度时，会出现滚动的问题（比如删除行）
        viewport_margin_bottom = static_cast<int>(editor_height*0.8);// - verticalScrollBar()->singleStep();
    else if (us->scroll_bottom_blank <= 10)
        viewport_margin_bottom = editor_height/us->scroll_bottom_blank;
    else
    {
        viewport_margin_bottom = us->scroll_bottom_blank;
        if (viewport_margin_bottom > editor_height*0.8)
            viewport_margin_bottom = static_cast<int>(editor_height * 0.8);
    }
    pre_scroll_viewport_top = cursorRect().top(); // 避免调整大小后位置的变化

    //resetScrollBarRange(); // 应该会触发范围改变事件，就不用多此一举了呢
    return QTextEdit::resizeEvent(e);
}

void NovelEditor::focusInEvent(QFocusEvent *e)
{
    focusing = true;
    QTextEdit::focusInEvent(e);
}

void NovelEditor::focusOutEvent(QFocusEvent *e)
{
    focusing = false;
    ctrling = shifting = alting = false;
    QTextEdit::focusOutEvent(e);
}

void NovelEditor::insertFromMimeData(const QMimeData *source)
{
    if (shifting && ctrling); // ctrl+shift+v 格式化粘贴
    else if (!us->typeset_paste)
        return QTextEdit::insertFromMimeData(source);

    if (source->hasText())
    {
        QString context = source->text();
        if (this->textCursor().hasSelection()) // 已经选择一部分，直接覆盖
            return QTextEdit::insertFromMimeData(source);
        int start = textCursor().position();
        int end = start + context.length();
        QTextEdit::insertFromMimeData(source);
        if (context.indexOf("\n") > -1 && !isBlankString(context))
            operatorTypesetPart(start, end);
        // typesetPaste(this->textCursor().position(), context);
    }

}

void NovelEditor::cutRecorderPosition()
{
    if (recorders_pos > 1 && recorders_pos == recorders.size())
    {
        recorders_pos--;
        recorders.removeLast();
    }
}

void NovelEditor::setTextInputColor()
{
    if (!us->input_animation) return ;
    // isNotInput(); // 已经转到 input_manager 对象里面的* _flag_is_not_input 了
    _flag_is_not_scrolled = true; // 因为里面有 setTextCursor，导致滚动条混乱
    input_manager->updateTextColor(_pos);
}

void NovelEditor::textChange()
{
    // ==== 滚动条高度调整 ====
    // 这一段之所以放到这里而不是 rangeChanged 信号槽里，是因为 rangeChanged 会经常蜜汁触发
    if (document_height + viewport_margin_bottom <= editor_height
            && (document_height = static_cast<int>(document()->size().height()))+viewport_margin_bottom>editor_height)
    {
        resetScrollBarRange();
        verticalScrollBar()->show();
    }
    document_height = static_cast<int>(document()->size().height()); // 要是用两层 if else ，避免重复获取高度，不知道能不能提高效率？

    //resetScrollBarRange();
    if (_flag_dont_save) { // 无需理会：打开新章时
        _flag_dont_save = false;
        return ;
    }

    if (toPlainText() == _text && !_change_text) // 文本没有改变（不是用户主动输入的，finishAnalyze也不算改变）
    {
        return ;
    }

    /*if (_flag_is_cursor_change)
    {
        _flag_is_cursor_change = false;
        return ;
    }*/

    // 撤销重做引起的改变。TextAnalyze中，已经加入了flag_isNotInput
    if (changed_by_redo_undo == false) {
        // 保存撤销重做
        // 注意，这个的光标位置是 0 的
        recorderOperator();
    }

    // ==== AI判断 ====
    // 如果不是 finishAnalyze 的话，则进行文本分析
    if (!_change_text)
    {
        textAnalyze();
    }

    // 自动保存
    if (us->save_power)
    {
        save();
    }

    emit signalWordCount(getWordCount(_text));

    //surroundWordSearch();
    if (/*completer &&*/ completer->popup()->isVisible())
    {
        surroundWordSearch();
        completer_timer_timeout = true;
        //completer->popup()->hide();
    }
    if (changed_by_redo_undo == false && !_change_text) {
        cursorRealChanged(_pos);
    }
}

void NovelEditor::cursorChange()
{
    if (_flag_user_change_cursor)
    {
        cursorRealChanged();
        _flag_user_change_cursor = false;
    }
    // ==== 显示选中数量的字数 ====
    if (this->textCursor().hasSelection())
    {
        QString text = textCursor().selectedText();
        int count = getWordCount(text);
        emit signalWordCount(count);
    }

    // ==== 当前行高亮 ====
    if (editor_height > -1) // 初始化结束后再显示下划线（不然打开动画很难受）
    {
        highlightCurrentLine();
    }


    /* // ==== 保存光标位置便于撤销 ====
    // 避免撤销重做引起的重新修改光标事件
    if (changedByRedoUndo == true) {
        return ;
    }
    updateRecorderPosition(textCursor.position();); // 光标改变 比 文本改变 还要早*/

    // ==== 自动提示 ====
    //if (!(completer_timer_timeout == false && completer_timer->interval() != us->completer_input_delay)) // 不是中文输入的延迟
   if (last_operator != OPERATOR_CN && last_operator != OPERATOR_NOT_COMPLETER)
   {
        if (last_operator == OPERATOR_IMD_COMPLETER) // 必须是可视的
            surroundWordSearch();
        else
            delayCompleter(us->completer_cursor_delay);
    }
}

void NovelEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> es;
    QTextEdit::ExtraSelection s;
    s.format.setBackground(us->editor_bg_line);
    s.format.setProperty(QTextFormat::FullWidthSelection, true);
    s.cursor = this->textCursor();
    s.cursor.clearSelection();
    es.append(s);
    this->setExtraSelections(es);
}

void NovelEditor::copyAva()
{
    // ==== 显示选中数量的字数 ====
    if (!textCursor().hasSelection())
    {
        emit signalWordCount(getWordCount(_text));
    }
    else
    {
        // 只在开始选中的时候保存
        // 取消选中的话，可能是输入文字，和这个有冲突
        // 一般取消文字也是需要用户手动点的
        // 所以就放到了用户修改光标的块里面
        updateRecorderSelection();
    }
}

void NovelEditor::cursorChangedByUser()
{
    // ==== 更新是否是光标最后 ====
    is_editing_end = isAtEditEnd();
    pre_scroll_position = verticalScrollBar()->sliderPosition();
    pre_scroll_bottom_delta = verticalScrollBar()->maximum()-pre_scroll_position;
    pre_scroll_viewport_top = cursorRect().top();
    updateRecorderSelection();
    if (!_edit->textCursor().hasSelection())
        selection_list.clear();
}

void NovelEditor::scrollChange()
{
    updateRecorderScroll();

    if (_flag_is_not_scrolled)
    {
        _flag_is_not_scrolled = false;
        return ;
    }

    if (is_editing_end)
    {
        pre_scroll_bottom_delta = verticalScrollBar()->maximum()-verticalScrollBar()->sliderPosition();
    }

    pre_scroll_viewport_top = cursorRect().top();

    /*if (us->cursor_animation)
    {
        editor_cursor->positionChanged(-1);
    }*/
}

void NovelEditor::resetScrollBarRange() // scrollbar range changed event
{
    if (_flag_is_not_range_changed) // 一个变量避免死循环（貌似可以设置成静态？）
    {
        _flag_is_not_range_changed = false;
        return ;
    }

    if (document_height+viewport_margin_bottom <= editor_height) // 不足一页的，不显示滚动条
        return ;

    if (us->scroll_bottom_blank) // 设置底部空白距离
    {
        _flag_is_not_range_changed = true; // 避免下面这句引发问题
        verticalScrollBar()->setRange(verticalScrollBar()->minimum(), verticalScrollBar()->minimum()+document_height-verticalScrollBar()->pageStep()+viewport_margin_bottom);
        _flag_is_not_range_changed = false;
    }

    // 玄学！！！！！！！！使用下面的语句，会导致窗口拉伸时滚动到回到 0~2的位置，注释掉后就不会有问题了……
    // 而且全文光标行固定和末尾光标行固定照样可以生效，不会出问题……
    /*if (verticalScrollBar()->maximum() != pre_scrollbar_max_range) // 是文本改变或者宽高改变引起的范围改变（哇，我真是个天才！！！（琢磨了一上午+一中午才想出来的！！！））
    {
        pre_scrollbar_max_range = verticalScrollBar()->maximum();  // 同上，通过高度的改变来判断是不是文本的变化
        if (us->scroll_bottom_fixed) // 固定尾部光标行位置
        {
            if (is_editing_end)
            {
                if (pre_scroll_bottom_delta_by_analyze == -1) // openChapter 里面设置的
                    pre_scroll_bottom_delta = verticalScrollBar()->maximum();
                else
                    pre_scroll_bottom_delta = pre_scroll_bottom_delta_by_analyze; // 恢复到 文本改变 之前
                scrollToEditEnd();

                return ;
            }
        }

        if (us->scroll_cursor_fixed) // 固定全文光标行位置
        {
            if (pre_scroll_viewpoer_top_by_analyze == -1) // openChanter 刚打开章节时是 -1
                pre_scroll_viewport_top = cursorRect().top();
            else
                pre_scroll_viewport_top = pre_scroll_viewpoer_top_by_analyze;
            scrollCursorFix();
        }
    }*/

}

void NovelEditor::cursorRealChanged(int pos)
{
    if (us->cursor_animation)
        editor_cursor->positionChanged(pos);
}

void NovelEditor::scrollToEditEnd()
{
    /*if (!isAtEditEnd())
    {
        is_editing_end = false;
        return ;
    }*/
    int pos = document_height+viewport_margin_bottom-editor_height-pre_scroll_bottom_delta;
    //if (pos < 0) return ;
    verticalScrollBar()->setSliderPosition(pos);
}

void NovelEditor::scrollCursorFix()
{
    int scroll_slide = verticalScrollBar()->sliderPosition();
    int viewport_slide = cursorRect().top();
    int delta = pre_scroll_viewport_top - viewport_slide;
    scroll_slide -= delta;
    verticalScrollBar()->setSliderPosition(scroll_slide);
}

void NovelEditor::initUndoRedo()
{
    _pos = textCursor().position();
    recorders_pos = 0;
    recorders.clear();
    recorderOperator();
}

void NovelEditor::recorderOperator()
{
    /*recorders_pos : 当前项目的下标，上次的坐标*/
    // 删除后面的
    while (recorders.size() > recorders_pos) {
        recorders.removeLast();
    }
    // 要是超过了上限，则删除多余的
    if (recorders.size() > us->editor_recorder_max)
    {
        recorders.removeFirst();
        recorders_pos--;
    }

    QString text = this->toPlainText();
    int pos = this->textCursor().position();
    if (pos == 0) pos = _pos;
    int scroll = this->verticalScrollBar()->sliderPosition();

    EditorRecorderItem item(text, pos, scroll);

    // 和上一个变化作比较，如果是同一个就合并
    // 可能会引发崩溃问题
    if (recorders_pos >= 1) // 前面有2条记录
    {
        // if (text == _text) return ; // 文字相同，当做没有改变 // 加上这句话会出bug……
        EditorRecorderItem last_item = recorders.at(recorders_pos-1);
        if (text == last_item.getText()) return ; // 文本相同，当做没有改变
        if (1/*last_item.getTime()+1000 >= item.getTime()*/)
        {
            //int last_pos = last_item.getPos();
            QString last_text = last_item.getText();
            int start, end;
            last_item.getSelection(start, end);
            if (start == end) // 没有选中内容
            {
                if (/*pos == last_pos+1 && */text.length() == last_text.length()+1) // 连续输入一个字
                {
                    int back = text.length() - pos;
                    if (text.left(pos-1) == last_text.left(pos-1) && text.right(back-1) == last_text.right(back-1))
                    {
                        if (last_item.getCase() == UNDOREDO_CASE_INPUTONE && last_item.getTime()+1000>=item.getTime())
                        {
                            recorders.removeLast();
                            // 有第一个，则说明上一个可能也是这个
                            // 继续删除上一个
                            if ( recorders_pos >= 2
                                     && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos-2))).getCase() == UNDOREDO_CASE_INPUTONE
                                     && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos-2))).getTime()+1000 > last_item.getTime())
                                recorders.removeLast();
                        }
                        item.setCase(UNDOREDO_CASE_INPUTONE);
                    }
                }
                else if (/*pos == last_pos-1 && */text.length() == last_text.length()-1) // 连续 backspace 一个字
                {
                    int back = text.length()-pos;
                    if (text.left(pos) == last_text.left(pos) && text.right(back) == last_text.right(back))
                    {
                        if (last_item.getCase() == UNDOREDO_CASE_BACKSPACEONE && last_item.getTime()+1000>=item.getTime())
                        {
                            recorders.removeLast();
                            if (recorders_pos >= 2
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos-2))).getCase() == UNDOREDO_CASE_BACKSPACEONE
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos-2))).getTime()+1000 > last_item.getTime())
                                recorders.removeLast();
                        }
                        item.setCase(UNDOREDO_CASE_BACKSPACEONE);
                    }
                }
                else if (/*pos == last_pos && */text.length() == last_text.length()-1) // 连续 delete 一个字
                {
                    int back = text.length()-pos;
                    if (text.left(pos) == last_text.left(pos) && text.right(back) == last_text.right(back))
                    {
                        if (last_item.getCase() == UNDOREDO_CASE_DELETEONE && last_item.getTime()+1000>=item.getTime())
                        {
                            recorders.removeLast();
                            if (recorders_pos >= 2
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos-2))).getCase() == UNDOREDO_CASE_DELETEONE
                                    && (static_cast<EditorRecorderItem>(recorders.at(recorders_pos-2))).getTime()+1000 > last_item.getTime())
                                recorders.removeLast();
                        }
                        item.setCase(UNDOREDO_CASE_DELETEONE);
                    }
                }
            }
        }
    }

    recorders.append(item);
    recorders_pos = recorders.size();
}

void NovelEditor::undoOperator()
{
    if (recorders_pos <= 1 || recorders.size() <= 1)
        return ;
    recorders_pos--;

    readRecorder(recorders_pos-1);
}

void NovelEditor::redoOperator()
{
    if (recorders_pos >= recorders.size())
        return ;
    recorders_pos++;

    readRecorder(recorders_pos-1);
    cursorRealChanged(); // 显示光标改变动画
}

void NovelEditor::readRecorder(int x) // x 从零开始计算（原本是从1开始）
{
    isNotInput();
    changed_by_redo_undo = true;
    EditorRecorderItem item = recorders.at(x);
    this->setPlainText(item.getText());

    QTextCursor cursor = this->textCursor();
    int position = item.getPos();
    cursor.setPosition(position);
    this->setTextCursor(cursor);
    _pos = position;

    int start, end;
    item.getSelection(start, end);

    if (start > -1 && end > -1)
    {
        if (start == _pos)
            start = end;
        cursor.setPosition(start, QTextCursor::MoveAnchor);
        cursor.setPosition(_pos, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }

    QScrollBar* scrollbar = this->verticalScrollBar();
    scrollbar->setSliderPosition(item.getScroll());
    changed_by_redo_undo = false;
    //this->ensureCursorVisible();
    cursorRealChanged(); // 显示光标改变动画
}

void NovelEditor::updateRecorderPosition(int x)
{
    // if (recorders_pos == recorders.size()) // 是最前面的位置，避免撤销后重做再撤销这种怪操作
    {
        recorders[recorders_pos-1].setPos(x);
        recorders[recorders_pos-1].setScroll(this->verticalScrollBar()->sliderPosition());
    }
    /*EditorRecorderItem item = &recorders.at(recorders.size()-1);
    if (x != item.getPos()) {
        item.setPos(x);

        QScrollBar* scrollbar = this->verticalScrollBar();
        int s = scrollbar->sliderPosition();
        if (s != item.getScroll())
            item.setScroll(s);
    }*/
}

void NovelEditor::updateRecorderScroll()
{
    if (_flag_is_not_scrolled)
    {
        _flag_is_not_scrolled = false;
        return ;
    }

    recorders[recorders_pos-1].setScroll(this->verticalScrollBar()->sliderPosition());
}

void NovelEditor::updateRecorderSelection()
{
    if (textCursor().hasSelection())
    {
        recorders[recorders_pos-1].setSelection(textCursor().selectionStart(), textCursor().selectionEnd());
    }
    else
    {
        recorders[recorders_pos-1].setSelection(-1, -1);
    }
}

void NovelEditor::startOpenFile()
{
    _flag_dont_save = true;
}

void NovelEditor::endOpenFIle()
{
    _flag_dont_save = false;
}

void NovelEditor::delayCompleter(int ms)
{
    if (!completer_timer_timeout)
    {
        completer_timer->stop();
    }
    if (us->lexicon_synonym || us->lexicon_related)
    {
        completer_timer_timeout = false;
        completer_timer->start(ms);
    }
}
