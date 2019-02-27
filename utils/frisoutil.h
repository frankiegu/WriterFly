#ifndef FRISOUTIL_H
#define FRISOUTIL_H

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QApplication>
extern "C" {
    #include "friso\src\friso.h"
    #include "friso\src\friso_API.h"
}

class FrisoUtil
{
public:
    FrisoUtil();
    ~FrisoUtil();

    QStringList WordSegment(QString _text);

    void Destructor();
    bool init();

private:
    friso_t friso;
    friso_config_t config;

    bool inited;
    bool initing; // 表示正在初始化中，避免多线程冲突
    bool valid;
    QStringList sList; // 返回的列表，也是最近一次的列表
    QString _recent;   // 上一次的文本，如果一样的话则套用上一次的结果，以便加快速度
};

#endif // FRISOUTIL_H
