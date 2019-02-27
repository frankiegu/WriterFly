#include "transparentcontainer.h"

TransparentContainer::TransparentContainer(QWidget *parent) : QWidget(parent)
{
    showing = false;
    animating = false;

    setAutoFillBackground(true);

    bgmap = new QPixmap(parent->size());
    blur_effect = new QGraphicsBlurEffect(this);
    blur_effect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
    this->setGraphicsEffect(blur_effect);

    this->hide();
}

void TransparentContainer::toShow()
{
    setGeometry(QRect(QPoint(0, 0), parentWidget()->size()));
    bgmap = new QPixmap(parentWidget()->size());
    parentWidget()->render(bgmap);

    QPalette palette; // 使用palette方法绘制背景图
    palette.setBrush(QPalette::Background, QBrush(*bgmap));
    setPalette(palette);

    showing = true;
    animating = true;
    show();

    QPropertyAnimation* ani = new QPropertyAnimation(this, "blur_radius");
    ani->setDuration(300);
    ani->setStartValue(0);
    ani->setEndValue(MAX_BLUR_RADIUS);
    ani->start();
    connect(ani, SIGNAL(finished()), this, SLOT(slotAniFinished()));
}

void TransparentContainer::toHide()
{

    showing = false;
    animating = true;

    QPropertyAnimation* ani = new QPropertyAnimation(this, "blur_radius");
    ani->setDuration(300);
    ani->setStartValue(MAX_BLUR_RADIUS);
    ani->setEndValue(0);
    ani->start();
    connect(ani, SIGNAL(finished()), this, SLOT(slotAniFinished()));

    emit signalToClose(show_type);
}

void TransparentContainer::setShowType(int t)
{
    show_type = t;
}

void TransparentContainer::paintEvent(QPaintEvent *event)
{
    /*if (showing)
    {
        QPainter painter(this); // 使用 paint 事件绘制背景图
        painter.drawPixmap(0, 0, this->width(), this->height(), *bgmap);
    }*/
    QWidget::paintEvent(event);
}

void TransparentContainer::mousePressEvent(QMouseEvent *)
{
    if (animating || !showing) return ;
    toHide();
}

void TransparentContainer::keyPressEvent(QKeyEvent *event)
{
    deb(event->key(), "trans key");
    if (event->key() == Qt::Key_Escape)
    {
        deb("esc trans");
        if (animating || !showing) return ;
        toHide();
    }

    return QWidget::keyPressEvent(event);
}

void TransparentContainer::resizeEvent(QResizeEvent *event)
{
    if (bgmap != nullptr) {
        ;
    }//delete bgmap; // 应该用的，但是闪退了。。。
    if (!showing)
        return QWidget::resizeEvent(event);
    bgmap = new QPixmap(event->size());
    parentWidget()->render(bgmap);
    QWidget::resizeEvent(event);
}

void TransparentContainer::slotAniFinished()
{

    if (showing)
    {
        ;
    }
    else
    {
        emit signalClosed(show_type);
        hide(); // 动画结束隐藏
    }
    animating = false;
}

void TransparentContainer::slotEsc()
{
    mousePressEvent(nullptr);
}

int TransparentContainer::getBlurRadius()
{
    return blur_radius;
}

void TransparentContainer::setBlurRadius(int x)
{
    blur_radius = x;
    blur_effect->setBlurRadius(x);
}

void TransparentContainer::resetBlurRadius()
{
    blur_radius = 0;
}
