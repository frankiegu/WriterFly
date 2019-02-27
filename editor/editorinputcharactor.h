#ifndef EDITORINPUTCHARATOR_H
#define EDITORINPUTCHARATOR_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QFont>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QTimer>
#include <QDebug>
#include <QPropertyAnimation>
#include <QPixmap>

class EditorInputCharactor : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int fontsize READ getFontSize WRITE setFontSize RESET resetFontSize)
public:
    EditorInputCharactor(QWidget* parent, QPoint point, QString str, int position, QFont font, int delay = 0, int duration = 200);

    void setPosition(int x);
    int getPosition();
    void changePosition(int x);

    void updateRect(QPoint point);

    void setFontSize(int x);
    int getFontSize();
    void resetFontSize();

signals:
    void aniFinished(int position, EditorInputCharactor* charactor);

private:
    QPoint point;
    QString str;
    int position;
    int duration;
    QPropertyAnimation *ani;
    int origin_position;

    int font_size;
    int font_size_l;
};

#endif // EDITORINPUTCHARATOR_H
