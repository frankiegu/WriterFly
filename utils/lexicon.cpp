#include "lexicon.h"

Lexicons::Lexicons(QString path)
{
    lexicon_dir = path;
    search_last = "";
    us = nullptr;

    synonym_inited = related_inited = sentence_inited = surname_inited = name_inited = random_inited = false;
    matched_case = COMPLETER_CONTENT_NONE;
}

void Lexicons::setUS(USettings* us)
{
    this->us = us;
}

void Lexicons::initAll(bool syno, bool rela, bool sent, bool random)
{
    if (syno)
    {
        initSynonym();
    }

    if (rela)
    {
        initRelated();
    }

    if (sent)
    {
        initSentence();
    }

    if (random)
    {
        initRandomName();
    }
}

void Lexicons::initSynonym()
{
    synonym_inited = true;
    search_last = "";
    QString path = lexicon_dir + "synonym/synonym.txt";
    if (isFileExist(path))
        synonym_text = readTextFile(path);
}

void Lexicons::initRelated()
{
    related_inited = true;
    search_last = "";
    QString path = lexicon_dir + "related/";
    QStringList files;
    QStringList filters;
    filters << "*.txt";
    QDir dir(path);
    files = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name);

    related_text = "";
    for (QString n : files)
    {
        QString text = readTextFile(path + n);
        related_text += text;
    }
}

void Lexicons::initSentence()
{
    sentence_inited = true;
    search_last = "";
    QString path = lexicon_dir + "sentence/";
    QStringList files;
    QStringList filters;
    filters << "*.txt";
    QDir dir(path);
    files = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name); // 获取文件名字列表（只带后缀名）

    sentence_text = "";
    for (QString f : files)
    {
        QString text = readTextFile(path+f);
        sentence_text += text + "\n";
    }
}

void Lexicons::initRandomName()
{
    random_inited = true;
    surname_inited = name_inited = true;
    search_last = "";
    QString path = lexicon_dir + "random/";
    QStringList files;
    QStringList filters;
    filters << "*.txt";
    QDir dir(path);
    files = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name);

    random_sort_list.clear();
    random_text_list.clear();
    for (QString f : files)
    {
        QString text = readTextFile(path+f);
        if (f.endsWith(".txt"))
            f.chop(4);
        random_sort_list.append(f);
        random_text_list.append(text);
    }

    // 将后缀一样的内容排序，比如“地名”和“奇幻地名”
    for (int i = 0; i < random_sort_list.size()-1; i++)
        for (int j = i+1; j < random_sort_list.size(); j++)
        {
            if (random_sort_list.at(j).endsWith(random_sort_list.at(i)))
            {
                random_sort_list.swap(i, j);
                random_text_list.swap(i, j);
            }
        }

}

void Lexicons::uninitAll(bool syno, bool rela, bool sent, bool random)
{
    if (syno)
    {
        synonym_inited = false;
    }

    if (rela)
    {
        related_inited = false;
    }

    if (sent)
    {
        sentence_inited = false;
    }

    if (random)
    {
        random_inited = false;
        surname_inited = name_inited = false;
    }
}

void Lexicons::test()
{
    int TIMES = 2000;

    QString key = "文化";
    int key_len = key.length();
    QStringList search_result;

    // ==========text==========
    QTime timedebuge1;//声明一个时钟对象
    timedebuge1.start();//开始计时
    QString syno_text;
    for (int i = 0; i < TIMES; i++)
    {
        syno_text = readTextFile(lexicon_dir+"synonym/synonym.txt");
    }
    qDebug()<<"加载1耗时："<<timedebuge1.elapsed()/1000.0<<"s";//输出计时

    // ===========stringlist=========
    QTime timedebuge2;//声明一个时钟对象
    timedebuge2.start();//开始计时
    QStringList syno_list;
    for (int i = 0; i < TIMES; i++)
    {
        syno_list = syno_text.split("\n", QString::SkipEmptyParts);
    }
    qDebug()<<"加载2耗时："<<timedebuge2.elapsed()/1000.0<<"s";//输出计时

    /*// ==========list<stringlist>==========
    QTime timedebuge5;//声明一个时钟对象
    timedebuge5.start();//开始计时
    QList<QStringList> syno_list_list;
    for (int i = 0; i < TIMES; i++)
    {
        syno_list = syno_text.split("\n", QString::SkipEmptyParts);
        syno_list_list.clear();
        int list_size = syno_list.size();
        for (int i = 0; i < list_size; i++)
        {
            syno_list_list.append(syno_list.at(i).split(" ", QString::SkipEmptyParts));
        }
    }
    qDebug()<<"加载3耗时："<<timedebuge5.elapsed()/1000.0<<"s";//输出计时*/

    // ==========text==========
    QTime timedebuge3;//声明一个时钟对象
    timedebuge3.start();//开始计时
    for (int i = 0; i < TIMES; i++)
    {
        search_result.clear();
        int pos = 0;
        while (1)
        {
            //pos = syno_text.indexOf(key, pos);
            pos = syno_text.indexOf(QRegExp("\\b"+key+"\\b"), pos);
            if (pos == -1) break;
            int left = syno_text.lastIndexOf("\n", pos)+1;
            int right = syno_text.indexOf("\n", pos);
            if (right == -1) right = syno_text.length();
            QString para = syno_text.mid(left, right-left);
            QStringList list = para.split(" ", QString::SkipEmptyParts);
            search_result.append(list);
            pos += key_len;
        }
    }
    qDebug() << "text length:" << syno_text.length();
    qDebug() << search_result;
    qDebug()<<"搜索1耗时："<<timedebuge3.elapsed()/1000.0<<"s";//输出计时

    // ===========stringlist=========
    QTime timedebuge4;//声明一个时钟对象
    timedebuge4.start();//开始计时
    for (int i = 0; i < TIMES; i++)
    {
        search_result.clear();
        int list_size = syno_list.size();
        for (int i = 0; i < list_size; i++)
        {
            if (syno_list.at(i).indexOf(key) >= 0)
            {
                QStringList list = syno_list.at(i).split(" ", QString::SkipEmptyParts);
                search_result.append(list);
            }
        }
    }
    qDebug() << "list size:" << syno_list.size();
    qDebug() << search_result;
    qDebug()<<"搜索2耗时："<<timedebuge4.elapsed()/1000.0<<"s";//输出计时

    /*// ==========list<stringlist>==========
    QTime timedebuge6;//声明一个时钟对象
    timedebuge6.start();//开始计时
    for (int i = 0; i < TIMES; i++)
    {
        search_result.clear();
        int list_size = syno_list.size();
        for (int i = 0; i < list_size; i++)
        {
            if (syno_list.at(i).indexOf(key) >= 0)
            {
                search_result.append(syno_list_list.at(i));
            }
        }
    }
    qDebug() << "list size:" << syno_list.size();
    qDebug() << search_result;
    qDebug()<<"搜索3耗时："<<timedebuge6.elapsed()/1000.0<<"s";//输出计时*/

}

bool Lexicons::surroundSearch(QString sent, int cursor)
{
    int len = sent.length();
    search_result.clear();
    match_sentence = false;
    bool find = false;
    matched_key = "";
    int start_pos = 0;
    QString l1 = "", l2 = "", l4 = "";

    if (cursor >= 1)
        l1 = sent.mid(cursor-1, 1);
    if (cursor >= 2)
        l2 = sent.mid(cursor-2, 2);
    if (cursor >= 4)
        l4 = sent.mid(cursor-4, 4);

    if (random_inited)
    {
        // 随机种类列表
        if (((matched_key = l2) == "随机")
         || ((matched_key = l2) == "取名")
         || ((matched_key = l4) == "随机取名"))
            if (searchRandom("随机取名"))
            {
                matched_case = COMPLETER_CONTENT_RAND_LIST;
                return true;
            }

        // 姓氏
        if (surname_inited && (((matched_key = l1) == "姓")
         || ((matched_key = l2) == "姓氏")))
            if (searchRandom("姓氏"))
            {
                matched_case = COMPLETER_CONTENT_SURN;
                return true;
            }

        // 人名
        if (name_inited && (((matched_key = l2) == "人名")
         || ((matched_key = l2) == "名字")))
            if (searchRandom("人名"))
            {
                matched_case = COMPLETER_CONTENT_NAME;
                return true;
            }

        // 随机列表
        for (QString s : random_sort_list)
        {
            if (cursor >= s.length() && sent.mid(cursor-s.length(), s.length()) == s && isFileExist(lexicon_dir + "random/" + s + ".txt"))
            {
                matched_key = s;
                matched_case = COMPLETER_CONTENT_RAND;
                searchRandom(s);
                return true;
            }
        }
    }

    // 搜索4个字
    if (!find)
    {
        start_pos = 0; // 开始搜索的位置
        if (start_pos < cursor-4) start_pos = cursor-4;
        for (int i = start_pos; i <= len-4 && i < cursor; i++)
        {
            if (search(sent.mid(i, 4), true))
            {
                find = true;
                matched_key = sent.mid(i, 4);
                break;
            }
        }
    }

    // 搜索三个字
    if (!find)
    {
        start_pos = 0;
        if (start_pos < cursor-3) start_pos = cursor-3;
        for (int i = start_pos; i <= len-3 && i < cursor; i++)
            if (search(sent.mid(i, 3), true))
            {
                find = true;
                matched_key = sent.mid(i, 3);
                break;
            }
    }

    // 搜索两个字
    if (!find)
    {
        start_pos = 0;
        if (start_pos < cursor-2) start_pos = cursor-2;
        for (int i = start_pos; i <= len-2 && i < cursor; i++)
            if (search(sent.mid(i, 2), true))
            {
                find = true;
                matched_key = sent.mid(i, 2);
                break;
            }
    }
    //qDebug() << "surround search:" << matched_key << "  result:" << search_result;
    search_last = sent;
    matched_case = COMPLETER_CONTENT_WORD;
    return find;
}

bool Lexicons::search(QString key, bool add)
{
    if (key.isEmpty()) return false;

    /* // 上次搜索的缓冲区，但是没必要了，因为每次surroundSearch的时候
     * // 都把上次的搜索结果清空了，key还在但是result没了
    if (key == search_last)
    {
        return true;
    }*/

    if (!add) search_result.clear();
    bool find = false;
    int key_len = key.length();

    if (synonym_inited)
    {
        QStringList synonym_list;
        int pos = 0;
        while (1)
        {
            //pos = synonym_text.indexOf(key, pos);
            pos = synonym_text.indexOf(QRegExp("\\b"+key+"\\b"), pos);
            if (pos == -1) break;
            int left = synonym_text.lastIndexOf("\n", pos)+1;
            int right = synonym_text.indexOf("\n", pos);
            if (right == -1) right = synonym_text.length();
            QString para = synonym_text.mid(left, right-left);
            QStringList list = para.split(" ", QString::SkipEmptyParts);
            // TODO 后期将改成 QList<CompleterItem>
            synonym_list.append(list);
            pos += key_len;
            find = true;
        }
        if (synonym_list.size() > 0 && shouldRandom())
        {
            if (shouldRandom())
            {
                std::random_shuffle(synonym_list.begin(), synonym_list.end());
            }
            search_result.append(synonym_list);
        }
    }
    if (related_inited)
    {
        QStringList related_list;
        int pos = 0;
        while (1)
        {
            pos = related_text.indexOf(QRegExp("\\b"+key+"\\b"), pos);
            if (pos == -1) break;
            if (pos > 0 && related_text.mid(pos-1, 1) == "{") // 是标题
            {
                int left = related_text.indexOf("[", pos)+1;
                int right = related_text.indexOf("]", pos);
                if (right < left-1) // 出现了错误
                {
                    pos = left+1;
                    continue;
                }
                if (right == -1) right = related_text.length();
                QString para = related_text.mid(left, right-left);
                QStringList list = para.split(" ", QString::SkipEmptyParts);
                // TODO 后期将改成 QList<CompleterItem>
                related_list.append(list);
                pos = right;
            }
            else // 是内容
            {
                int left = related_text.lastIndexOf("[", pos)+1;
                int right = related_text.indexOf("]", pos);
                if (right == -1) right = related_text.length();
                QString para = related_text.mid(left, right-left);
                QStringList list = para.split(" ", QString::SkipEmptyParts);
                // TODO 后期将改成 QList<CompleterItem>
                related_list.append(list);
                pos = right;
            }

            find = true;
        }
        if (related_list.size() > 0)
        {
            if (shouldRandom())
            {
                std::random_shuffle(related_list.begin(), related_list.end());
            }
            search_result.append(related_list);
        }
    }
    if (sentence_inited && find)
    {
        if (sentence_text.indexOf(key) > -1)
        {
            match_sentence = true;
            search_result.append("-->");
        }
    }
    //qDebug() << "search:" << key << "  result:" << search_result;
    return find;
}

bool Lexicons::searchRandom(QString key)
{
	// ==== 随机取名列表 ====
	if (key == "随机取名" || key == "随机" || key == "取名")
	{
		for (QString s : random_sort_list)
            search_result.append(s);
        std::random_shuffle(search_result.begin(), search_result.end());
		return true;
	}

	// ==== 随机取名具体 ====
    for (int i = 0; i < random_sort_list.size(); i++)
        if (random_sort_list.at(i) == key)
		{
            QStringList list = random_text_list.at(i).split(" ", QString::SkipEmptyParts);
            search_result = list;
            std::random_shuffle(search_result.begin(), search_result.end());
            search_result = search_result.mid(0, 100);
	        return true;
		}

	return false;
}

QStringList Lexicons::getResult()
{
    return search_result;
}

QString Lexicons::getMatched()
{
    return matched_key;
}

int Lexicons::getMatchedCase()
{
    return matched_case;
}

QStringList Lexicons::getSentence()
{
    return getSentence(matched_key);
}

QStringList Lexicons::getSentence(QString key)
{
    search_result.clear();
    if (key.isEmpty()) return search_result;
    matched_case = COMPLETER_CONTENT_SENT;

    int pos = 0;
    while (1)
    {
        pos = sentence_text.indexOf(key, pos);
        if (pos == -1) break;
        int left = sentence_text.lastIndexOf("\n", pos)+1;
        int right = sentence_text.indexOf("\n", pos);
        if (right == -1) right = sentence_text.length();
        QString para = sentence_text.mid(left, right-left);
        // TODO 后期将改成 QList<CompleterItem>
        search_result.append(para);
        pos = right+1;
    }
    if (shouldRandom())
    {
        std::random_shuffle(search_result.begin(), search_result.end());
    }
    return search_result;
}

bool Lexicons::shouldRandom()
{
    if (us == nullptr)
        return true;
    return us->completer_random;
}
