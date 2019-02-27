#include "stackwidget.h"

StackWidget::StackWidget(QWidget *parent) : QStackedWidget(parent)
{
    /*QGraphicsDropShadowEffect* e_shadow = new QGraphicsDropShadowEffect(this);
    e_shadow->setOffset(-8, 0);
    this->setGraphicsEffect(e_shadow);*/

    flag_isNew = false;
    flag_last = -1;
    open_pixmap = new QPixmap(this->size());

    this->setMinimumWidth(200);
    this->setContentsMargins(5, 0, 0, 5);
}

void StackWidget::slotOpenChapter(QString novelName, QString chapterName, QString fullChapterName)
{
    // 检查是否已经打开了
	for (int i = 0; i < editors.size(); i++)
    {
        if (editors[i]->getInfo().getNovelName() == novelName && editors[i]->getInfo().getChapterName() == chapterName)
        {
            switchEditor(i);
            flag_isNew = false;
			return ;
        }
	}

    // 关闭超过上限数量的标签
    if (editors.size() > us->tab_maxnum)
	{
        ;
	}

	// 打开新的标签页
    int now = flag_last;
    flag_isNew = false; // 需要这样才能开启动画，不知道为什么之前flag会是true
    createEditor(novelName, chapterName, fullChapterName); // 标签切换动画是在这个时候的

    // 如果是打开第一章（启动后自动打开的那一章节）（可能会有bug？）
    if (us->open_recent && editors.size() == 1
            && novelName == us->getVar("recent/novel") && chapterName == us->getVar("recent/chapter"))
    {
        slotResotrePosition();
    }

    if (us->open_chapter_animation) // 开启动画：切换回去
    {
        flag_aim = this->currentIndex();

        // 将widget的内容渲染到pixmap对象中
        parentWidget()->parentWidget()->render(open_pixmap, QPoint(0,0), geometry()); // 父类的父类，画这种背景……
        if (now != -1)
        {
            switchEditor(now);
        }
        else if (count() == 1 && !rt->is_initing ) // 打开第一个，并且不是在初始化
        { // 打开后数量还是为一的
            editors[0]->hide();
        }
    }

    us->setVal("recent/chapter", chapterName);
    us->setVal("recent/fullChapterName", fullChapterName);
    us->sync();
}

void StackWidget::slotOpenAnimation(QPoint point)
{
    if (!flag_isNew || !us->open_chapter_animation) return ;

    // 获取当前页面的widget
    //QPixmap* pixmap = new QPixmap(this->size());
    QWidget* w = editors[this->currentIndex()];

    // 将widget的内容渲染到pixmap对象中
    //((QWidget*)w->parent())->render(pixmap); // 不用父类画的话会黑背景*/

    // 绘制当前的widget
    StackWidgetAnimation* animation = new StackWidgetAnimation(this->parentWidget(), open_pixmap, point, w->pos()+this->pos(), w->size(), flag_aim);

    connect(animation, SIGNAL(signalAni1Finished(int)), this, SLOT(slotOpenAnimationFinished(int)));
}

void StackWidget::slotOpenAnimationFinished(int x)
{
    if (count() >= 1 && x == 0/*1表示下标0*/)
    {
        editors[0]->show();
    }
    switchEditor(x);
}

void StackWidget::slotDeleteChapter(QString novelName, QString chapterName)
{
    for (int i = 0; i < editors.size(); i++)
    {
        if ( editors[i]->getInfo().getNovelName() == novelName && editors[i]->getInfo().getChapterName() == chapterName )
        {
            if (currentIndex() == i)
            {
                // 先渲染再切换和删除
                parentWidget()->parentWidget()->render(open_pixmap, QPoint(0,0), geometry());

                // 切换到其他章节 // TODO 弄一个章节打开的栈
                if (currentIndex() > 0) // 切换到上一个
                {
                    switchEditor(currentIndex()-1);
                    NovelEditor* edit = editors[currentIndex()];
                    us->setVal("recent/novel", edit->getInfo().getNovelName());
                    us->setVal("recent/chapter", edit->getInfo().getChapterName());
                }
                else if (currentIndex() < editors.size()-1) // 切换到下一个
                {
                    switchEditor(currentIndex()+1);
                    NovelEditor* edit = editors[currentIndex()];
                    us->setVal("recent/novel", edit->getInfo().getNovelName());
                    us->setVal("recent/chapter", edit->getInfo().getChapterName());
                }

                // 删除章节的动画
                QRect sr(geometry());
                QRect er(sr.left()+sr.width()/2, sr.top()+sr.height()/2, 1, 1);
                /*ZoomGeometryAnimationWidget* animation = */new ZoomGeometryAnimationWidget(this->parentWidget(), open_pixmap, sr, er, -1);
            }
            this->removeWidget(editors[i]);
            delete editors.at(i);
            editors.removeAt(i);
        }
    }
}

void StackWidget::slotDeleteChapters(QString novelName, QStringList chapterNames)
{
    for (QString n : chapterNames)
    {
        slotDeleteChapter(novelName, n);
    }
}

void StackWidget::slotChangeName(bool isChapter, QString novelName, QString oldName, QString newName)
{
    if (!isChapter) return ;

    Log(oldName+" --> "+newName, "myTabWidget.slotChangeName");
    for (int i = 0; i < editors.size(); i++)
    {
        if ( editors[i]->getInfo().getNovelName() == novelName && editors[i]->getInfo().getChapterName() == oldName )
        {
            editors[i]->getInfo().setChapterName(newName);
            Log("index:"+QString("%1").arg(i), "change chapter name");
            if (i == this->currentIndex()) // 正在编辑的章节，修改程序标题
            {
                //emit signalChangeTitleByChapter(editors[i]->getInfo().getFullName());
                emit signalSetTitleFull(editors[i]->getInfo().getFullChapterName());
            }
            break;
        }
    }
}

void StackWidget::slotCloseEditor(int index)
{
	if (index >= editors.size()) return ;

    editors[index]->beforeClose();

    this->removeWidget(editors[index]);

    editors.removeAt(index);
}

void StackWidget::slotTextChanged()
{

}

void StackWidget::slotUpdateUI()
{
    for (int i = 0; i < editors.size(); i++)
    {
        editors[i]->updateUI();
    }
}

void StackWidget::slotUpdateAllEditor()
{
    for (int i = 0; i < editors.size(); i++)
    {
        editors[i]->updateEditorMarked();
    }
}

void StackWidget::slotEditorBottomSettingsChanged()
{
    for (int i = 0; i < editors.size(); i++)
    {
        editors[i]->slotEditorBottomSettingsChanged();
    }
}

void StackWidget::slotEditorWordCount(int x)
{
    emit signalEditorWordCount(x);
    // emit signalSetTitleWc(x);
}

void StackWidget::slotSavePosition()
{
    if (editors.size() == 0)
    {
        us->setVal("recent/cursor", -1);
        us->setVal("recent/scroll", -1);
        return ;
    }
    NovelEditor* edit = editors[currentIndex()];
    int pos = edit->textCursor().position();
    int scr = edit->verticalScrollBar()->sliderPosition();
    us->setVal("recent/cursor", pos);
    us->setVal("recent/scroll", scr);
    if (edit->textCursor().hasSelection())
    {
        us->setVal("recent/selection_start", edit->textCursor().selectionStart());
        us->setVal("recent/selection_end", edit->textCursor().selectionEnd());
    }
    else
    {
        us->setVal("recent/selection_start", -1);
        us->setVal("recent/selection_end", -1);
    }
    us->sync();
}

void StackWidget::slotResotrePosition()
{
    if (editors.size() == 0) return ;
    NovelEditor* edit = editors[currentIndex()];
    int pos = us->getInt("recent/cursor");
    int scr = us->getInt("recent/scroll");
    if (pos > 0 && pos <= edit->toPlainText().length())
    {
        QTextCursor text_cursor = edit->textCursor();
        text_cursor.setPosition(pos);
        int selection_start = us->getInt("recent/selection_start", -1),
            selection_end = us->getInt("recent/selection_end", -1);
        if (selection_start > -1 && selection_end > -1)
        {
            if (pos == selection_start)
            {
                int temp = selection_start;
                selection_start = selection_end;
                selection_end = temp;
            }
            text_cursor.setPosition(selection_start, QTextCursor::MoveAnchor);
            text_cursor.setPosition(selection_end, QTextCursor::KeepAnchor);
        }
        edit->setTextCursor(text_cursor);
        edit->closeCompleteTimerOnce();
    }
    if (scr > 0/* && scr < edit->verticalScrollBar()->maximum()*/)
    {
        edit->verticalScrollBar()->setSliderPosition(scr); // 后面有延迟再重新设置一遍
    }
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotDelayFocus()));
    connect(timer, SIGNAL(timeout()), this, SLOT(slotDelayScroll())); // 延迟设置
    timer->setSingleShot(true);
    timer->start(100);
    // edit->setFocus(Qt::ActiveWindowFocusReason); // 不知道为什么需要用时钟延迟一段时间才行
}

void StackWidget::slotDelayFocus()
{
    // 不知道为什么，一定要延迟一段时间才行……
    // 猜测可能是，点击列表项打开章节，当时鼠标还按在那个列表项上，焦点强行聚在列表项，编辑框获取不到
    // 也可能是因为初始化时，其他控件自动抢走了焦点
    if (editors.size() == 0) return ;
    this->setFocus(Qt::ActiveWindowFocusReason);
    if (currentIndex()<0) return ;
    editors[currentIndex()]->setFocus(Qt::ActiveWindowFocusReason);
}

void StackWidget::slotDelayScroll()
{
    // editor 触发 scrollbar 的 rangeChanged 事件需要一时间
    // 触发后调整下方的空白
    // 但是直接加载的话，下面的空白可能没有调整好，高度不够，此时只能滚动到document的位置
    // 延迟一段时间，就能在 resetScrollRange 后再进行滚动
    if (editors.size() == 0) return ;
    if (currentIndex()<0) return ;
    int scr = us->getInt("recent/scroll");
    NovelEditor* edit = editors[currentIndex()];
    edit->verticalScrollBar()->setSliderPosition(scr);
}

void StackWidget::switchEditor(int index)
{
    if (index >= this->count()) return ;
    this->setCurrentIndex(index);
    flag_last = index;
    if (!us->one_click)
        editors[index]->setFocus();

    if (flag_isNew == true) // 这个会切换三次，故延迟动画
        return ;

    // emit signalChangeTitleByChapter(editors[index]->getInfo().getFullName());
    emit signalSetTitleNovel(editors[index]->getInfo().getNovelName());
    emit signalSetTitleChapter(editors[index]->getInfo().getChapterName());
    emit signalSetTitleFull(editors[index]->getInfo().getFullChapterName());
//    emit signalSetTitleWc(editors[index]->getInfo().getWc());
//    emit signalEditorWordCount(editors[index]->getInfo().getWc());
    editors[index]->updateWordCount();
    editors[index]->setFocus();
}

void StackWidget::createEditor(QString novelName, QString chapterName, QString fullChapterName)
{
    NovelEditor* editor = new NovelEditor(this, novelName, chapterName, fullChapterName);
	editors.append(editor);

    editor->setStyleSheet("NovelEditor{background-color: transparent; border:none;}");

    //connect(editor, SIGNAL(textChanged()), this, SLOT(slotTextChanged()));
    connect(editor, SIGNAL(signalWordCount(int)), this, SLOT(slotEditorWordCount(int)));
    //editor->updateWordCount();
    connect(editor, SIGNAL(signalUpdateAllEditor()), this, SLOT(slotUpdateAllEditor()));

	this->addWidget(editor);
    Log(novelName+"/"+chapterName, "create new editor");
    switchEditor(editors.size()-1);

    flag_isNew = true;
}

void StackWidget::resizeEvent(QResizeEvent *)
{
    if (open_pixmap != nullptr) {
        ;
    }//delete open_pixmap; // 出bug就注释掉这一行
    open_pixmap = new QPixmap(this->size());
}

void StackWidget::slotRenameNovel(QString oldName, QString newName)
{
    // 是否修改了当前打开的编辑器名字
    int index = currentIndex();
    if (index > -1 && editors[index]->getInfo().getNovelName() == oldName)
    {
        emit signalSetTitleNovel(newName);
    }

    for (int i = 0; i < editors.size(); i++)
    {
        if (editors[i]->getInfo().getNovelName() == oldName)
        {
            editors[i]->getInfo().setNovelName(newName);
        }
    }
}

void StackWidget::slotDeleteNovel(QString novelName)
{
    // 是否关闭了当前打开的编辑器
    bool isSwitch = false;
    int index = currentIndex();
    if (index > -1 && editors[index]->getInfo().getNovelName() == novelName)
    {
        isSwitch = true;
    }

    for (int i = editors.size()-1; i >= 0; i--)
    {
        if (editors[i]->getInfo().getNovelName() == novelName)
        {
            slotCloseEditor(i);
        }
    }

    // 更换标题
    if (isSwitch)
    {
        if (editors.size() > 0)
        {
            switchEditor(editors.size()-1);
        }
        else
        {
            deb("title");
            emit signalChangeTitleByChapter("写作天下");
        }
    }
}
