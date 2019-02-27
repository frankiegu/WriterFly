#ifndef FRISOTHREAD_H
#define FRISOTHREAD_H

#include <QThread>
#include "globalvar.h"

class FrisoThread : public QThread
{
    Q_OBJECT
public:
    FrisoThread(){}

protected:
    void run()
    {
        friso_AI->init();
    }
};

#endif // FRISOTHREAD_H
