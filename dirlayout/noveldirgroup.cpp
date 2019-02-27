#include "noveldirgroup.h"
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>

NovelDirGroup::NovelDirGroup(QWidget * parent)
        : QWidget(parent), fixed_max_width(200), is_packing(false)
{
    novel_new_name_temp = "";
    novel_had_deleted_temp = false;

    initLayout();
    initDir();

}

NovelDirGroup::~NovelDirGroup()
{
    ;
}

void NovelDirGroup::initLayout()
{
    this->setMinimumWidth(50);
    book_switch_combo = new QComboBox(this);
    novel_dir_listview = new NovelDirListView(this);
    add_roll_button = new AniFButton(tr("新卷"), this);
    add_chapter_button = new AniFButton(tr("新章"), this);
    info_button = new AniFButton(tr("一览"), this);
    splitter_widget = new SplitterWidget(this);
    QHBoxLayout* main_hlayout = new QHBoxLayout(this);
    QVBoxLayout *vlayout = new QVBoxLayout();
    btn_layout = new QHBoxLayout();

    btn_layout->addWidget(add_roll_button);
    btn_layout->addWidget(add_chapter_button);
    btn_layout->addWidget(info_button);
    QRect rect = btn_layout->geometry();
    rect.setHeight(add_roll_button->getMiniHeight());
    btn_layout->setGeometry(rect);

    vlayout->addWidget(book_switch_combo);
    vlayout->addWidget(novel_dir_listview);
    vlayout->addLayout(btn_layout);
    vlayout->setSpacing(2);
    vlayout->setContentsMargins(3, us->mainwin_titlebar_height, 0, 3);

    main_hlayout->addLayout(vlayout);
    main_hlayout->addWidget(splitter_widget);
    main_hlayout->setSpacing(0);
    main_hlayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(main_hlayout);

    this->setMinimumWidth(50);
    this->setMaximumWidth(fixed_max_width);

    connect(book_switch_combo, SIGNAL(activated(int)), this, SLOT(switchBook(int)));
    connect(add_roll_button, SIGNAL(clicked(bool)), this, SLOT(slotAddRollButtonClicked()));
    connect(add_chapter_button, SIGNAL(clicked(bool)), this, SLOT(slotAddChapterButtonClicked()));
    //connect(add_roll_button, SIGNAL(clicked(bool)), novel_dir_listview, SLOT(slotAddRoll()));
    //connect(add_chapter_button, SIGNAL(clicked(bool)), novel_dir_listview, SLOT(slotAddChapter()));
    connect(novel_dir_listview, SIGNAL(signalCurrentChanged(const QModelIndex, const QModelIndex)), this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));
    connect(splitter_widget, SIGNAL(signalMoveEvent(int)), this, SLOT(slotFixedWidthChanged(int)));
    connect(info_button, SIGNAL(clicked(bool)), this, SLOT(slotInfoClicked()));

    initStyleSheet();

    QPoint pos = QCursor::pos();
    pos = mapFromGlobal(pos);
    if (!rect.contains(pos))
    {
        /*add_roll_button->hide();
        add_chapter_button->hide();
        info_button->hide();*/
        leaveEvent(nullptr);
    }

    int x = us->getInt("layout/sidebar_width", 200);
    if (x >= 50 && x <= parentWidget()->width()*4/5)
        fixed_max_width = x;
    else if (x < 50)
        x = 50;
    else if (x > parentWidget()->width()*4/5)
        fixed_max_width = parentWidget()->width()*4/5;
    else
        fixed_max_width = 200;
    setMaximumWidth(fixed_max_width);
}

void NovelDirGroup::initStyleSheet()
{
    QString trans("QPushButton,QListView{background: transparent; border:none;}");
    novel_dir_listview->setStyleSheet(trans);

    //QListView view((QListView)(bookSwitchComboBox->view()));
    QListView* pop_listview = new QListView(this);
    pop_listview->setSpacing(2);
    book_switch_combo->setView(pop_listview);

    book_switch_combo->setStyleSheet(QString("\
        QComboBox{ padding: 4px; border:1px solid #88888888;  border-radius:1px; min-width:4em; background:rgba(255,255,255,0%);}\
        QComboBox:hover{border-radius:0px; background:rgba(240,248,255,80%);}\
        QComboBox::drop-down{subcontrol-origin:padding; subcontrol-position:top right; border-left-width:0px;border-left-color:darkgray; border-left-style:solid; border-top-right-radius:3px; border-bottom-right-radius:3px;}\
        QComboBox::down-arrow{image: url(:/icons/arrow_down_s);}")); // QComboBox:pressed{border-radius:0px; background:rgba(240,248,255,80%);}
}

void NovelDirGroup::initDir()
{
    // 目录文件夹是否存在
    ensureDir(rt->DATA_PATH);
    ensureDir(rt->NOVEL_PATH);

    //stts->setSection("dir");

    // 打开上次小说
    QString recent_name = us->getStr("recent/novel");
    if (!novel_new_name_temp.isEmpty())
    {
        recent_name = novel_new_name_temp;
        novel_new_name_temp = "";
    }
    if (recent_name != "")
    {
        QDir bookDir(rt->NOVEL_PATH+recent_name);
        if (!bookDir.exists())
        {
            //QMessageBox::information(this, tr("作品不存在"), QObject::tr("很抱歉，您的作品《%1》文件不存在！\n文件路径：%2").arg(recentName).arg(NovelPath+recentName));
            recent_name = "";
            us->setVal("recent/novel", "");
            novel_dir_listview->readNovel(""); // 打开空的小说，即清空
        }
        else
        {
            novel_dir_listview->readNovel(recent_name); // 打开这个小说
        }
    }

    // 读取所有目录
    book_switch_combo->clear();
    QDir dirDir(rt->NOVEL_PATH);
    dirDir.setFilter(QDir::Dirs);
    QFileInfoList list = dirDir.entryInfoList();
    int count = list.count();
    for (int i = 0; i < count; i++)
    {
        QFileInfo fileInfo = list.at(i);
        QString fileName = fileInfo.fileName();
        if (fileName == "." || fileName == "..")
            continue;
        book_switch_combo->addItem(fileName);

        if (fileName == recent_name)
        {
            book_switch_combo->setCurrentIndex(i);
            book_switch_combo->setCurrentText(recent_name);
        }
    }

    // 创建新书的选项
    book_switch_combo->addItem(QWidget::tr("创建新书"));
}

void NovelDirGroup::paintEvent(QPaintEvent *event)
{
    //设置背景色;
    QPainter painter(this);
    if (isFileExist(rt->IMAGE_PATH+"sidebar_bg.png"))
    {
        painter.drawPixmap(0, 0, this->width(), this->height(), QPixmap(rt->IMAGE_PATH+"win_bg.jpg"));
    }
    else
    {
        QPainterPath path_back;
        path_back.setFillRule(Qt::WindingFill);
        path_back.addRect(0, 0, this->width(), this->height());
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.fillPath(path_back, QBrush(us->mainwin_sidebar_color));
    }
    return QWidget::paintEvent(event);
}

void NovelDirGroup::enterEvent(QEvent *event)
{
    if (isAnimating())
        return ;

    /*add_roll_button->show();
    add_chapter_button->show();
    info_button->show();

    QPropertyAnimation* animation1 = new QPropertyAnimation(add_roll_button, "pos");
    animation1->setDuration(300);
    animation1->setStartValue(QPoint(add_roll_button->geometry().left(), height()));
    animation1->setEndValue(QPoint(add_roll_button->geometry().left(), height()-add_roll_button->height()-3));
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->start();
    connect(animation1, &QPropertyAnimation::finished, [=]{
        delete animation1;
    });

    QPropertyAnimation* animation2 = new QPropertyAnimation(add_chapter_button, "pos");
    animation2->setDuration(300);
    animation2->setStartValue(QPoint(add_chapter_button->geometry().left(), height()));
    animation2->setEndValue(QPoint(add_chapter_button->geometry().left(), height()-add_chapter_button->height()-3));
    animation2->setEasingCurve(QEasingCurve::OutQuad);
    animation2->start();
    connect(animation2, &QPropertyAnimation::finished, [=]{
        delete animation2;
    });

    QPropertyAnimation* animation3 = new QPropertyAnimation(info_button, "pos");
    animation3->setDuration(300);
    animation3->setStartValue(QPoint(info_button->geometry().left(), height()));
    animation3->setEndValue(QPoint(info_button->geometry().left(), height()-info_button->height()-3));
    animation3->setEasingCurve(QEasingCurve::OutQuad);
    animation3->start();
    connect(animation3, &QPropertyAnimation::finished, [=]{
        delete animation3;
    });*/

    /*QGraphicsOpacityEffect* effect1 = new QGraphicsOpacityEffect;
    effect1->setOpacity(1.0);
    add_roll_button->setGraphicsEffect(effect1);
    QPropertyAnimation* ani1 = new QPropertyAnimation(effect1, "opacity");
    ani1->setDuration(300);
    ani1->setStartValue(0);
    ani1->setEndValue(1.0);
    connect(ani1, &QPropertyAnimation::finished, [=]{
        add_roll_button->setGraphicsEffect(NULL);
    });
    ani1->start();

    QGraphicsOpacityEffect* effect2 = new QGraphicsOpacityEffect(add_chapter_button);
    effect2->setOpacity(1.0);
    add_chapter_button->setGraphicsEffect(effect2);
    QPropertyAnimation* ani2 = new QPropertyAnimation(effect2, "opacity");
    ani2->setDuration(300);
    ani2->setStartValue(0);
    ani2->setEndValue(1.0);
    connect(ani2, &QPropertyAnimation::finished, [=]{
        add_chapter_button->setGraphicsEffect(NULL);
    });
    ani2->start();

    QGraphicsOpacityEffect* effect3 = new QGraphicsOpacityEffect(info_button);
    effect3->setOpacity(1.0);
    info_button->setGraphicsEffect(effect3);
    QPropertyAnimation* ani3 = new QPropertyAnimation(effect3, "opacity");
    ani3->setDuration(300);
    ani3->setStartValue(0);
    ani3->setEndValue(1.0);
    connect(ani3, &QPropertyAnimation::finished, [=]{
        info_button->setGraphicsEffect(NULL);
    });
    ani3->start();*/

    return QWidget::enterEvent(event);
}

void NovelDirGroup::leaveEvent(QEvent *event)
{
    if (isAnimating())
        return ;

    /*QPropertyAnimation* animation1 = new QPropertyAnimation(add_roll_button, "pos");
    animation1->setDuration(300);
    animation1->setStartValue(add_roll_button->pos());
    animation1->setEndValue(add_roll_button->pos()+QPoint(0, btn_layout->geometry().height()));
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->start();
    connect(animation1, &QPropertyAnimation::finished, [=]{
        delete animation1;
        add_roll_button->hide();
    });

    QPropertyAnimation* animation2 = new QPropertyAnimation(add_chapter_button, "pos");
    animation2->setDuration(300);
    animation2->setStartValue(add_chapter_button->pos());
    animation2->setEndValue(add_chapter_button->pos()+QPoint(0, btn_layout->geometry().height()));
    animation2->setEasingCurve(QEasingCurve::OutQuad);
    animation2->start();
    connect(animation2, &QPropertyAnimation::finished, [=]{
        delete animation2;
        add_chapter_button->hide();
    });

    QPropertyAnimation* animation3 = new QPropertyAnimation(info_button, "pos");
    animation3->setDuration(300);
    animation3->setStartValue(info_button->pos());
    animation3->setEndValue(info_button->pos()+QPoint(0, btn_layout->geometry().height()));
    animation3->setEasingCurve(QEasingCurve::OutQuad);
    animation3->start();
    connect(animation3, &QPropertyAnimation::finished, [=]{
        delete animation3;
        info_button->hide();
    });*/

    /*QGraphicsOpacityEffect* effect1 = new QGraphicsOpacityEffect(add_roll_button);
    effect1->setOpacity(1.0);
    add_roll_button->setGraphicsEffect(effect1);
    QPropertyAnimation* ani1 = new QPropertyAnimation(effect1, "opacity");
    ani1->setDuration(300);
    ani1->setStartValue(1.0);
    ani1->setEndValue(0);
    connect(ani1, &QPropertyAnimation::finished, [=]{
        add_roll_button->setGraphicsEffect(NULL);
    });
    ani1->start();

    QGraphicsOpacityEffect* effect2 = new QGraphicsOpacityEffect(add_chapter_button);
    effect2->setOpacity(1.0);
    add_chapter_button->setGraphicsEffect(effect2);
    QPropertyAnimation* ani2 = new QPropertyAnimation(effect2, "opacity");
    ani2->setDuration(300);
    ani2->setStartValue(1.0);
    ani2->setEndValue(0);
    connect(ani2, &QPropertyAnimation::finished, [=]{
        add_chapter_button->setGraphicsEffect(NULL);
    });
    ani2->start();

    QGraphicsOpacityEffect* effect3 = new QGraphicsOpacityEffect(info_button);
    effect3->setOpacity(1.0);
    info_button->setGraphicsEffect(effect3);
    QPropertyAnimation* ani3 = new QPropertyAnimation(effect3, "opacity");
    ani3->setDuration(300);
    ani3->setStartValue(1.0);
    ani3->setEndValue(0);
    connect(ani3, &QPropertyAnimation::finished, [=]{
        info_button->setGraphicsEffect(NULL);
    });
    ani3->start();*/

    return QWidget::leaveEvent(event);
}

void NovelDirGroup::resizeEvent(QResizeEvent *event)
{
    /*qDebug() << "resize event";
    static bool _flag_is_not_resize = false;
    if (_flag_is_not_resize)
    {
        _flag_is_not_resize = false;
        return ;
    }

    if (novel_dir_listview != nullptr)
    {
        novel_dir_listview->setFixedHeight(height()-add_roll_button->height()-book_switch_combo->height()-3*5);
        _flag_is_not_resize = true;
        return ;
    }*/

    return QWidget::resizeEvent(event);
}

void NovelDirGroup::ensureDir(QString path) // 保证目录一定存在（未递归）
{
    QDir data_dir(path);
    if (!data_dir.exists())
        data_dir.mkdir(path);
}

bool NovelDirGroup::isPacking() // 是否折叠中
{
    return is_packing;
}

bool NovelDirGroup::isAnimating()
{
    return is_animating;
}

NovelDirListView* NovelDirGroup::getListView()
{
    return novel_dir_listview;
}

void NovelDirGroup::startHideSidebar(int distance) // 向左为负
{
    is_packing = true;

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(geometry().left()+distance, geometry().top(), geometry().width(), geometry().height()));
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->start();

    QGraphicsOpacityEffect *m_pGraphicsOpacityEffect = new QGraphicsOpacityEffect(this);
    m_pGraphicsOpacityEffect->setOpacity(1);
    this->setGraphicsEffect(m_pGraphicsOpacityEffect);

    QPropertyAnimation *animation2 = new QPropertyAnimation(m_pGraphicsOpacityEffect, "opacity", this);
    animation2->setDuration(200);
    animation2->setStartValue(1);
    animation2->setEndValue(0);
    animation2->setEasingCurve(QEasingCurve::InOutQuad);
    animation2->start();
}

void NovelDirGroup::startShowSidebar(int distance) // 向右为正
{
    is_packing = false;
    is_animating = true;

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);
    animation->setStartValue(geometry());
    animation->setEndValue(QRect(geometry().left()+distance, geometry().top(), geometry().width(), geometry().height()));
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->start();

    QGraphicsOpacityEffect *opacity_effect = new QGraphicsOpacityEffect(this);
    opacity_effect->setOpacity(0);
    this->setGraphicsEffect(opacity_effect);

    QPropertyAnimation *animation2 = new QPropertyAnimation(opacity_effect, "opacity", this);
    animation2->setDuration(250);
    animation2->setStartValue(0.2);
    animation2->setEndValue(1);
    animation2->setEasingCurve(QEasingCurve::InOutQuad);
    animation2->start();

    /*QTimer * timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(400);
    connect(timer, &QTimer::timeout, [=]{
        is_animating = false;
        delete timer;
    });*/

}

void NovelDirGroup::switchBook(int index)
{
    int count = book_switch_combo->count();
    if (index == count-1) // 创建新书
    {
        createBook();
        return ;
    }

    QString novel_name = book_switch_combo->currentText();

    novel_dir_listview->readNovel(novel_name);

    us->setVal("recent/novel", novel_name);
}

void NovelDirGroup::createBook()
{
    // 输入书名
    bool isOk;
    QString new_name = "", tip = "请输入新书书名（不包含特殊字符）";

    while (1)
    {
        new_name = QInputDialog::getText(this, "创建新书", tip, QLineEdit::Normal, new_name, &isOk);
        if (!isOk || new_name.isEmpty()) return ;

        // 判断是否包含特殊字符
        QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\''};
        int i = 0;
        for (i = 0; i < 10; i++)
        {
            if (new_name.contains(cs[i]))
            {
                tip = tr("书名包含特殊字符【%1】，请重新输入").arg(cs[i]);
                //QMessageBox::information(NULL, tr("创建新书失败"), tr("包含特殊字符【%1】啦！").arg(cs[i]));
                break;
            }
        }
        if (i == 10) break;
    }

    // 检验重复书籍
    QDir bookDir(rt->NOVEL_PATH+new_name);
    if (bookDir.exists())
    {
        QMessageBox::information(this, tr("创建新书失败"), tr("已经有同名书籍《%1》啦！").arg(new_name));
        return ;
    }

    // 开始创建文件夹
    bookDir.mkdir(rt->NOVEL_PATH+new_name);
    bookDir.mkdir(rt->NOVEL_PATH+new_name+"/chapters");
    bookDir.mkdir(rt->NOVEL_PATH+new_name+"/details");
    bookDir.mkdir(rt->NOVEL_PATH+new_name+"/outlines");
    bookDir.mkdir(rt->NOVEL_PATH+new_name+"/recycles");

    // 读取默认目录模板
    QString def_dir = "", lineStr = "";
    QFile def_dir_file(":/default/dir");
    def_dir_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream tsr(&def_dir_file);
        tsr.setCodec("UTF-8");
        while (!tsr.atEnd())
        {
            lineStr = tsr.readLine();
            def_dir += lineStr;
        }
    def_dir_file.close();

    // 创建默认目录
    QFile dir_file(rt->NOVEL_PATH+new_name+"/"+NOVELDIRFILENAME);
    dir_file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream tsw(&dir_file);
        tsw.setCodec("UTF-8");
        tsw << def_dir << endl;
    dir_file.close();

    // 重新读取目录(设置+目录）
    novel_dir_listview->readNovel(new_name);

    // 添加到目录列表
    int count = book_switch_combo->count();
    book_switch_combo->insertItem(count-1, new_name);
    book_switch_combo->setCurrentIndex(count);
    book_switch_combo->setCurrentText(new_name);

     // 保存到最近作品
    us->setVal("recent/novel", new_name);
}

void NovelDirGroup::slotRenameNovel()
{
    QString novel_name = novel_dir_listview->getNovelName();
    bool b;
    QString new_name = QInputDialog::getText(this, "修改书名", "请输入新的作品名字（不包含特殊符号）", QLineEdit::Normal, novel_name, &b);
    if (!b || new_name.isEmpty())
        return ;

    // 判断是否包含特殊字符
    QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\''};
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        if (new_name.contains(cs[i]))
        {
            QMessageBox::information(this, tr("修改书名失败"), tr("看起来好像似乎有可能包含了特殊字符【%1】哦~").arg(cs[i]));
            return ;
        }
    }

    if (new_name == novel_name)
        return ;

    // 检验重复书籍
    QDir book_dir(rt->NOVEL_PATH+new_name);
    if (book_dir.exists())
    {
        QMessageBox::information(this, tr("修改书名失败"), tr("已经有同名书籍《%1》啦！").arg(new_name));
        return ;
    }

    // 开始重命名路径
    renameFile(rt->NOVEL_PATH+novel_name, rt->NOVEL_PATH+new_name);
    us->setVal("recent/novel", new_name);
    novel_new_name_temp = new_name;
    initDir();

    // 发出信号（编辑器修改名字）
    emit signalRenameNovel(novel_name, new_name);
}

void NovelDirGroup::slotDeleteNovel()
{
    QString novel_name = novel_dir_listview->getNovelName();
    QString recycle_path;
    int exist_index = 0;
    if (isFileExist(rt->DATA_PATH+"recycles/"+novel_name))
        exist_index++;
    while (isFileExist(rt->DATA_PATH+"recycles/"+novel_name+"("+QString("%1").arg(exist_index)+")"))
        exist_index++;
    if (exist_index > 0)
        recycle_path = rt->DATA_PATH+"recycles/"+novel_name+"("+QString("%1").arg(exist_index)+")";
    else
        recycle_path = rt->DATA_PATH+"recycles/"+novel_name;

    if (QMessageBox::question(this, QString("是否删除《%1》？").arg(novel_name), QObject::tr("删除后作品将会放到回收站，路径：%1/").arg(recycle_path), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return ;

    ensureDirExist(rt->DATA_PATH+"recycles");

    // 先发送删除的信号，其他地方接收到后，可以先进行保存操作，然后再删除
    emit signalDeleteNovel(novel_name);

    renameFile(rt->NOVEL_PATH+novel_name, recycle_path); // 重命名文件到回收站
    novel_dir_listview->readNovel(""); // 打开空的小说，即清空列表

    //novel_had_deleted_temp = true;
    us->setVal("recent/novel", "");
    initDir();
}

void NovelDirGroup::slotNeedLineGuide()
{
    rt->setLineGuideWidget(book_switch_combo);
}

void NovelDirGroup::slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    add_chapter_button->setEnabled(current.isValid());
}

void NovelDirGroup::slotFixedWidthChanged(int x)
{
    if (fixed_max_width+x >= 50 && fixed_max_width+x <= parentWidget()->width()>>1)
    {
        fixed_max_width += x;
        this->setMaximumWidth(fixed_max_width);
        us->setVal("layout/sidebar_width", fixed_max_width);
    }
}

void NovelDirGroup::slotInfoClicked()
{
    QPoint pos = QCursor::pos();
    pos = mapFromGlobal(pos);
    pos.setY(pos.y()); // 相对于整个窗口来说的
    emit signalShowInfo(novel_dir_listview->getNovelName(), pos);
}

void NovelDirGroup::slotAddRollButtonClicked()
{
    QRect r = add_roll_button->rect();
    QRect rect(r.left(), r.top()+btn_layout->geometry().top()-novel_dir_listview->geometry().top(), r.width(), r.height());
    novel_dir_listview->slotExternRect(rect);

    QPoint p = QCursor::pos();
    p = this->mapFromGlobal(p);
    QPoint point(p.x(), p.y());
    novel_dir_listview->slotExternPoint(point);

    novel_dir_listview->slotAddRoll();
}

void NovelDirGroup::slotAddChapterButtonClicked()
{
    QRect r = add_chapter_button->rect();
    QRect rect(r.left(), r.top()+btn_layout->geometry().top()-novel_dir_listview->geometry().top(), r.width(), r.height());
    novel_dir_listview->slotExternRect(rect);

    QPoint p = QCursor::pos();
    p = this->mapFromGlobal(p);
    QPoint point(p.x(), p.y());
    novel_dir_listview->slotExternPoint(point);

    novel_dir_listview->slotAddChapter();
}

void NovelDirGroup::slotImportFinished(QString name)
{
    Q_UNUSED(name);
    initDir();
}

