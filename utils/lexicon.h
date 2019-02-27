#ifndef LEXICON_H
#define LEXICON_H

#include <QString>
#include <QStringList>
#include <QApplication>
#include <QDebug>
#include <QTime>
#include <QList>
#include <QRegExp>
#include "fileutil.h"
#include "usettings.h"

#define COMPLETER_CONTENT_NONE 0 // 无
#define COMPLETER_CONTENT_WORD 1 // 词语
#define COMPLETER_CONTENT_SENT 2 // 句子
#define COMPLETER_CONTENT_RAND_LIST 3 // 随机种类列表
#define COMPLETER_CONTENT_SURN 4 // 姓氏
#define COMPLETER_CONTENT_NAME 5 // 名字
#define COMPLETER_CONTENT_RAND 6 // 随机具体

class Lexicons
{
public:
    Lexicons(QString path);

    void setUS(USettings *us);
    void initAll(bool syno, bool rela, bool sent, bool random);
    void initSynonym();
    void initRelated();
    void initSentence();
    void initRandomName();
    void uninitAll(bool, bool, bool, bool random);
    void test();

    bool surroundSearch(QString sent, int cursor/*相对*/); // 包含光标前四个字符、后三个字母，长度每四三二都搜索过去
    bool search(QString key, bool add = false);
    bool searchRandom(QString key);
    QStringList getResult();
    QString getMatched();
    int getMatchedCase();
    QStringList getSentence();
    QStringList getSentence(QString key);

private:
    bool shouldRandom();

private:
    QString lexicon_dir;

    bool synonym_inited, related_inited, sentence_inited, surname_inited, name_inited, random_inited;
    QString synonym_text, related_text, sentence_text, surname_text, name_text;
    QStringList random_sort_list, random_text_list;

    QString search_last, matched_key;
    QStringList search_result;
    int matched_case;
    bool match_sentence;

    USettings* us;
};

#endif // LEXICON_H
