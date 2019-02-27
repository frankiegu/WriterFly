#include "noveldiritem.h"

NovelDirItem::NovelDirItem() : hide(false), animating(false)
{

}

NovelDirItem::NovelDirItem(bool roll, QString name)
        : ChapterInfo(name), roll(roll), hide(false), animating(false)
{

}

NovelDirItem::NovelDirItem(bool roll, QString name, int number)
        : ChapterInfo(name, number), roll(roll), hide(false), animating(false)
{

}

NovelDirItem::NovelDirItem(bool roll, bool hide, QString name, int number)
        : ChapterInfo(name, number), roll(roll), hide(hide), animating(false)
{

}

bool NovelDirItem::isRoll()
{
    return roll;
}

bool NovelDirItem::isHide()
{
    return hide;
}

bool NovelDirItem::isAnimating()
{
    return animating;
}

void NovelDirItem::setHide(bool state)
{
    hide = state;
}

void NovelDirItem::setAnimating(bool a)
{
    animating = a;
}

void NovelDirItem::setS_temp(QString s)
{
    s_temp = s;
}

QString NovelDirItem::getS_temp()
{
    return s_temp;
}
