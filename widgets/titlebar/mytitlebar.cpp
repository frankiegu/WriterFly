#include <QHBoxLayout>
#include <QPainter>
#include <QFile>
#include <QMouseEvent>
#include "mytitlebar.h"
#include "defines.h"
#include "globalvar.h"

MyTitleBar::MyTitleBar(QWidget *parent)
    : QWidget(parent), color_R(255), color_G(250), color_B(250), is_pressed(false), is_transparent(true)
{
    initControl();
    initConnections();
    initStyle();

    this->setTitleContent(APPLICATION_NAME);
}

MyTitleBar::~MyTitleBar()
{

}

void MyTitleBar::initControl()
{
    button_rect = QRect(0, 0, 1, 1);

    titlebar_icon = new QLabel;
    title_content_widget = new AniVLabel(this);
    title_content_widget->setFixedHeight(us->mainwin_titlebar_height);

    QPixmap* pixmap_min, *pixmap_restore, *pixmap_max, *pixmap_close, *pixmap_sidebar, *pixmap_settings;
    pixmap_min = new QPixmap(32, 32);
    pixmap_min->load(":/icons/win_min");
    pixmap_restore = new QPixmap(32, 32);
    pixmap_restore->load(":/icons/win_restore");
    pixmap_max = new QPixmap(32, 32);
    pixmap_max->load(":/icons/win_max");
    pixmap_close = new QPixmap(32, 32);
    pixmap_close->load(":/icons/win_close");
    pixmap_sidebar = new QPixmap(32, 32);
    pixmap_sidebar->load(":/icons/sidebar");
    pixmap_settings = new QPixmap(32, 32);
    pixmap_settings->load(":/icons/settings");

    QIcon *icon_min = new QIcon(*pixmap_min), *icon_restore = new QIcon(*pixmap_restore),
            *icon_max = new QIcon(*pixmap_max), *icon_close = new QIcon(*pixmap_close),
            *icon_sidebar = new QIcon(*pixmap_sidebar), *icon_settings = new QIcon(*pixmap_settings);

    min_btn = new QPushButton(*icon_min, "");
    restore_btn = new QPushButton(*icon_restore, "");
    max_btn = new QPushButton(*icon_max, "");
    close_btn = new QPushButton(*icon_close, "");
    sidebar_btn = new QPushButton(*icon_sidebar, "");
    settings_btn = new QPushButton(*icon_settings, "");

    min_btn->setFixedSize(QSize(us->mainwin_titlebar_height, us->mainwin_titlebar_height));
    restore_btn->setFixedSize(QSize(us->mainwin_titlebar_height, us->mainwin_titlebar_height));
    max_btn->setFixedSize(QSize(us->mainwin_titlebar_height, us->mainwin_titlebar_height));
    close_btn->setFixedSize(QSize(us->mainwin_titlebar_height, us->mainwin_titlebar_height));
    sidebar_btn->setFixedSize(QSize(us->mainwin_titlebar_height, us->mainwin_titlebar_height));
    settings_btn->setFixedSize(QSize(us->mainwin_titlebar_height, us->mainwin_titlebar_height));

    title_content_widget->setObjectName("Title");
    min_btn->setObjectName("Min");
    restore_btn->setObjectName("Restore");
    max_btn->setObjectName("Max");
    close_btn->setObjectName("Close");
    sidebar_btn->setObjectName("Sidebar");
    settings_btn->setObjectName("Settings");

    min_btn->setToolTip(QStringLiteral("最小化"));
    restore_btn->setToolTip(QStringLiteral("还原"));
    max_btn->setToolTip(QStringLiteral("最大化"));
    close_btn->setToolTip(QStringLiteral("关闭"));
    sidebar_btn->setToolTip(QStringLiteral("展开侧边栏"));
    settings_btn->setToolTip(QStringLiteral("写作天下设置"));

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->addWidget(sidebar_btn);
    hlayout->addWidget(settings_btn);
    //hlayout->addWidget(m_pIcon);
    titlebar_icon->setVisible(false);
    hlayout->addWidget(title_content_widget);

    hlayout->addWidget(min_btn);
    hlayout->addWidget(restore_btn);
    hlayout->addWidget(max_btn);
    hlayout->addWidget(close_btn);

    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setSpacing(0);

    title_content_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    this->setFixedHeight(us->mainwin_titlebar_height);
    this->setWindowFlags(Qt::FramelessWindowHint);

    restore_btn->setVisible(false);
}

void MyTitleBar::initConnections()
{
    connect(min_btn, SIGNAL(clicked()), this, SLOT(onButtonMinClicked()));
    connect(restore_btn, SIGNAL(clicked()), this, SLOT(onButtonRestoreClicked()));
    connect(max_btn, SIGNAL(clicked()), this, SLOT(onButtonMaxClicked()));
    connect(close_btn, SIGNAL(clicked()), this, SLOT(onButtonCloseClicked()));
    connect(sidebar_btn, SIGNAL(clicked()), this, SLOT(onButtonSidebarClicked()));
    connect(settings_btn, SIGNAL(clicked()), this, SLOT(onButtonSettingsClicked()));
}

void MyTitleBar::initStyle()
{
    QString styleSheet = QString("QPushButton {background-color: transparent; border: none; }\
                                QPushButton:hover {background-color: rgb(220, 220, 220); }\
                                QPushButton:pressed {background-color: rgb(192, 192, 192); }");
    min_btn->setStyleSheet(styleSheet);
    restore_btn->setStyleSheet(styleSheet);
    max_btn->setStyleSheet(styleSheet);
    sidebar_btn->setStyleSheet(styleSheet);
    settings_btn->setStyleSheet(styleSheet);
    QString styleSheet_redbg = QString("QPushButton {background-color: transparent; border: none; }\
                                       QPushButton:hover {background-color: rgb(240, 77, 64); }\
                                       QPushButton:pressed {background-color: rgb(255, 47, 34); }");
    close_btn->setStyleSheet(styleSheet_redbg);

    if (isFileExist(rt->STYLE_PATH + "titlebar.qss"))
        setStyleSheet(readTextFile(rt->STYLE_PATH + "titlebar.qss"));
    if (isFileExist(rt->STYLE_PATH + "titlebar_button.qss"))
    {
        styleSheet = readTextFile(rt->STYLE_PATH + "titlebar_button.qss");
        min_btn->setStyleSheet(styleSheet);
        restore_btn->setStyleSheet(styleSheet);
        max_btn->setStyleSheet(styleSheet);
        close_btn->setStyleSheet(styleSheet);
        sidebar_btn->setStyleSheet(styleSheet);
        settings_btn->setStyleSheet(styleSheet);
    }
}

void MyTitleBar::setBackgroundColor(int r, int g, int b, bool isTransparent)
{
    color_R = r;
    color_G = g;
    color_B = b;
    is_transparent = isTransparent;
    update(); // 重新绘制（调用paintEvent事件）
}

void MyTitleBar::setTitleIcon(QString filePath, QSize IconSize)
{
    QPixmap titleIcon(filePath);
    titlebar_icon->setPixmap(titleIcon.scaled(IconSize));
}

void MyTitleBar::setTitleContent(QString titleContent, int titleFontSize)
{
    Q_UNUSED(titleFontSize);
    // 设置标题字体大小;
    /*QFont font = m_pTitleContent->font();
    font.setPointSize(titleFontSize);
    m_pTitleContent->setFont(font);*/
    // 设置标题内容;
    title_content_widget->setMainText(titleContent);
    title_content = titleContent;
}

void MyTitleBar::setTitleContent2(QString titleContent, int titleFontSize)
{
    Q_UNUSED(titleFontSize);
    title_content_widget->setStaticText(titleContent);
    title_content = titleContent;
}

void MyTitleBar::setTitleWidth(int width)
{
    this->setFixedWidth(width);
}

void MyTitleBar::saveRestoreInfo(const QPoint point, const QSize size)
{
    restore_pos = point;
    restore_size = size;
    this->setGeometry(0, 0, size.width(), us->mainwin_titlebar_height);
}

void MyTitleBar::getRestoreInfo(QPoint &point, QSize &size)
{
    point = restore_pos;
    size = restore_size;
}

void MyTitleBar::setRestoreInfo(QPoint point, QSize size)
{
    restore_pos = point;
    restore_size = size;
}

void MyTitleBar::paintEvent(QPaintEvent *event)
{
    // 是否设置标题透明;
    if (!is_transparent)
    {
        //设置背景色;
        QPainter painter(this);
        QPainterPath pathBack;
        pathBack.setFillRule(Qt::WindingFill);
        pathBack.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(pathBack, QBrush(QColor(color_R, color_G, color_B)));
    }

    // 当窗口最大化或者还原后，窗口长度变了，标题栏的长度应当一起改变;
    // 这里减去m_windowBorderWidth ，是因为窗口可能设置了不同宽度的边框;
    // 如果窗口有边框则需要设置m_windowBorderWidth的值，否则m_windowBorderWidth默认为0;
    if (this->width() != (this->parentWidget()->width()))
    {
        this->setFixedWidth(this->parentWidget()->width());
    }
    QWidget::paintEvent(event);
}

void MyTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 通过最大化按钮的状态判断当前窗口是处于最大化还是原始大小状态;
    // 或者通过单独设置变量来表示当前窗口状态;
    if (max_btn->isVisible())
    {
        onButtonMaxClicked();
    }
    else
    {
        onButtonRestoreClicked();
    }


    return QWidget::mouseDoubleClickEvent(event);
}

void MyTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (!max_btn->isVisible())
    {
        // 在窗口最大化时禁止拖动窗口;
    }
    else
    {
        is_pressed = true;
        start_move_pos = event->globalPos();
    }

    return QWidget::mousePressEvent(event);
}

void MyTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (is_pressed)
    {
        QPoint movePoint = event->globalPos() - start_move_pos;
        QPoint widgetPos = this->parentWidget()->pos();
        this->parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
        start_move_pos = event->globalPos();
        //QMessageBox::information(NULL, "save", QString("%1, %2").arg(this->parentWidget()->pos().x()).arg(this->parentWidget()->pos().y()));
        emit windowChanged(this->parentWidget()->pos(), QSize(this->parentWidget()->width(), this->parentWidget()->height()));
        is_moved = true;
    }
    return QWidget::mouseMoveEvent(event);
}

void MyTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    is_pressed = false;
    if (is_moved) {
        emit changeFinished(this->parentWidget()->pos(), QSize(this->parentWidget()->width(), this->parentWidget()->height()));
        is_moved = false;
    }
    return QWidget::mouseReleaseEvent(event);
}

bool MyTitleBar::isMaxxing()
{
    return !max_btn->isVisible();
}

bool MyTitleBar::simulateMax()
{
    button_rect = max_btn->geometry();
    max_btn->setVisible(false);
    restore_btn->setVisible(true);
    emit signalButtonMaxClicked();
    return true;
}

QRect MyTitleBar::getBtnRect()
{
    return button_rect;
}

void MyTitleBar::onButtonMinClicked()
{
    button_rect = min_btn->geometry();
    emit signalButtonMinClicked();
}

void MyTitleBar::onButtonRestoreClicked()
{
    button_rect = restore_btn->geometry();
    restore_btn->setVisible(false);
    max_btn->setVisible(true);
    emit signalButtonRestoreClicked();
}

void MyTitleBar::onButtonMaxClicked()
{
    button_rect = max_btn->geometry();
    max_btn->setVisible(false);
    restore_btn->setVisible(true);
    emit signalButtonMaxClicked();
}

void MyTitleBar::onButtonCloseClicked()
{
    button_rect = close_btn->geometry();
    emit signalButtonCloseClicked();
}

void MyTitleBar::onButtonSidebarClicked()
{
    button_rect = sidebar_btn->geometry();
    emit signalButtonSidebarClicked();
}

void MyTitleBar::onButtonSettingsClicked()
{
    button_rect = settings_btn->geometry();
    emit signalButtonSettingsClicked();
}

void MyTitleBar::onRollTitle()
{
    // 已废除
}
