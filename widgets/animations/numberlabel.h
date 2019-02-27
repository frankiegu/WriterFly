#ifndef NUMBERLABEL_H
#define NUMBERLABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>

class NumberLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int animation_progress READ getAnimationProgress WRITE setAnimationProgress RESET resetAnimationProgress)
public:
    NumberLabel();
    NumberLabel(QWidget* parent);

    void setShowNum(int x);

private:
    void setAnimationProgress(int x);
    int getAnimationProgress();
    void resetAnimationProgress();

private:
    int show_num;
    int animation_progress;
    int dn;
};

#endif // NUMBERLABEL_H
