#ifndef TRANSPARENTCONTAINER_H
#define TRANSPARENTCONTAINER_H

#include <QObject>
#include <QWidget>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QPalette>
#include "defines.h"
#include "globalvar.h"

#define MAX_BLUR_RADIUS 8

class TransparentContainer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int blur_radius READ getBlurRadius WRITE setBlurRadius RESET resetBlurRadius)
public:
    explicit TransparentContainer(QWidget *parent = nullptr);

    void toShow();
    void toHide();
    void setShowType(int t);

protected :
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent *event);

signals:
    void signalToClose(int kind);
    void signalClosed(int kind);

public slots:
    void slotAniFinished();
    void slotEsc();

public :
    int getBlurRadius();
    void setBlurRadius(int x);
    void resetBlurRadius();

protected:
    bool showing;
    bool animating;
    int show_type;
    QPixmap* bgmap;
    int blur_radius;
    QGraphicsBlurEffect* blur_effect;
};

#endif // TRANSPARENTCONTAINER_H
