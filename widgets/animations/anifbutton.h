#ifndef ANIBUTTON_H
#define ANIBUTTON_H

#include <QPropertyAnimation>
#include <QPushButton>
#include <QFontMetrics>

class AniFButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(int fontsize READ getFontSize WRITE setFontSize RESET resetFontSize)
public:
    AniFButton();
    AniFButton(QString caption);
    AniFButton(QString caption, QWidget *parent);

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

    void setFontSize(int x);
    int getFontSize();
    void resetFontSize();

    int getMiniHeight();

private:
    int font_size;
    int font_size_l;
};

#endif // ANIBUTTON_H
