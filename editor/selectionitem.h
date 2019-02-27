#ifndef SELECTIONITEM_H
#define SELECTIONITEM_H


class SelectionItem
{
public:
    SelectionItem()
    {
        pos = start = end = -1;
    }

    SelectionItem(int s, int e) : start(s), end(e), pos(-1)
    {

    }

    SelectionItem(int s, int e, int p) : start(s), end(e), pos(p)
    {

    }

public:
    int start;
    int end;
    int pos;
};

#endif // SELECTIONITEM_H
