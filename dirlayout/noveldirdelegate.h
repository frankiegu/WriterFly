#ifndef NOVELDIRDELEGATE_H
#define NOVELDIRDELEGATE_H

#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QModelIndex>
#include <QPainter>
#include <QWidget>
#include <QRect>
#include <QLineEdit>
#include <noveldirmodel.h>
#include <noveldiritem.h>
#include "defines.h"
#include "noveldirsettings.h"

class NovelDirDelegate : public QItemDelegate, public NovelDirSettings
{
    Q_OBJECT
public:
    NovelDirDelegate(QObject *parent = nullptr)
    {
        Q_UNUSED(parent);

    }

    QString getItemText(const QModelIndex &index) const
    {
        QString text = "", numstr = "";
        const QString& name = index.data(Qt::UserRole+DRole_CHP_NAME).toString();
        if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 卷
        {
            if (isShowRollNum() && index.data(Qt::UserRole+DRole_CHP_NUM).toInt() != 0)
            {
                int num = getRollStartNum()+index.data(Qt::UserRole+DRole_CHP_NUM).toInt() - 1;
                if (num > 0)
                {
                    if (isUseArab())
                    {
                        // numstr = QString("第")+index.data(Qt::UserRole+DRole_CHP_NUM).toString()+getRollChar()+QString(" ");
                        numstr = QString("第")+QString("%1").arg(num)+getRollChar()+QString(" ");
                    }
                    else
                    {
                        //numstr = QString("第")+index.data(Qt::UserRole+DRole_CHP_NUM_CN).toString()+getRollChar()+QString(" ");
                        numstr = QString("第")+ArabToCN(num)+getRollChar()+QString(" ");
                    }
                }

            }
            text = numstr + name;
        }
        else // 章
        {
            if (isShowChapterNum() && index.data(Qt::UserRole+DRole_CHP_NUM).toInt() != 0)
            {
                int num;
                if (isRecountNumByRoll())
                    num = getChapterStartNum()-1 + index.data(Qt::UserRole+DRole_CHP_CHPTINDEX).toInt();
                else
                    num = getChapterStartNum()-1 + index.data(Qt::UserRole+DRole_CHP_NUM).toInt();
                if (num > 0)
                {
                    if (isUseArab())
                    {
                        //numstr = QString("第")+index.data(Qt::UserRole+DRole_CHP_NUM).toString()+getChapterChar()+QString(" ");
                        numstr = QString("第")+QString("%1").arg(num)+getChapterChar()+QString(" ");
                    }
                    else
                    {
                        //numstr = QString("第")+index.data(Qt::UserRole+DRole_CHP_NUM_CN).toString()+getChapterChar()+QString(" ");
                        numstr = QString("第")+ArabToCN(num)+getChapterChar()+QString(" ");
                    }
                }
            }
            text = numstr + name;
        }
        return text;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        painter->save();

        // 获取图像绘制区域
        //QRect decorationRect = QRect(option.rect.topLeft(), QSize(80,80));

        // 获取文字
        QString text;
        int deviate = DIR_DEVIATE;
        int subs = -1;
        text = getItemText(index);
        if (index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()) // 是分卷，画数字
        {
            subs = index.data(Qt::UserRole+DRole_ROLL_SUBS).toInt();
        }
        else // 是章节
        {
            if (isNoRoll() && index.data(Qt::UserRole+DRole_CHP_NUM).toInt() > 0) // 不显示分卷 且 是正文，则取消缩进
                ;
            else
                deviate += DIR_DEVIATE_ADD; // 增加章节的缩进
        }

        // 获取文字区域并绘制
        QFontMetrics fm(painter->font());
        QRect display_rect = QRect(option.rect.topLeft()+QPoint(deviate,3),QSize(fm.width(text),fm.height()));
        drawBackground(painter, option, index);

        if (!index.data(Qt::UserRole+DRole_RC_ANIMATING).toBool()) // 不是在动画中（动画时不显示文字，即背景透明）
        {
            if (isNoRoll() && subs >= 0) // 不使用分卷，并且刚好是分卷，则使用灰色，弱化分卷存在感
            {
                painter->setPen(QColor(128, 128, 128));
            }
            painter->drawText(display_rect, text);

            // 画左边的小标记
            if (subs >= 0)
            {

            }

            // 画右边的小标记
            if (subs >= 0) // 是分卷（章节默认 -1）
            {
                if (subs == 0) // 没有章节，灰色的数字
                {
                    painter->setPen(QColor(128, 128, 128));
                }
                else // 章节，黑偏灰色的
                {
                    painter->setPen(QColor(64, 64, 64));
                }
                QString subs_text = QString("%1").arg(subs);
                QRect display_rect2 = QRect(option.rect.topRight()-QPoint(fm.width(subs_text)+10,-3),QSize(fm.width(subs_text),fm.height()));

                painter->drawText(display_rect2, subs_text);
            }
            else // 是章节，绘制打开状态
            {
                if (index.data(Qt::UserRole+DRole_CHP_OPENING).toBool()) // 打开状态中，画右边的小点
                {
                    QRect state_rect = QRect(option.rect.topRight()+QPoint(-14,6),QSize(4,4));
                    painter->setPen(QPen(QColor(us->primary_color), Qt::SolidLine));
                    painter->setBrush(QBrush(QColor(us->primary_color), Qt::SolidPattern));
                    painter->drawEllipse(state_rect);
                }
            }
        }

        // drawFocus(painter, option, displayRect); // 画选区虚线
        drawFocus(painter, option, QRect(0, 0, 0, 0));

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem o2(option);
        o2.rect.setHeight(option.rect.height());
        return QItemDelegate::sizeHint(o2, index);
    }

    /* 编辑框修改名字 */

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        QLineEdit* edit = new QLineEdit(parent);
        edit->setAcceptDrops(false);
        edit->setStyleSheet("selection-color:yello; selection-background-color:blue;");
        edit->setStyleSheet("border:none;");
        return edit;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString name = index.data(Qt::UserRole+DRole_CHP_NAME).toString();
        if (index.data(Qt::UserRole+DRole_CHP_STEMP).toString() != "")
            name = index.data(Qt::UserRole+DRole_CHP_STEMP).toString();
        QLineEdit* edit = static_cast<QLineEdit*>(editor);
//        renameEdit = edit;
        edit->setText(name);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QLineEdit* edit = static_cast<QLineEdit*>(editor);
        QString str = edit->text();
        //closeEditor(edit, QAbstractItemDelegate::NoHint);
        if (model->setData(index, str))
            model->setData(index, str, Qt::UserRole+DRole_CHP_STEMP);
        else
            model->setData(index, str, Qt::UserRole+DRole_CHP_STEMP);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const  QModelIndex &index) const
    {
        Q_UNUSED(index);
        int deviate = DIR_DEVIATE-2;
        if (!(index.data(Qt::UserRole+DRole_CHP_ISROLL).toBool()))
            deviate += DIR_DEVIATE_ADD;
        QRect rect = option.rect;
        rect.setLeft(rect.left()+deviate);
        editor->setGeometry(rect);
    }

    QLineEdit* getEditor()
    {
        return rename_edit;
    }

private :
    //NovelDirSettings dirSettings;
    QLineEdit* rename_edit;
};

#endif // MYDELEGATE_H
