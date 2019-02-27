#ifndef LEXICONSTHREAD_H
#define LEXICONSTHREAD_H

#include <QThread>
#include "globalvar.h"

class LexiconsThread : public QThread
{
    Q_OBJECT
public:
    LexiconsThread(){}

protected:
    void run()
    {
        lexicon_AI->initAll(us->lexicon_synonym, us->lexicon_related, us->lexicon_sentence, us->lexicon_random);
    }
};

#endif // LEXICONSTHREAD_H
