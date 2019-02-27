#include "editorinputcharactor.h"

EditorInputCharactor::EditorInputCharactor(QWidget* parent, QPoint point, QString str, int position, QFont font, int delay, int duration)
    : QLabel(str, parent), point(point), str(str), position(position), duration(duration)
{
    setFont(font);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    origin_position = position;

    this->show(); // 默认 hide，但是必须 show，不然不会自动伸缩宽度……（操蛋）
    this->hide();
    move(point);

    /*// 几何变换动画
    ani = new QPropertyAnimation(this, "geometry");
    QRect rect(point, this->size());
    QRect start_rect(rect.left()+rect.width()/2-1, rect.top()+rect.height()/2-1, 2, 2);
    ani->setStartValue(start_rect);
    ani->setEndValue(rect);
    ani->setDuration(duration);*/


    // 字体大小改变动画
    QSize size = this->size();
    setFixedSize(size);
    setAlignment(Qt::AlignCenter);
    font_size = 1;
    font_size_l = font.pointSize();
    ani = new QPropertyAnimation(this, "fontsize");
    ani->setDuration(duration);
    ani->setStartValue(font_size);
    ani->setEndValue(font_size_l);
    ani->setEasingCurve(QEasingCurve::InOutQuad);

    connect(ani, &QPropertyAnimation::finished, [=]{
        emit aniFinished(this->position, this);
        this->deleteLater(); // close
    });

    if (delay == 0)
    {
        this->show();
        ani->start();
    }
    else
    {
        QTimer::singleShot(delay, [=]{
            this->show();
            ani->start();
        });
    }
}

void EditorInputCharactor::updateRect(QPoint point)
{
    this->move(point);
}

void EditorInputCharactor::setPosition(int x)
{
    position = -1;
}

int EditorInputCharactor::getPosition()
{
    return position;
}

void EditorInputCharactor::changePosition(int x)
{
    if (position == -1) return ;
    position += x;
}

void EditorInputCharactor::setFontSize(int x)
{
    QFont font = this->font();
    font.setPointSize(x);
    this->setFont(font);
}

int EditorInputCharactor::getFontSize()
{
    QFont font = this->font();
    return font.pointSize();
}

void EditorInputCharactor::resetFontSize()
{
    setFontSize(font_size);
}
