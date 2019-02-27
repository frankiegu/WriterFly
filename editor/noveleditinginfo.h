#ifndef NOVELEDITINGINFO_H
#define NOVELEDITINGINFO_H

#include <QString>
#include "globalvar.h"
#include "stringutil.h"

class NovelEditingInfo
{
public:
    NovelEditingInfo();

    void setNames(QString novel_name, QString chapter_name);
    void setChapterName(QString chapter_name); // 用于重命名
    void setNovelName(QString novel_name);
    void setPath(QString path);
    void setFullChapterName(QString full_chapter_name); // 带序号的章节名
    void setPos(int x);
    void setWc(int x);
    void setSavedText(QString text);

    QString getFullChapterName();
    QString getFullName(); // 书名/序号 章名 [字数]
    QString getNovelName();
    QString getChapterName();
    int getPos();
    int getWc();
    QString getPath();
    QString getSavedText();

private:
    QString novel_name;
    QString chapter_name;
    QString full_chapter_name;
    QString full_name;  // 要显示的全名：书名/序号 章名 [字数]
    QString saved_text; // 初始内容，判断有没有更改
    QString file_path;  // 文件路径
    int position;      // 编辑的位置
    int words_count;    // 字数

    int open_time;      // 打开时间，用来关闭最不常用的
    int edit_time;      // 编辑时间
    int frequency;     // 切换到这个标签页的次数
};

#endif // NOVELEDITINGINFO_H
