#ifndef DRAGSIZEWIDGET_H
#define DRAGSIZEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsEffect>
#include <QTimer>
#include "fileutil.h"
#include "globalvar.h"

class DragSizeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DragSizeWidget(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);

signals:
    void signalMoveEvent(int x, int y);

public slots:

private:
    int mx, my;
    bool is_pressing;
    QGraphicsOpacityEffect effect;
    QTimer timer;
};

#endif // DRAGSIZEWIDGET_H
