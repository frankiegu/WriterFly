#include "basewindow.h"

BaseWindow::BaseWindow(QMainWindow *parent)
    : QMainWindow(parent)
{
    // FramelessWindowHint属性设置窗口去除边框;
    // WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);

    initLayout();

    showWindowAnimation();

    rt->startup_timestamp = getTimestamp(); // 打开的时间
}

BaseWindow::~BaseWindow()
{
    deleteGlobal();
    if (user_settings_window != nullptr)
        delete user_settings_window;
    if (novel_schedule_widget != nullptr)
        delete novel_schedule_widget;
}

void BaseWindow::initDataFiles()
{

}

void BaseWindow::initLayout()
{
    QWidget *widget = new QWidget();
    this->setCentralWidget(widget);

    // 初始化标题栏
    m_titlebar = new MyTitleBar(this);
    m_titlebar->move(0, 0);
    connect(m_titlebar, SIGNAL(signalButtonMinClicked()), this, SLOT(onButtonWinMinClicked()));
    connect(m_titlebar, SIGNAL(signalButtonRestoreClicked()), this, SLOT(onButtonWinRestoreClicked()));
    connect(m_titlebar, SIGNAL(signalButtonMaxClicked()), this, SLOT(onButtonWinMaxClicked()));
    connect(m_titlebar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonWinCloseClicked()));
    connect(m_titlebar, SIGNAL(signalButtonSidebarClicked()), this, SLOT(onButtonSidebarClicked()));
    connect(m_titlebar, SIGNAL(signalButtonSettingsClicked()), this, SLOT(onButtonSettingsClicked()));
    connect(m_titlebar, SIGNAL(changeFinished(QPoint,QSize)), this, SLOT(saveWinLayout(QPoint,QSize)));

    // 初始化左右布局
    dir_layout = new NovelDirGroup(widget);
    tab_widget = new StackWidget(this);
    QVBoxLayout* extra_tab_layout = new QVBoxLayout(); // margin 需要增加一个 layout
    QHBoxLayout *main_layout = new QHBoxLayout();
    drag_size_widget = new DragSizeWidget(this);

    extra_tab_layout->addWidget(tab_widget);
    extra_tab_layout->setContentsMargins(0, us->mainwin_titlebar_height, 0, 5); // 和顶部状态栏的间隔、底部窗口大小调整按钮的间隔

    main_layout->addWidget(dir_layout);
    //mainLayout->addWidget(tableWidget);
    main_layout->addLayout(extra_tab_layout);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    // 读取上次的窗口位置
    QVariant varPos = us->getVar("layout/win_point", QPoint(QApplication::desktop()->size().width()/10, QApplication::desktop()->size().height()/10));
    QVariant varSize = us->getVar("layout/win_size", QSize(QApplication::desktop()->size().width()*4/5, QApplication::desktop()->size().height()*4/5));
    QPoint window_pos;
    QSize window_size;
    if (us->have("layout/win_point") && us->have("layout/win_size") /*&& varPos != NULL && varSize != NULL*/)
    {
        window_pos = varPos.toPoint();
        window_size = varSize.toSize();
        if (window_pos.x() >= QApplication::desktop()->size().width()) // 超过右边
            window_pos.setX(QApplication::desktop()->size().width()*3/4);
        if (window_pos.x()+window_size.width() <= 0) // 超过左边
            window_pos.setX(-window_size.width()/4);
        if (window_pos.y() >= QApplication::desktop()->size().height()) // 超过底边
            window_pos.setY(QApplication::desktop()->size().height()*3/4);
        if (window_pos.y()+window_size.height() <= 0) // 超过顶边
            window_pos.setY(0);
    }
    else // 没有默认的坐标数据
    {
        // 获取多显示器中，当前显示器的坐标
        QDesktopWidget * desktop = QApplication::desktop();
        int curMonitor = desktop->screenNumber ( this ); // desktop->primaryScreen() 主显示器编号
        QRect rect = desktop->availableGeometry(curMonitor); // availableGeometry() 去掉任务栏后的坐标  screenGeometry  包括任务栏的坐标
        // 根据显示器的坐标调整初始值
        window_pos = QPoint(rect.left() + rect.width()/10, rect.top() + rect.height()/10);
        window_size = QSize(rect.width()*4/5, rect.height()*4/5);
    }
    this->setGeometry(QRect(window_pos, window_size));
    m_titlebar->saveRestoreInfo(window_pos, window_size);
    if (us->contains("layout/win_max") && us->getBool("layout/win_max"))
        m_titlebar->simulateMax();

    widget->setLayout(main_layout);
    drag_size_widget->move(this->geometry().width()-drag_size_widget->width(), this->geometry().height()-drag_size_widget->height());
    connect(drag_size_widget, SIGNAL(signalMoveEvent(int, int)), this, SLOT(slotWindowSizeChanged(int,int)));

    // 初始化事件
    connect(dir_layout->getListView(), SIGNAL(signalOpenChapter(QString, QString, QString)), tab_widget, SLOT(slotOpenChapter(QString, QString, QString)));
    connect(dir_layout->getListView()->getModel(), SIGNAL(signalChangeName(bool,QString,QString,QString)), tab_widget, SLOT(slotChangeName(bool,QString,QString,QString)));
    connect(dir_layout->getListView(), SIGNAL(signalOpenAnimationStartPos(QPoint)), tab_widget, SLOT(slotOpenAnimation(QPoint)));
    //connect(novelDirLayout->getListView()->getModel(), SIGNAL(signalDeleteChapter(QString, QString)), tabWidget, SLOT(slotDeleteChapter(QString, QString)));
    connect(dir_layout->getListView(), SIGNAL(signalDeleteChapter(QString,QString)), tab_widget, SLOT(slotDeleteChapter(QString,QString)));
    connect(dir_layout->getListView(), SIGNAL(signalDeleteChapters(QString,QStringList)), tab_widget, SLOT(slotDeleteChapters(QString,QStringList)));
    connect(dir_layout, SIGNAL(signalRenameNovel(QString,QString)), tab_widget, SLOT(slotRenameNovel(QString,QString)));
    connect(dir_layout, SIGNAL(signalDeleteNovel(QString)), tab_widget, SLOT(slotDeleteNovel(QString)));
    connect(tab_widget, SIGNAL(signalChangeTitleByChapter(QString)), this, SLOT(slotSetWindowTitleBarContent(QString)));
    connect(tab_widget, SIGNAL(signalEditorWordCount(int)), this, SLOT(slotChapterWordCount(int)));
    connect(tab_widget, SIGNAL(signalSetTitleNovel(QString)), this, SLOT(slotSetTitleNovel(QString)));
    connect(tab_widget, SIGNAL(signalSetTitleChapter(QString)), this, SLOT(slotSetTitleChapter(QString)));
    connect(tab_widget, SIGNAL(signalSetTitleFull(QString)), this, SLOT(slotSetTitleFull(QString)));
    // connect(tabWidget, SIGNAL(signalSetTitleWc(int)), this, SLOT(slotSetTitleWc(int)));

    // 初始化其他窗口
    user_settings_window = nullptr;  // 设置窗口，等用到时再进行初始化
    novel_schedule_widget = nullptr; // 一览窗口，同上
    im_ex_window = nullptr;          // 导入导出，同上
    trans_ctn = new TransparentContainer(this);
    connect(dir_layout, SIGNAL(signalShowInfo(QString,QPoint)), this, SLOT(slotShowNovelInfo(QString,QPoint)));
    connect(trans_ctn, SIGNAL(signalToClose(int)), this, SLOT(slotTransContainerToClose(int)));
    connect(trans_ctn, SIGNAL(signalClosed(int)), this, SLOT(slotTransContainerClosed(int)));
}

void BaseWindow::resizeEvent(QResizeEvent* /*event*/)
{
    if (dir_layout->isPacking())
    {
        if (dir_layout->geometry().left() >= -10)
        {
            dir_layout->move(-dir_layout->geometry().width()/2, dir_layout->geometry().top());
            tab_widget->setGeometry(0, tab_widget->geometry().top(), geometry().width(), tab_widget->geometry().height());
        }
    }
    tab_widget->setFocus();
}

void BaseWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (rt->is_line_guide)
        paintEvent(nullptr);

    return QMainWindow::mouseMoveEvent(event);
}

void BaseWindow::paintEvent(QPaintEvent* event)
{
    //设置背景色;
    QPainter painter(this);
    if (isFileExist(rt->IMAGE_PATH+"win_bg.png"))
    {
        QPainterPath path_back;
        path_back.setFillRule(Qt::WindingFill);
        path_back.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.fillPath(path_back, QBrush(us->mainwin_bg_color));

        painter.drawPixmap(0, 0, this->width(), this->height(), QPixmap(rt->IMAGE_PATH+"win_bg.png"));
    }
    else if (isFileExist(rt->IMAGE_PATH+"win_bg.jpg"))
    {
        painter.drawPixmap(0, 0, this->width(), this->height(), QPixmap(rt->IMAGE_PATH+"win_bg.jpg"));
    }
    else
    {
        QPainterPath path_back;
        path_back.setFillRule(Qt::WindingFill);
        path_back.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.fillPath(path_back, QBrush(us->mainwin_bg_color));
    }
    if (rt->is_line_guide)
    {
        qDebug() << "guide";
        QPoint aim_pos = rt->guide_widget->pos();
        if (rt->line_guide_case == LINE_GUIDE_CREATE_BOOK)
            aim_pos += rt->guide_widget->parentWidget()->pos();
        QPoint cur_pos = QCursor::pos();
        cur_pos = mapFromGlobal(cur_pos);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QColor(34, 139, 34));
        painter.drawLine(cur_pos, aim_pos);
    }
    return QWidget::paintEvent(event);
}

void BaseWindow::onButtonWinMinClicked()
{
    //showMinimized();
    if (!m_titlebar->isMaxxing())
        m_titlebar->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
    toWinMinAnimation();
}

void BaseWindow::onButtonWinMaxClicked()
{
    m_titlebar->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
    drag_size_widget->hide();
    toWinMaxAnimation();
    us->setVal("layout/win_max", true);
    us->sync();
}

void BaseWindow::onButtonWinRestoreClicked()
{
    drag_size_widget->show();
    toWinRestoreAnimation();
    us->setVal("layout/win_max", false);
    us->sync();
}

void BaseWindow::onButtonWinCloseClicked()
{
    //close();
    closeWindowAnimation();
}

void BaseWindow::onButtonSidebarClicked()
{
    if (dir_layout->geometry().left() >= -10) // 开始隐藏（负）
    {
        setSidebarShowed(false);
    }
    else // 开始显示
    {
        setSidebarShowed(true);
    }
    tab_widget->setFocus();
}

void BaseWindow::setSidebarShowed(bool show)
{
    if (show) // 开始显示
    {
        int distance = 0-dir_layout->geometry().left();
        dir_layout->startShowSidebar(distance);

        distance = dir_layout->geometry().width()-tab_widget->geometry().left();
        QPropertyAnimation *animation = new QPropertyAnimation(tab_widget, "geometry");
        animation->setDuration(200);
        animation->setStartValue(tab_widget->geometry());
        animation->setEndValue(QRect(tab_widget->geometry().left()+distance, tab_widget->geometry().top(), tab_widget->geometry().width()-distance, tab_widget->geometry().height()));
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start();
    }
    else // 开始隐藏
    {
        int distance = dir_layout->geometry().width()/2 + dir_layout->geometry().left(); // 正的
        dir_layout->startHideSidebar(-distance);

        distance = tab_widget->geometry().left(); // 正的
        QPropertyAnimation *animation = new QPropertyAnimation(tab_widget, "geometry");
        animation->setDuration(200);
        animation->setStartValue(tab_widget->geometry());
        animation->setEndValue(QRect(tab_widget->geometry().left()-distance, tab_widget->geometry().top(), tab_widget->geometry().width()+distance, tab_widget->geometry().height()));
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start();
    }
}

void BaseWindow::onButtonSettingsClicked()
{
    if (user_settings_window == nullptr)
    {
        if (us->settings_in_mainwin) // 界面嵌入到主窗口
        {
            user_settings_window = new UserSettingsWindow(this);
        }
        else
        {
            user_settings_window = new UserSettingsWindow(this);
            user_settings_window->setWindowFlags(Qt::Dialog);
            user_settings_window->setWindowModality(Qt::WindowModal);
        }

        connect(user_settings_window, SIGNAL(signalEditorUIChanged()), tab_widget, SLOT(slotUpdateUI()));
        connect(user_settings_window, SIGNAL(signalEditorBottomSettingsChanged()), tab_widget, SLOT(slotEditorBottomSettingsChanged()));
        connect(user_settings_window, SIGNAL(signalEsc()), trans_ctn, SLOT(slotEsc()));
    }

    int left = this->geometry().left(), top = this->geometry().top();
    int width = this->width(), height = this->height();
    if (us->settings_in_mainwin)
    {
        user_settings_window->setGeometry(width/10,height/10, width*4/5, height*4/5);
    }
    else
    {
        user_settings_window->setGeometry(left+width/10,top+height/10, width*4/5, height*4/5);
        user_settings_window->show();
        return ;
    }
    callback_point = mapFromGlobal(QCursor::pos());
    user_settings_window->show();
    QPoint po(callback_point);
    QRect er(user_settings_window->geometry());
    QPixmap* pixmap = new QPixmap(er.size());
    user_settings_window->render(pixmap);
    ZoomGeometryAnimationWidget* ani = new ZoomGeometryAnimationWidget(this, pixmap, QRect(po, QSize(1,1)), er, QEasingCurve::Linear, 200, 1);
    connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=]{
        user_settings_window->show();
        user_settings_window->setFocus();
    });
    user_settings_window->hide();
    trans_ctn->toShow();

    /*QPropertyAnimation *animation = new QPropertyAnimation(userSettingsWindow, "geometry");
    animation->setDuration(150);
    QRect startR = m_titleBar->getBtnRect();
    animation->setStartValue(QRect(QCursor::pos(), QSize(userSettingsWindow->width()>>1, userSettingsWindow->height()>>1)));
    animation->setEndValue(userSettingsWindow->geometry());
    animation->start();*/
}

void BaseWindow::toWinMin()
{
    showMinimized();

    if (m_titlebar->isMaxxing())
    {
        toWinMax();
    }
    else
    {
        toWinRestore();
    }
}

void BaseWindow::toWinMax()
{
    QRect desktop_rect = QApplication::desktop()->availableGeometry();
    QRect fact_rect = QRect(desktop_rect.x() - 3, desktop_rect.y() - 3, desktop_rect.width() + 6, desktop_rect.height() + 6);
    setGeometry(fact_rect);
    us->setVal("layout/win_max", true);
}

void BaseWindow::toWinRestore()
{
    QPoint window_pos;
    QSize window_size;
    m_titlebar->getRestoreInfo(window_pos, window_size);
    this->setGeometry(QRect(window_pos, window_size));
    us->setVal("layout/win_max", false);
}

void BaseWindow::showWindowAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(300);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();
}

void BaseWindow::closeWindowAnimation()
{
   QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
   animation->setDuration(300);
   animation->setStartValue(1);
   animation->setEndValue(0);
   animation->start();
   connect(animation, SIGNAL(finished()), this, SLOT(close()));
}

void BaseWindow::toWinMinAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(300);
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(geometry().x()+width()/10,geometry().y()+height()/2,width()*4/5,0));
    animation->start();
    connect(animation, SIGNAL(finished()), this, SLOT(toWinMin()));
}

void BaseWindow::toWinMaxAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(300);
    animation->setStartValue(QRect(geometry().left(), geometry().top(), geometry().width(), geometry().height()));
    QRect desktopRect = QApplication::desktop()->availableGeometry();
    QRect FactRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
    animation->setEndValue(FactRect);
    animation->start();
    connect(animation, SIGNAL(finished()), this, SLOT(toWinMax()));
}

void BaseWindow::toWinRestoreAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(300);
    animation->setStartValue(QRect(geometry().left(), geometry().top(), geometry().width(), geometry().height()));
    QPoint point;
    QSize size;
    m_titlebar->getRestoreInfo(point, size);
    animation->setEndValue(QRect(point, size));
    animation->start();
}

void BaseWindow::closeEvent(QCloseEvent *)
{
    us->sync(); // 保存所有设置
    tab_widget->slotSavePosition(); // 保存没有更改的章节
    int dir_scroll = dir_layout->getListView()->verticalScrollBar()->sliderPosition();
    if (dir_scroll >= 0)
        us->setVal("recent/dir_scroll", dir_scroll);
}

void BaseWindow::saveWinLayout(QPoint point, QSize size)
{
    us->setVal("layout/win_point", point);
    us->setVal("layout/win_size", size);
    us->setVal("layout/win_max", false);
}

/*void BaseWindow::slotSetTitleBarContent(QString title)
{
    m_titleBar->setTitleContent(title);
}*/

void BaseWindow::slotWindowSizeChanged(int dw, int dh)
{
    if (m_titlebar->isMaxxing())
        return ;
    int w = this->width();
    int h = this->height();
    if (w+dw > 100)
        w += dw;
    if (h+dh > 80)
        h += dh;
    this->setGeometry(this->geometry().left(), this->geometry().top(), w, h);
    drag_size_widget->move(this->geometry().width()-drag_size_widget->width(), this->geometry().height()-drag_size_widget->height());
    us->setVal("layout/win_size", this->size());
}

void BaseWindow::slotSetWindowTitleBarContent(QString title)
{
    m_titlebar->setTitleContent(title);
    setWindowTitle(title);
}

void BaseWindow::slotSetTitleNovel(QString t)
{
    title_novel = t;
    m_titlebar->setTitleContent(QString(" %1  [ %2字 ]").arg(title_full).arg(title_wc));
    setWindowTitle(QString("%1").arg(title_full));
}

void BaseWindow::slotSetTitleChapter(QString t)
{
    title_chapter = t;
    m_titlebar->setTitleContent(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
    setWindowTitle(QString("%1").arg(title_full));
}

void BaseWindow::slotSetTitleFull(QString t)
{
    title_full = t;
    m_titlebar->setTitleContent(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
    setWindowTitle(QString("%1").arg(title_full));
}

void BaseWindow::slotChapterWordCount(int x)
{
    title_wc = x;
    m_titlebar->setTitleContent2(QString("%1  [ %2字 ]").arg(title_full).arg(title_wc));
    setWindowTitle(QString("%1").arg(title_full));
}

void BaseWindow::slotTransContainerToClose(int kind)
{
    Q_UNUSED(kind);
    if (user_settings_window != nullptr && !user_settings_window->isHidden())
    {
        QRect sr = user_settings_window->geometry();
        QRect er(callback_point, QSize(1,1));
        QPixmap* pixmap = new QPixmap(sr.size());
        user_settings_window->render(pixmap);
        ZoomGeometryAnimationWidget* ani = new ZoomGeometryAnimationWidget(this, pixmap, sr, er, QEasingCurve::Linear, 200, 1);
        connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=]{

        });
        user_settings_window->hide();
    }
    if (novel_schedule_widget != nullptr && !novel_schedule_widget->isHidden())
    {
        QRect sr = novel_schedule_widget->geometry();
        QRect er(callback_point, QSize(1,1));
        QPixmap* pixmap = new QPixmap(sr.size());
        novel_schedule_widget->render(pixmap);
        ZoomGeometryAnimationWidget* ani = new ZoomGeometryAnimationWidget(this, pixmap, sr, er, 1);
        connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=]{

        });
        novel_schedule_widget->hide();
    }
}

void BaseWindow::slotTransContainerClosed(int kind)
{
    Q_UNUSED(kind);
    tab_widget->slotDelayFocus();
}

void BaseWindow::slotShowNovelInfo(QString novel_name, QPoint point)
{
    if (novel_name == "")
    {
        if (rt->promptCreateNovel(this))
            dir_layout->slotNeedLineGuide();
        return ;
    }
    if (novel_schedule_widget == nullptr)
    {
        novel_schedule_widget = new NovelScheduleWidget(dir_layout->getListView(), this);

        // 初始化信号槽
        connect(novel_schedule_widget, &NovelScheduleWidget::signalToHide, [=]{
            novel_schedule_widget->hide();
            trans_ctn->hide();
        });
        connect(novel_schedule_widget, SIGNAL(signalChangeNovelName()), dir_layout, SLOT(slotRenameNovel()));
        connect(dir_layout, SIGNAL(signalRenameNovel(QString,QString)), novel_schedule_widget, SLOT(slotChangeNovelNameFinished(QString,QString)));
        connect(novel_schedule_widget, SIGNAL(signalDeleteNovel()), dir_layout, SLOT(slotDeleteNovel()));
        connect(novel_schedule_widget, SIGNAL(signalImExPort(int,QString)), this, SLOT(slotShowImExWindow(int,QString)));
        connect(novel_schedule_widget, SIGNAL(signalDirSettings()), dir_layout->getListView(), SLOT(slotDirSettings()));
        connect(novel_schedule_widget, SIGNAL(signalEsc()), trans_ctn, SLOT(slotEsc()));
    }
    callback_point = point;

    // 动画
    novel_schedule_widget->toShow(novel_name, point); // 先显示，再隐藏
    QPixmap* pixmap = new QPixmap(novel_schedule_widget->geometry().size());
    novel_schedule_widget->render(pixmap);
    QRect sr(point, QSize(1,1));
    QRect er(novel_schedule_widget->geometry());
    novel_schedule_widget->hide(); // 隐藏掉
    trans_ctn->toShow(); // 先显示蒙版，避免把小窗口图标也显示出来
    ZoomGeometryAnimationWidget* ani = new ZoomGeometryAnimationWidget(this, pixmap, sr, er, QEasingCurve::OutBack, us->mainwin_bg_color.alpha(), -1);
    connect(ani, &ZoomGeometryAnimationWidget::signalAni1Finished, [=]{
        novel_schedule_widget->show();
        novel_schedule_widget->setFocus();
    });

}

void BaseWindow::slotShowImExWindow(int kind, QString def_name)
{
    if (im_ex_window == nullptr)
    {
        // 初始化导入导出窗口
        im_ex_window = new ImExWindow(this);
        // if (!us->settings_in_mainwin)
        {
            im_ex_window->setWindowFlags(Qt::Dialog);
            im_ex_window->setWindowModality(Qt::WindowModal);
        }

        // 初始化信号连接
        // 导出
        connect(im_ex_window->export_page, SIGNAL(signalExport()), dir_layout->getListView(), SLOT(slotExport()));
        connect(dir_layout->getListView(), SIGNAL(signalExportFinished()), im_ex_window->export_page, SLOT(slotExportFinished()));
        // 导入
        connect(im_ex_window->import_page, SIGNAL(signalImport(QString,bool,int)), dir_layout->getListView(), SLOT(slotImport(QString,bool,int)));
        connect(dir_layout->getListView(), SIGNAL(signalImportFinished(QString)), im_ex_window->import_page, SLOT(slotImportFinished(QString)));
        connect(im_ex_window->import_page, SIGNAL(signalImportFinished(QString)), dir_layout, SLOT(slotImportFinished(QString)));

        connect(im_ex_window->import_page, &ImportPage::signalImportFinished, [=]{
            if (novel_schedule_widget != nullptr && !novel_schedule_widget->isHidden())
                novel_schedule_widget->toShow(im_ex_window->import_page->getTargetName(), QPoint(0, 0));
        });
        connect(im_ex_window->import_mzfy_page, SIGNAL(signalImportMzfyFinished(QString)), dir_layout, SLOT(slotImportFinished(QString)));
    }
    im_ex_window->import_page->setNovelDirListView(dir_layout->getListView());
    //im_ex_window->setNovelName(def_name);
    im_ex_window->toShow(kind, def_name);
}
