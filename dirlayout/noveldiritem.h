#ifndef NOVELDIRITEM_H
#define NOVELDIRITEM_H

#include "chapterinfo.h"

class NovelDirItem : public ChapterInfo
{
public:
    NovelDirItem();
    NovelDirItem(bool roll, QString name);
    NovelDirItem(bool roll, QString name, int number);
    NovelDirItem(bool roll, bool hide, QString name, int number);

public:
    bool isRoll(); // 是否是卷
    bool isHide(); // 是否收起/隐藏中
    bool isAnimating();
    void setHide(bool state);
    void setAnimating(bool a);
    void setS_temp(QString s);
    QString getS_temp();

private:
    bool roll;
    bool hide;
    bool animating;
    QString s_temp;
};



#endif // NOVELDIRITEM_H
