#ifndef ANINLABEL_H
#define ANINLABEL_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class AniNLabel : public QWidget
{
    Q_OBJECT
public:
    explicit AniNLabel(QWidget *parent = nullptr);

    void setShowNum(int x);

protected:
    void paintEvent(QPaintEvent* e);

signals:

public slots:

private:
    int _num, _old;
    int _ani_new, _ani_old;
    int _fix_width;
};

#endif // ANINLABEL_H
