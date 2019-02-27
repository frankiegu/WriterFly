#ifndef EDITORINPUTMANAGER_H
#define EDITORINPUTMANAGER_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QList>
#include <QTextCharFormat>
#include <QDebug>
#include "editorinputcharactor.h"

class EditorInputManager : public QObject
{
    Q_OBJECT
public:
    EditorInputManager();

    void setEditor(QTextEdit* edit);
    void setFlag(bool* flag);

    void setColor(QColor color);

    void textChanged(int old_position, int diff);
    void updateRect(int range_start, int rande_end);

    void addInputAnimation(QPoint point, QString str, int position, int delay, int duration);

    void updateTextColor(int current_position);

public slots:
    void aniFinished(int position, EditorInputCharactor *);

private:
    QTextEdit* _edit;
    bool* _flag_is_cursor_change;
    QColor font_color;
    QList<EditorInputCharactor*> ani_list;
};

#endif // EDITORINPUTMANAGER_H
