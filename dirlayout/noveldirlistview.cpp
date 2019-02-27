#include <QWidget>
#include "noveldirlistview.h"

NovelDirListView::NovelDirListView(QWidget *parent) : QListView(parent)
{
    pre_index = -1;
    pressed_time1 = pressed_time2 = 0;
    released_time = 0;
    // 设置 delegate
    novel_dir_delegate = new NovelDirDelegate(this);
    setItemDelegate(novel_dir_delegate);

    // 设置 model
    novel_dir_model = new NovelDirModel(this);
    setModel(novel_dir_model);

    initStyle();

    initMenu();

    connect(novel_dir_model, SIGNAL(signalListDataChange()), this, SLOT(slotInitRowHidden()));
    connect(novel_dir_model, SIGNAL(signalReedit()), this,SLOT(slotReedit()));
    connect(novel_dir_model, SIGNAL(signalOpenCurrentChapter()), this, SLOT(slotOpenCurrentChapter()));
    connect(novel_dir_model, SIGNAL(signalOpenChapter(int)), this, SLOT(slotOpenChapter(int)));

    setAcceptDrops(true);

    QTimer* read_last_timer = new QTimer(this);
    read_last_timer->setInterval(100);
    read_last_timer->setSingleShot(true);
    connect(read_last_timer, &QTimer::timeout, [=]{
        int last_index = us->getInt("recent/index");
        if (last_index >= 0 && last_index < novel_dir_model->getRcCount())
        {
            setCurrentIndex(model()->index(last_index, 0));
            int pos = us->getInt("recent/dir_scroll", -1);
            if (pos >= 0)
                verticalScrollBar()->setSliderPosition(pos);
            else
                scrollTo(model()->index(last_index, 0), QAbstractItemView::PositionAtCenter);
            NovelDirItem* item = novel_dir_model->getItem(last_index);
            if (item->getName() == us->getStr("recent/chapter") || item->getName() == fnDecode(us->getStr("recent/chapter")))
                item->open();
        }
        delete read_last_timer;
    });
    read_last_timer->start();
}

void NovelDirListView::initStyle()
{
    this->setSpacing(us->mainwin_sidebar_spacing); // 章节间间距njkshjjfhjfhsf
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    setStyleSheet("QScrollBar:vertical\
        {                                         \
            width:8px;                            \
            background:rgba(128,128,128,0%);            \
            margin:0px,0px,0px,0px;               \
            padding-top:0px;                      \
            padding-bottom:0px;                   \
        }                                         \
        QScrollBar::handle:vertical               \
        {                                         \
            width:6px;                            \
            background:rgba(128,128,128,20%);           \
            border-radius:0px;                    \
            min-height:20;                        \
        }                                         \
        QScrollBar::handle:vertical:hover         \
        {                                         \
            width:7px;                            \
            background:rgba(128,128,128,40%);           \
            border-radius:3px;                    \
            min-height:20;                        \
        }                                         \
        QScrollBar::handle:vertical:pressed       \
        {                                         \
            width:9px;                            \
            background:rgba(128,128,128,70%);           \
            border-radius:3px;                    \
            min-height:20;                        \
        }                                         \
        QScrollBar::sub-line:vertical                                      \
        {                                                                  \
            height:9px;width:8px;                                          \
            border-image:url(:/images/a/1.png);                            \
            subcontrol-position:top;                                       \
        }                                                                  \
        QScrollBar::add-line:vertical               \
        {                                                                  \
            height:9px;width:8px;                                          \
            border-image:url(:/images/a/3.png);                            \
            subcontrol-position:bottom;                                    \
        }                                                                  \
        QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical        \
        {                                                                  \
            background:rgba(0,0,0,0%);                                     \
            border-radius:3px;                                             \
                  }");
}

void NovelDirListView::readNovel(QString novelName)
{
    this->novel_name = novelName;
    novel_dir_delegate->readSettings(novelName);
    novel_dir_model->readDir(novelName);
}

void NovelDirListView::initMenu()
{
    roll_menu = new QMenu();
    chapter_menu = new QMenu();
    blank_menu = new QMenu();
    QAction *insert_roll_action = new QAction(tr("插入新卷"), this);
    QAction *delete_roll_action = new QAction(tr("删除分卷"), this);
    QAction *rename_roll_action = new QAction(tr("重命名"), this);
    roll_menu->addAction(insert_roll_action);
    roll_menu->addAction(delete_roll_action);
    roll_menu->addAction(rename_roll_action);

    QAction *insert_chapter_action = new QAction(tr("插入新章"), this);
    QAction *delete_chapter_action = new QAction(tr("删除章节"), this);
    QAction *rename_chapter_action = new QAction(tr("重命名"), this);
    chapter_menu->addAction(insert_chapter_action);
    chapter_menu->addAction(delete_chapter_action);
    chapter_menu->addAction(rename_chapter_action);

    QAction *add_roll_action = new QAction(tr("添加新卷"), this);
    QAction *add_chapter_action = new QAction(tr("添加新章"), this);
    blank_menu->addAction(add_roll_action);
    blank_menu->addAction(add_chapter_action);

    connect(insert_roll_action, SIGNAL(triggered(bool)), this, SLOT(slotInsertRoll()));
    connect(delete_roll_action, SIGNAL(triggered(bool)), this, SLOT(slotDeleteRoll()));
    connect(rename_roll_action, SIGNAL(triggered(bool)), this, SLOT(slotRenameRoll()));
    connect(insert_chapter_action, SIGNAL(triggered(bool)), this, SLOT(slotInsertChapter()));
    connect(delete_chapter_action, SIGNAL(triggered(bool)), this, SLOT(slotDeleteChapter()));
    connect(rename_chapter_action, SIGNAL(triggered(bool)), this, SLOT(slotRenameChapter()));
    connect(add_roll_action, SIGNAL(triggered(bool)), this, SLOT(slotAddRoll()));
    connect(add_chapter_action, SIGNAL(triggered(bool)), this, SLOT(slotAddChapter()));
}

void NovelDirListView::initRecent()
{
    if (us->open_recent) {
        // 打开上次读取的章节（如果存在）
        QString recent_open_novel = us->getStr("recent/novel");
        QString recent_open_chapter = us->getStr("recent/chapter");
        QString recent_open_fullChapterName = us->getStr("recent/fullChapterName");
        int recent_open_roll_index = us->getInt("recent/roll_index", -1);
        int recent_open_chapter_index = us->getInt("recent/chapter_index", -1);

        if (novel_dir_model->isExistChapter(recent_open_roll_index, recent_open_chapter_index))
        {
            emit signalOpenChapter(novel_name, recent_open_chapter, recent_open_fullChapterName);
        }
        else
        {
            us->setVal("recent/roll_index", -1);
            us->setVal("recent/chapter_index", -1);
        }
    }

}

void NovelDirListView::mousePressEvent(QMouseEvent *event) // 左键右键的点击都能获取
{
    QListView::mousePressEvent(event);

    pressed_time1 = pressed_time2;
    pressed_time2 = getTimestamp();

    if (!currentIndex().isValid()) // 空白处
    {
        current_item = nullptr;
        return ;
    }
    else
    {
        int index = currentIndex().row();
        current_item = novel_dir_model->getItem(index);
        us->setVal("recent/index", index);
    }

    if (event->button() == Qt::LeftButton)
    {
        drag_start_pos = event->pos();
    }

}

void NovelDirListView::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (current_item == nullptr) return ;
    QMimeData * mime_data = new QMimeData;
    mime_data->setText(current_item->getName());
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mime_data);
	// drag->setPixmap(pixmap);
	Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
	if (dropAction == Qt::MoveAction)
	{
        // delete item; // 删除原来的
	}
}

void NovelDirListView::mouseReleaseEvent(QMouseEvent* event)
{
    if (current_item == nullptr)
        return QListView::mouseReleaseEvent(event);

    int distance = (drag_end_pos - drag_start_pos).manhattanLength();
    if (distance >= QApplication::startDragDistance() << 1)
    {
        startDrag(Qt::MoveAction);
    }
    if (currentIndex().isValid())
    {
        int index = currentIndex().row(); // 行数
        current_item = novel_dir_model->getItem(index);

        if (us->one_click && event->button() == Qt::LeftButton) // 是分组
        {
            emitItemEvent(index);
        }

        pre_index = index; // 保留上次的
    }
    released_time = getTimestamp();

    QListView::mouseReleaseEvent(event);
}

void NovelDirListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (us->one_click)
        return ;//QListView::mouseDoubleClickEvent(event); // 取消双击编辑

    int index = currentIndex().row(); // 行数
    emitItemEvent(index);

    QListView::mouseDoubleClickEvent(event);
}

void NovelDirListView::keyPressEvent(QKeyEvent *event)
{
    return QListView::keyPressEvent(event);

    /* 取消 Enter 键激活 Item，因为和重命名确定冲突了
    if (!currentIndex().isValid())
        return ;
    if (event->key() != Qt::Key_Enter && event->key() != Qt::Key_Return)
        return QListView::keyPressEvent(event);

    int index = currentIndex().row(); // 行数
    currentItem = novelDirModel->getItem(index);

    emitItemEvent(index);
    pre_index = index; // 保留上次的*/
}

void NovelDirListView::keyboardSearch(const QString &search)
{
    int count = novel_dir_model->data(model()->index(-1, 0), Qt::UserRole+DRole_CR_COUNT).toInt();

    // 寻找标题
    for (int i = 0; i < count; i++)
    {
        QModelIndex index = novel_dir_model->index(i,0);
        QString name = novel_dir_model->data(index, Qt::UserRole+DRole_CHP_NAME).toString();
        if (name.indexOf(search) > -1)
        {
            setCurrentIndex(index);
            scrollTo(index, ScrollHint::PositionAtCenter);
            return QListView::keyboardSearch(search);
        }
    }

    // 寻找序号


    // 寻找内容（太卡了，还是算了）


    QListView::keyboardSearch(search);
}

/**
 * @brief NovelDirListView::emitItemEvent
 * 激活一个 item（oneclick单击、双击、Enter）
 * @param index
 */
void NovelDirListView::emitItemEvent(int index)
{
    current_item = novel_dir_model->getItem(index);
    if (current_item != nullptr && current_item->isRoll())
    {
        bool hide = !current_item->isHide();
        current_item->setHide(hide);

        int i = index;
        while (++i < novel_dir_model->rowCount())
        {
            NovelDirItem* item = novel_dir_model->getItem(i);
            if (item->isRoll()) break;
            item->setHide(hide);
            setRowHidden(i, hide);
        }
        if (hide) // 原来是隐藏状态，展开，确保能见到更多
        {
            scrollTo(novel_dir_model->index(i,0));
            scrollTo(novel_dir_model->index(index,0));
        }

        // 保存Hide情况到文件
        novel_dir_model->setRollHide(index, hide);
    }
    else // 是章节，单击打开章节
    {
        slotOpenCurrentChapter();
    }
}

void NovelDirListView::slotOpenChapter(int index) // 这个是从model的edit完毕事件里面传回来的
{
    setCurrentIndex(model()->index(index, 0));
    if (novel_dir_model->getItem(index)->isRoll())
    {
        us->setVal("recent/index", index);
        return ;
    }
    slotOpenCurrentChapter();
}

void NovelDirListView::slotOpenCurrentChapter()
{
    if (!currentIndex().isValid()) return ;
    int index = currentIndex().row(); // 行数
    current_item = novel_dir_model->getItem(index);
    //us->setVal("recent/chapter_index", index); // 这是打开章节的index，和recent/index有所区分

    QString chapter_name = current_item->getName();
    QString full_chapter_name = novel_dir_delegate->getItemText(currentIndex());

    emit signalOpenChapter(novel_name, chapter_name, full_chapter_name);

    QRect ir = rectForIndex(currentIndex());
    QRect r(ir.left(), ir.top()-verticalOffset()+geometry().top(), ir.width(), ir.height()); // ListView 中的坐标
    QPoint p = mapFromGlobal(QCursor::pos())+this->parentWidget()->pos()+QPoint(0,geometry().top());
    if (!r.contains(p))
    {
        p.setX(r.left()+r.width()/2-10);
        p.setY(r.top()+r.height()/2-5);
    }

    emit signalOpenAnimationStartPos(p);

    current_item->open();
    us->setVal("recent/index", index);
    us->setVal("recent/roll_index", current_item->getRollIndex());    // 保存最近分组
    us->setVal("recent/chapter_index", current_item->getChptIndex()); // 保存最近子章
}

void NovelDirListView::slotOpenChapter(QString name)
{
    int index = novel_dir_model->getIndexByName(name);
    if (index < 0) return ;
    // 效果等同于 emitItemEvent();
    slotOpenChapter(index);
}

void NovelDirListView::dragEnterEvent(QDragEnterEvent* event)
{
    if (!currentIndex().isValid()) return ;
    NovelDirListView * source = static_cast<NovelDirListView*>(static_cast<void*>(event->source()));
	if (source && source == this)
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
    else if (source && source != this) // 其他列表拖过来的
    {
        ;
    }
}

void NovelDirListView::dragMoveEvent(QDragMoveEvent* event)
{
    if (!currentIndex().isValid()) return ;
    NovelDirListView * source = static_cast<NovelDirListView*>(static_cast<void*>(event->source()));
            //(NovelDirListView*)((void*)(event->source()));
    if (source && source == this)
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
    else if (source && source != this) // 其他列表拖过来的
    {
        ;
    }
}

void NovelDirListView::dropEvent(QDropEvent* event)
{
    NovelDirListView* source = static_cast<NovelDirListView*>(static_cast<void*>(event->source()));
	if (source && source == this)
	{
        drag_end_pos = event->pos();

        int org_index = indexAt(drag_start_pos).row();
        int aim_index = indexAt(drag_end_pos).row();
        novel_dir_model->exchangeItem(org_index, aim_index);

		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
    else if (source && source != this) // 其他列表拖过来的
    {
        ;
    }
}

void NovelDirListView::currentChange(const QModelIndex &current, const QModelIndex &previous)
{
    emit signalCurrentChanged(current, previous);
}

void NovelDirListView::contextMenuEvent(QContextMenuEvent *event)
{
    QListView::contextMenuEvent(event);
    if ( !(currentIndex().isValid()) )
    {
        blank_menu->exec(QCursor::pos());
    }
    else if (novel_dir_model->getItem(currentIndex().row())->isRoll())
    {
        roll_menu->exec(QCursor::pos());
    }
    else
    {
        chapter_menu->exec(QCursor::pos());
    }
}

void NovelDirListView::initRowHidden()
{
    NovelDirModel *model = getModel();
    for (int i = 0; i < model->rowCount(); i++)
    {
        NovelDirItem* item = model->getItem(i);
        if (!item->isRoll() && item->isHide())
        {
            setRowHidden(i, true);
        }
    }

}

void NovelDirListView::slotInsertRoll()
{
    if (current_item == nullptr) return ;
    if (!current_item->isRoll()) return ;
    int list_index = currentIndex().row();
    if (list_index <= 0) return ;
    if (list_index == 0)
    {
        QMessageBox::information(this, QObject::tr("你在干嘛"), QObject::tr("难道【作品相关】上面还能再加个卷？"));
        return ;
    }

    QString full_text = getModel()->getFullText();
    int i = 1;
    while (full_text.indexOf(QObject::tr("<RINF><n:新卷%1>").arg(i)) != -1)
        i++;
    QString new_name = QObject::tr("新卷%1").arg(i);

    int add_pos = novel_dir_model->insertRoll(list_index, new_name);
    QModelIndex modelIndex = model()->index(add_pos, 0);
    if (add_pos > 0)
    {
        setCurrentIndex(modelIndex);
        edit(modelIndex);
    }
}

void NovelDirListView::slotDeleteRoll()
{
    if (current_item == nullptr)
    {
        Log("删除分卷失败：没有选中项目");
        return ;
    }
    int list_index = currentIndex().row();

    if (list_index > 0)
    {
        QStringList names = novel_dir_model->deleteRoll(list_index);
        emit signalDeleteChapters(novel_name, names);

        for (QString s : names)
        {
            if (!s.isEmpty() && isFileExist(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(s)+".txt"))
            {
                deleteFile(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(s)+".txt");
            }
        }
    }
    else
        QMessageBox::information(this, QObject::tr("删除分卷失败"), QObject::tr("【作品相关】是开发者罩着的，不允许动它！"));
}

void NovelDirListView::slotRenameRoll()
{
    if (currentIndex().row() > 0)
        edit(currentIndex()); // 手动重命名函数
    else
        QMessageBox::information(this, QObject::tr("嘻嘻嘻"), QObject::tr("【作品相关】受到开发者庇佑，不允许更改名字"));
}

void NovelDirListView::slotInsertChapter()
{
    if (current_item == nullptr) return ;
    if (current_item->isRoll()) return ;
    int list_index = currentIndex().row();
    if (list_index <= 0) return ;
    if (list_index == 0)
    {
        QMessageBox::information(this, QObject::tr("你在干嘛"), QObject::tr("难道【作品相关】上面还能再加个章节？"));
        return ;
    }

    QString full_text = getModel()->getFullText();
    int i = 1;
    while (full_text.indexOf(QObject::tr("<CHPT><n:新章%1>").arg(i)) != -1)
        i++;
    QString new_name = QObject::tr("新章%1").arg(i);

    int add_pos = novel_dir_model->insertChapter(list_index, new_name);
    QModelIndex model_index = model()->index(add_pos, 0);
    if (add_pos > 0)
    {
        setCurrentIndex(model_index);
        edit(model_index);
    }
}

void NovelDirListView::slotDeleteChapter()
{
    if (current_item == nullptr)
    {
        Log("删除章节失败：没有选中项目");
        return ;
    }
    int list_index = currentIndex().row();
    QString name = novel_dir_model->deleteChapter(list_index);
    emit signalDeleteChapter(novel_name, name); // 通过信号槽删除编辑器中的章节

    if (isFileExist(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(name)+".txt"))
    {
        deleteFile(rt->NOVEL_PATH+novel_name+"/chapters/"+fnEncode(name)+".txt");
    }
}

void NovelDirListView::slotRenameChapter()
{
    if (currentIndex().isValid())
        edit(currentIndex()); // 手动重命名函数
}

void NovelDirListView::slotAddRoll()
{
    if (novel_name == "")
    {
        if (rt->promptCreateNovel(parentWidget()))
            emit signalNeedLineGuide();
        return ;
    }
    //if (!(currentIndex().isValid())) return ;

    QString full_text = getModel()->getFullText();

    int i = 1;
    while (full_text.indexOf(QObject::tr("<RINF><n:新卷%1>").arg(i)) > 0)
        i++;
    QString new_name = QObject::tr("新卷%1").arg(i);
    addRoll(new_name);
}

void NovelDirListView::slotAddChapter()
{
    if (novel_name == "")
    {
        if (rt->promptCreateNovel(parentWidget()))
            emit signalNeedLineGuide();
        return ;
    }
    if (!(currentIndex().isValid()))
    {
        QMessageBox::information(this, QObject::tr("请先选中某一卷"), QObject::tr("请选择或者创建一个分卷后，再新建章节"));
        return ;
    }

    QString full_Text = getModel()->getFullText();
    int list_index = currentIndex().row();

    int i = 1;
    while (full_Text.indexOf(QObject::tr("<CHPT><n:新章%1>").arg(i)) > 0)
        i++;
    QString new_name = QObject::tr("新章%1").arg(i);
    addChapter(list_index, new_name);
}

void NovelDirListView::slotInitRowHidden()
{
    initRowHidden();
}

void NovelDirListView::slotReedit()
{
    //this->closeEditor(novelDirDelegate->getEditor(), QAbstractItemDelegate::NoHint);

    // 因为还在编辑时不能重新编辑，所以增加了延迟
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotRenameChapter()));
    timer->start(100);
}

NovelDirModel *NovelDirListView::getModel()
{
    return novel_dir_model;
}

NovelDirDelegate *NovelDirListView::getDelegate()
{
    return novel_dir_delegate;
}

QString NovelDirListView::getNovelName()
{
    return novel_name;
}

void NovelDirListView::addRoll(QString newName)
{
    int add_pos = novel_dir_model->addRoll(newName);
    QModelIndex model_index = model()->index(add_pos, 0);
    if (add_pos > 0)
    {
        /*bool topOverView = false; // 如果top超过view的高度，即不显示时，不隐藏项目（因为底部一下子就上来了）
        if (rectForIndex(modelIndex).top() >= this->geometry().top())
            topOverView = true;*/

        setCurrentIndex(model_index);
        scrollTo(model_index);
        if (us->open_chapter_animation) // 开启动画
        {
            QRect rect = rectForIndex(model_index);
            QPixmap *pixmap = new QPixmap(rect.size());
            QRect index_rect(rect.left(), rect.top()-verticalOffset(), rect.width(), rect.height());
            //((QWidget*)this->parent())->render(pixmap, QPoint(0,0), rect,  QWidget::DrawWindowBackground | QWidget::DrawChildren);
            render(pixmap, QPoint(0,0), index_rect);
            StackWidgetAnimation* ani = new StackWidgetAnimation(this, pixmap, extern_point, index_rect.topLeft(), index_rect.size(), add_pos);
            connect(ani, SIGNAL(signalAni1Finished(int)), this, SLOT(slotAddAnimationFinished(int)));

            //if (!topOverView)
                //setRowHidden(addPos, true);
            novel_dir_model->getItem(add_pos)->setAnimating(true);
            novel_dir_model->listDataChanged();
        }
        else
        {
            edit(model_index);
        }
    }
}

void NovelDirListView::addChapter(int index, QString newName)
{
    int add_pos = novel_dir_model->addChapter(index, newName);
    QModelIndex model_index = model()->index(add_pos, 0);
    if (add_pos > 0)
    {
        /*bool topOverView = false;
        if (rectForIndex(modelIndex).top() >= this->geometry().top())
            topOverView = true;*/

        setCurrentIndex(model_index);
        scrollTo(model_index);
        if (us->open_chapter_animation) // 开启动画
        {
            QRect rect = rectForIndex(model_index);
            QPixmap *pixmap = new QPixmap(rect.size());
            QRect index_rect(rect.left(), rect.top()-verticalOffset(), rect.width(), rect.height());
            //((QWidget*)this->parent())->render(pixmap, QPoint(0,0), rect,  QWidget::DrawWindowBackground | QWidget::DrawChildren);
            render(pixmap, QPoint(0,0), index_rect);
            StackWidgetAnimation* ani = new StackWidgetAnimation(this, pixmap, extern_point, index_rect.topLeft(), index_rect.size(), add_pos);
            connect(ani, SIGNAL(signalAni1Finished(int)), this, SLOT(slotAddAnimationFinished(int)));

            /*if (!topOverView)
                setRowHidden(addPos, true);*/
            novel_dir_model->getItem(add_pos)->setAnimating(true);
            novel_dir_model->listDataChanged();
        }
        else
        {
            edit(model_index);
        }
    }
}

void NovelDirListView::slotExternRect(QRect r)
{
    extern_rect = r;
}

void NovelDirListView::slotExternPoint(QPoint p)
{
    extern_point = p;
}

void NovelDirListView::slotAddAnimationFinished(int index)
{
    //setRowHidden(index, false);
    novel_dir_model->getItem(index)->setAnimating(false);
    novel_dir_model->listDataChanged();
    QModelIndex model_index = model()->index(index, 0);
    setCurrentIndex(model_index);
    scrollTo(model_index);
    edit(model_index);
}

void NovelDirListView::slotExport()
{
    // 选取一个路径
    QString recent_export_path = us->getStr("recent/export_path");
    QString recent;
    if (!recent_export_path.isEmpty())
        recent = recent_export_path;
    else
        recent = us->getStr("recent/file_path");
    if (!isFileExist(recent))
        recent = "/home";
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择导出路径"), recent,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return ;
    us->setVal("recent/export_path", dir);

    QString file_path = getPathWithIndex(dir, novel_name, ".txt");

    if (exportNovel(file_path))
    {
        emit signalExportFinished();
    }
    else
    {
        QMessageBox::information(this, tr("导出失败"), QString("导出作品失败，路径：%1").arg(file_path));
    }
}

bool NovelDirListView::exportNovel(QString export_path)
{
    // 各种变量配置
    bool contains_about = us->getBool("export/about", false);
    bool contains_roll_split = us->getBool("export/split", true);
    bool contains_chpt_roll = us->getBool("export/roll", false);
    QString export_text = "";
    QString chapters_path = rt->NOVEL_PATH+novel_name+"/chapters/";
    NovelDirModel* model = novel_dir_model;
    NovelDirDelegate* delegate = novel_dir_delegate;

    // 导出的内容（后期应该可以手动调整）
    QString roll_prefix("　　　　　　");
    QString roll_suffix("\n\n\n");
    QString chapter_prefix("　　　　");
    QString chapter_suffix("\n\n\n");
    QString content_prefix("");
    QString content_suffix("\n\n\n");
    QString current_roll_name; // 现在正在遍历的卷

    // 遍历作品相关
    int traverse_start;
    if (contains_about)
        traverse_start = 0;
    else
        traverse_start = model->roll_subs[0]+1;

    // 遍历各卷
    int cr_len = model->getRcCount();
    for (int i = traverse_start; i < cr_len; i++) // 遍历每一个项目
    {
        NovelDirItem item = model->cr_list[i];
        if (item.isRoll()) // 是卷
        {
            if (contains_roll_split)
                export_text += roll_prefix + delegate->getItemText(model->index(i, 0)) + roll_suffix;
            current_roll_name = delegate->getItemText(model->index(i, 0));
        }
        else // 是章节
        {
            if (contains_chpt_roll)
                export_text += chapter_prefix + current_roll_name + " " +delegate->getItemText(model->index(i, 0)) + chapter_suffix;
            else
                export_text += chapter_prefix + delegate->getItemText(model->index(i, 0)) + chapter_suffix;

            // 读取内容并且导出
            QString content = readTextFile(chapters_path+fnEncode(item.getName())+".txt");
            export_text += content_prefix + content + content_suffix;
        }
    }

    writeTextFile(export_path, export_text);
    return true;
}

/**
 * 导入文件到当前小说
 * @param file_path  文件路径
 * @param split_roll 是否分卷
 * @param index      如果分卷，index==0时表示从最后一卷开始，否则新建一卷
 *                   如果不分卷，index<size时表示添加的卷下标，index>=size时表示新建一卷；
 */
void NovelDirListView::slotImport(QString file_path, bool split_roll, int index)
{
    Q_UNUSED(split_roll);

    // ==== 读取导入参数 ====
    bool auto_split_roll = us->getBool("import/auto_split_roll", true);
    bool keep_cr_number = us->getBool("import/keep_cr_number", false);
    bool add_new_roll = true;

    QString roll_reg = us->getStr("import/roll_regexp", "(第.{1,5}卷) +([^\\n]+)\\n");
    QString chpt_reg = us->getStr("import/chpt_regexp", "(第.{1,5}章) +([^\\n]+)\\n");

    if (novel_name.isEmpty() || file_path.isEmpty() || roll_reg.isEmpty() || chpt_reg.isEmpty())
        return ;
    if (!isFileExist(file_path))
        return ;

    // ==== 判断文件变量 ====
    QString novel_path = rt->NOVEL_PATH + novel_name;   // 小说文件夹
    QString dir_path = novel_path+"/dir.txt";      // 目录文件
    QString chapter_path = novel_path+"/chapters/"; // 章节文件夹

    // ==== 判断小说目录 ====
    ensureDirExist(novel_path);
    ensureDirExist(novel_path+"/chapters");
    ensureDirExist(novel_path+"/details");
    ensureDirExist(novel_path+"/outlines");
    ensureDirExist(novel_path+"/recycles");

    QString total_content = readTextFile(file_path);

    QRegExp roll_rx(roll_reg);
    QRegExp chpt_rx(chpt_reg);

    if (auto_split_roll)
    {
        int add_roll_index = novel_dir_model->getRcCount()-1; // 暂定为最后一卷
        if (index == 0)
        {
            add_new_roll = false; // 从最后一卷开始添加
        }
        else
        {
            add_new_roll = true; // 新建一卷开始添加
        }
        int roll_count = 0, chpt_count = 0;
        int roll_pos = 0;
        while (1)
        {
            // 当前卷的起始位置
            roll_pos = roll_rx.indexIn(total_content, roll_pos);
            if (roll_pos == -1) break;
            roll_pos += roll_rx.matchedLength();

            // 目录内容加上卷名
            QString roll_numb_str = roll_rx.cap(1); // 卷序号
            QString roll_name_str = roll_rx.cap(2); // 卷名字
            if (roll_count > 0 || add_new_roll == true)
            {
                if (keep_cr_number)
                {
                    novel_dir_model->addRoll(QString("%1 %2").arg(roll_numb_str).arg(roll_name_str));
                }
                else
                {
                    novel_dir_model->addRoll(QString("%1").arg(roll_name_str));
                }
                //add_roll_index++; // 添加章节的目标卷的索引+1
                add_roll_index = novel_dir_model->getRcCount()-1;
            }

            // 下一卷的位置
            int next_roll_pos = roll_rx.indexIn(total_content, roll_pos);
            if (next_roll_pos == -1)
                next_roll_pos = total_content.length();

            // 解析章节
            // 从 roll_pos ~ nextroll_pos 中间的都是章节
            QString roll_content = total_content.mid(roll_pos, next_roll_pos-roll_pos);
            int chpt_pos = 0;
            while ( 1 )
            {
                // 这一章名的位置
                chpt_pos = chpt_rx.indexIn(roll_content, chpt_pos);
                if (chpt_pos == -1) break;
                chpt_pos += chpt_rx.matchedLength();

                // 目录加上章节的名字
                QString chpt_numb_str = chpt_rx.cap(1);
                QString chpt_name_str = chpt_rx.cap(2);
                // 判断章节同名情况，加上后缀
                if (novel_dir_model->isExistChapter(chpt_name_str))
                {
                    int index = 1;
                    while (novel_dir_model->isExistChapter(QString("%1(%2)").arg(chpt_name_str).arg(index)))
                        index++;
                    chpt_name_str = QString("%1(%2)").arg(chpt_name_str).arg(index);
                }
                if (keep_cr_number) // addChapter 参数一：crList的列表索引，从索引中获取章节
                {
                    novel_dir_model->addChapter(add_roll_index, QString("%1 %2").arg(chpt_numb_str).arg(fnEncode(chpt_name_str)));
                }
                else
                {
                    novel_dir_model->addChapter(add_roll_index, QString("%1").arg(fnEncode(chpt_name_str)));
                }

                // 下一章名的位置
                int next_chpt_pos = chpt_rx.indexIn(roll_content, chpt_pos);
                if (next_chpt_pos == -1)
                    next_chpt_pos = roll_content.length();

                // 获取章节正文
                QString chpt_content = roll_content.mid(chpt_pos, next_chpt_pos-chpt_pos);
                chpt_content = simplifyChapter(chpt_content);
                QString chpt_file_path = chapter_path + fnEncode(chpt_name_str) + ".txt";
                writeTextFile(chpt_file_path, chpt_content);

                chpt_count++;
            }

            roll_count++;
        }

        QMessageBox::information(this, tr("导入成功"), QString("导入作品《%1》完成\n卷数：%2\n章数：%3").arg(novel_name).arg(roll_count).arg(chpt_count));
    }
    else
    {
        // ==== 清除卷名行 ====
        int roll_pos = 0;
        while ( (roll_pos=roll_rx.indexIn(total_content, roll_pos)) != -1 )
            total_content = total_content.left(roll_pos) + total_content.right(total_content.length()-roll_pos-roll_rx.matchedLength());

        int add_roll_index = index;
        int chpt_count = 0;
        if (index < novel_dir_model->getRcCount())
        {
            add_new_roll = false; // 加到现有的卷
            add_roll_index = novel_dir_model->getListIndexByRoll(index);
        }
        else
        {
            add_new_roll = true; // 新建一卷再添加
            QFileInfo info(file_path);
            QString file_name = info.baseName();
            novel_dir_model->addRoll(file_name);
            add_roll_index = novel_dir_model->getRcCount()-1;
        }

        // ==== 开始导入 ====
        int chpt_pos = 0;
        while ( 1 )
        {
            // 章名位置
            chpt_pos = chpt_rx.indexIn(total_content, chpt_pos);
            if (chpt_pos == -1) break;
            chpt_pos += chpt_rx.matchedLength();

            QString chpt_numb_str = chpt_rx.cap(1);
            QString chpt_name_str = chpt_rx.cap(2);

            // 判断章节同名情况，加上后缀
            if (novel_dir_model->isExistChapter(chpt_name_str))
            {
                int index = 1;
                while (novel_dir_model->isExistChapter(QString("%1(%2)").arg(chpt_name_str).arg(index)))
                    index++;
                chpt_name_str = QString("%1(%2)").arg(chpt_name_str).arg(index);
            }
            if (keep_cr_number)
            {
                novel_dir_model->addChapter(add_roll_index, QString("%1 %2").arg(chpt_numb_str).arg(fnEncode(chpt_name_str)));
            }
            else
            {
                novel_dir_model->addChapter(add_roll_index, QString("%1").arg(fnEncode(chpt_name_str)));
            }

            // 下一章名的位置
            int next_chpt_pos = chpt_rx.indexIn(total_content, chpt_pos);
            if (next_chpt_pos == -1)
                next_chpt_pos = total_content.length();

            // 获取章节正文
            QString chpt_content = total_content.mid(chpt_pos, next_chpt_pos-chpt_pos);
            chpt_content = simplifyChapter(chpt_content);
            QString chpt_file_path = chapter_path + fnEncode(chpt_name_str) + ".txt";
            writeTextFile(chpt_file_path, chpt_content);

            chpt_count++;
        }

        QMessageBox::information(this, tr("导入成功"), QString("导入作品《%1》完成\n章数：%2").arg(novel_name).arg(chpt_count));
    }

    emit signalImportFinished(novel_name);
}

QString NovelDirListView::simplifyChapter(QString text)
{
    int pos = 0, len = text.length();

    // 去除前面的空白
    while (pos < len && isBlankChar(text.mid(pos, 1)))
        pos++;
    while (pos > 0 && text.mid(pos-1, 1) != "\n")
        pos--;
    int start_pos = pos;

    pos = len;
    while (pos > 0 && isBlankChar(text.mid(pos-1, 1)))
        pos--;
    while (pos < len && text.mid(pos, 1) != "\n")
        pos++;
    int end_pos = pos;

    return text.mid(start_pos, end_pos-start_pos);
}

bool NovelDirListView::isBlankChar(QString c)
{
    if (c == " " || c == "\n" || c == "　" || c == "\t")
        return true;
    return false;
}

void NovelDirListView::slotDirSettings()
{
    // deb("show");
    DirSettingsWidget* dir_settings_widget = new DirSettingsWidget(this);
    dir_settings_widget->setNS(novel_dir_delegate);
    dir_settings_widget->setModal(true);
    dir_settings_widget->exec(); // 模态，运行结束后再进行读取
    // dir_settings_widget->show();
    novel_dir_delegate->readSettings(novel_name);
    update();
}
