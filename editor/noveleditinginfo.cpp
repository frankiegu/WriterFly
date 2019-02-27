#include "noveleditinginfo.h"

NovelEditingInfo::NovelEditingInfo()
{
    words_count = 0;
}

void NovelEditingInfo::setNames(QString novelName, QString chapterName)
{
    this->novel_name = novelName;
    this->chapter_name = chapterName;

    this->full_name = novelName + " / " + chapterName;
    this->file_path = rt->NOVEL_PATH + fnEncode(novelName) + "/chapters/" + fnEncode(chapterName) + ".txt";
}

void NovelEditingInfo::setChapterName(QString chapterName)
{
    if (novel_name == "") return ;
    this->chapter_name = chapterName;
    this->full_name = novel_name + " / " + chapterName;
    this->file_path = rt->NOVEL_PATH + fnEncode(novel_name) + "/chapters/" + fnEncode(chapterName) + ".txt";
}

void NovelEditingInfo::setNovelName(QString novelName)
{
    this->novel_name = novelName;
    this->file_path = rt->NOVEL_PATH + fnEncode(novelName) + "/chapters/" + fnEncode(chapter_name) + ".txt";
}

void NovelEditingInfo::setPath(QString path)
{
    this->file_path = path;
}

void NovelEditingInfo::setFullChapterName(QString fullChapterName)
{
    this ->full_chapter_name = fullChapterName;
    if (fullChapterName != "")
        this->full_name = QString("%1 / %2").arg(novel_name).arg(fullChapterName);
    else
        this->full_name = QString("%1 / %2").arg(novel_name).arg(chapter_name);
}

void NovelEditingInfo::setSavedText(QString text)
{
    this->saved_text = text;
}

QString NovelEditingInfo::getFullChapterName()
{
    return full_chapter_name;
}

QString NovelEditingInfo::getFullName()
{
    return full_name; // 返回章节全内容
}

QString NovelEditingInfo::getNovelName()
{
    return novel_name;
}

QString NovelEditingInfo::getChapterName()
{
    return chapter_name;
}

void NovelEditingInfo::setPos(int x)
{
    this->position = x;
}

int NovelEditingInfo::getPos()
{
    return position;
}

void NovelEditingInfo::setWc(int x)
{
    this->words_count = x;
}

int NovelEditingInfo::getWc()
{
    return words_count;
}

QString NovelEditingInfo::getPath()
{
    return file_path;
}

QString NovelEditingInfo::getSavedText()
{
    return saved_text;
}
