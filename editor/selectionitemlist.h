#ifndef SELECTIONITEMLIST_H
#define SELECTIONITEMLIST_H

#include <QList>
#include <QTextCursor>
#include <QDebug>
#include "selectionitem.h"

class SelectionItemList
{
public:
    SelectionItemList();

    void clear();
    void append(SelectionItem si);
    void append(int s, int e, int p = -1);
    void append(QTextCursor c);
    SelectionItem get(int index = -1);
    SelectionItem back();
    SelectionItem next();

    bool getCallback(SelectionItem si, int &s, int &e);
    bool getCallback(int &s, int &e);

private:
    void clearRedundant();

private:
    QList<SelectionItem>list;
    int position;
};

#endif // SELECTIONITEMLIST_H
