#include "numberlabel.h"

NumberLabel::NumberLabel()
{
    show_num = 0;
}

NumberLabel::NumberLabel(QWidget *parent) : QLabel(parent)
{
    show_num = 0;
}

void NumberLabel::setShowNum(int x)
{
    dn = x - show_num;
    show_num = x;
    this->setText(QString("%1").arg(x));

    QPropertyAnimation* animation = new QPropertyAnimation(this, "animation_progress");
    animation->setDuration(500);
    animation->setStartValue(0);
    animation->setEndValue(100);
    animation->start();
}

void NumberLabel::setAnimationProgress(int x)
{
    animation_progress = x;

    int n = show_num - dn * (100-animation_progress) / 100;
    this->setText(QString("%1").arg(n));
}

int NumberLabel::getAnimationProgress()
{
    return animation_progress;
}

void NumberLabel::resetAnimationProgress()
{
    animation_progress = 0;
}


