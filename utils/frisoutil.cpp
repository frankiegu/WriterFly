#include "frisoutil.h"


FrisoUtil::FrisoUtil()
{
    inited = false;
    initing = false;
    valid = true;
}

FrisoUtil::~FrisoUtil()
{
    Destructor();
}

QStringList FrisoUtil::WordSegment(QString _text)
{
    Q_UNUSED(_text);
#if defined(Q_OS_WIN)
    if (!valid)
        return sList;

    if (!inited)
    {
        init();
        if (inited == false) // 初始化失败
        {
            QStringList list;
            int len = _text.length();
            for (int i = 0; i < len; i++)
                list.append(_text.mid(i, 1));
            return list;
        }
    }

    if (_text == _recent) return sList;
    _recent = _text;

    friso_task_t task = friso_new_task();
    fstring text = _text.toUtf8().data();
    friso_set_text(task, text);

    sList.clear();
    while ( (friso_next(friso, config, task)) != nullptr )
    {
        sList.append(task->hits->word);
    }

    friso_free_task(task);
#endif
    return sList;
}

void FrisoUtil::Destructor()
{
#if defined(Q_OS_WIN)
    friso_free_config(config);
    friso_free(friso);
#endif
}

bool FrisoUtil::init()
{
#if defined(Q_OS_Android)
    initing = true;
    valid = false;
#elif defined(Q_OS_WIN)
    if (initing) return false;
    initing = true;

    char pa[1000] = "";
    strcpy(pa, QApplication::applicationDirPath().toLocal8Bit().data());
    strcat(pa, "/tools/friso/friso.ini");
    fstring _ifile = pa;
    friso = friso_new();
    config = friso_new_config();
    if (friso_init_from_ifile(friso, config, _ifile) != 1) {
        qDebug() << "fail to initialize friso and config.";
        return initing = false;
    }
    initing = false;
    inited = true;
#endif
    return true;
}
