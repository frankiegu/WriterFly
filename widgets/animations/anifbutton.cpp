#include "anifbutton.h"

AniFButton::AniFButton()
{

}

AniFButton::AniFButton(QString caption) : QPushButton(caption)
{

}

AniFButton::AniFButton(QString caption, QWidget *parent) : QPushButton(caption, parent)
{
    //this->setStyleSheet(QString("AniButton { padding: 0px; }"));
    QString btnStyle("QPushButton{ background: transparent; border-style: outset; border-width: 1px; border-color: #88888888; padding: 5px; }\
                     QPushButton:hover {background-color: rgb(220, 220, 220, 200);}\
                     QPushButton:pressed {background-color: rgb(200, 200, 200, 200); border-style: inset; } ");
    this->setStyleSheet(btnStyle);

    font_size = this->font().pointSize();
    font_size_l = static_cast<int>(font_size * 1.5);

    QFont font = this->font();
    font.setPointSize(static_cast<int>(font_size * 2.5));
    QFontMetrics fm(font);
    int height = fm.height();
    this->setMinimumHeight(height);
}

void AniFButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "fontsize");
    animation->setDuration(300);
    animation->setStartValue(font_size);
    animation->setEndValue(font_size_l);
    animation->start();
}

void AniFButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "fontsize");
    animation->setDuration(300);
    animation->setStartValue(font_size_l);
    animation->setEndValue(font_size);
    animation->start();
}

void AniFButton::setFontSize(int x)
{
    QFont font = this->font();
    font.setPointSize(x);
    this->setFont(font);
}

int AniFButton::getFontSize()
{
    QFont font = this->font();
    return font.pixelSize();
}

void AniFButton::resetFontSize()
{
    setFontSize(font_size);
}

int AniFButton::getMiniHeight()
{
    QFont font = this->font();
    font.setPointSize(font_size * 3);
    QFontMetrics fm(font);
    int height = fm.height();
    return height;
}
