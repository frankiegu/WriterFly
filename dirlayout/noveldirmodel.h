#ifndef NOVELDIRMODEL_H
#define NOVELDIRMODEL_H

#include <QAbstractItemModel>
#include <QWidget>
#include <QTimer>
#include "stringutil.h"
#include "noveldirmdata.h"
#include "globalvar.h"

class NovelDirModel : public QAbstractListModel, public NovelDirMData
{
	Q_OBJECT
public:
    NovelDirModel(const QList<NovelDirItem> &list, QObject *parent = nullptr)
    {
        Q_UNUSED(parent);
        cr_list = list;
        //connect(this, SIGNAL(listChanged()), this, SLOT(refrushModel()));
    }

    NovelDirModel(QObject *parent = nullptr)
    {
        Q_UNUSED(parent);
        //connect(this, SIGNAL(listChanged()), this, SLOT(refrushModel()));
    }

    NovelDirModel(QString novel_name, QObject *parent = nullptr) : NovelDirMData(novel_name)
    {
        Q_UNUSED(parent);
        //connect(this, SIGNAL(listChanged()), this, SLOT(refrushModel()));
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return cr_list.count();
    }

    int getIndexByName(QString name)
    {
        for (int i = 0; i < cr_list.size(); i++)
            if (name == cr_list[i].getName())
                return i;
        return -1;
    }

    QModelIndex getModelIndexByName(QString name)
    {
        for (int i = 0; i < cr_list.size(); i++)
            if (name == cr_list[i].getName())
                return index(i, 0);
        return index(0, 0);
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        // 不需要 index 的 data
        if (role == Qt::UserRole+DRole_ROLL_COUNT)
        {
            return roll_subs.size();
        }
        else if (role == Qt::UserRole+DRole_CR_COUNT)
        {
            return cr_list.size();
        }
        else if (role == Qt::UserRole+DRole_ROLL_NAMES)
        {
            return roll_names;
        }

        // 判断 index
        if ((!index.isValid()) || (index.row() >= cr_list.size()))
            return QVariant();
        NovelDirItem item = cr_list.at(index.row());

        if (role == Qt::UserRole+DRole_CHP_NAME)
        {
            return item.getName(); // 章节名
        }
        else if (role == Qt::UserRole+DRole_CHP_NUM)
        {
            return item.getNumber(); // 序号
        }
        else if (role == Qt::UserRole+DRole_CHP_NUM_CN)
        {
            return item.getNumber_cn(); // 序号
        }
        else if (role == Qt::UserRole+DRole_CHP_ROLLINDEX)
        {
            return item.getRollIndex(); // 序号
        }
        else if (role == Qt::UserRole+DRole_CHP_CHPTINDEX)
        {
            return item.getChptIndex(); // 序号
        }
        else if (role == Qt::UserRole+DRole_CHP_DETAIL)
        {
            return item.getDetail(); // 细纲
        }
        else if (role == Qt::UserRole+DRole_CHP_OPENING)
        {
            return item.isOpening(); // 是否编辑中
        }
        else if (role == Qt::UserRole+DRole_CHP_ISROLL)
        {
            return item.isRoll(); // 是否为分卷
        }
        else if (role == Qt::UserRole+DRole_CHP_ISHIDE)
        {
            return item.isHide(); // 是否隐藏
        }
        else if (role == Qt::UserRole+DRole_CHP_STEMP)
        {
            return item.getS_temp(); // 双击编辑，出错后恢复原来的文本
        }
        else if (role == Qt::UserRole+DRole_ROLL_SUBS)
        {
            int rIndex = item.getRollIndex();
            return roll_subs[rIndex]; // 分卷章数量
        }
        else if (role == Qt::UserRole+DRole_ROLL_COUNT)
        {
            return roll_subs.size();
        }
        else if (role == Qt::UserRole+DRole_CR_COUNT)
        {
            return cr_list.size();
        }
        else if (role == Qt::UserRole+DRole_CHP_FULLNAME)
        {
             return item.getFullChapterName();
        }
        else if (role == Qt::UserRole+DRole_RC_ANIMATING)
        {
             return item.isAnimating();
        }
        else if (role == Qt::UserRole+DRole_ROLL_NAMES)
        {
             return roll_names;
        }


        return QVariant();
    }

    // 重写 flags 和 setData 使 Model 可双击编辑
    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        //if (index.row() > 0 && us->one_click) // 作品相关卷名不允许更改
            //flags |= Qt::ItemIsEditable;
        if (index.row() > 0)
            flags |= Qt::ItemIsEditable;
        return flags;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (!index.isValid()) return false;
        NovelDirItem item = cr_list.at(index.row());
        if (role == Qt::EditRole)
        {
            QString old_name = item.getName();
            bool isNew = false;
            if (canRegExp(old_name, "新[章卷]\\d+")) // 分卷是聚集焦点用
                isNew = true;
            bool rst = tryChangeName(index.row(), value.toString());
            if (isNew && rst)
            {
                emit signalOpenChapter(index.row());
            }
            return rst;
        }
        else if (role == Qt::UserRole+DRole_CHP_STEMP)
        {
            cr_list[index.row()].setS_temp(value.toString());
        }
        else if (role == Qt::UserRole+DRole_CHP_FULLNAME)
        {
            cr_list[index.row()].setFullChapterName(value.toString());
            deb(value.toString(), "model.setFullChapterName");
        }
        return true;
    }

    NovelDirItem* getItem(int index)
    {
        return &cr_list[index];
    }

    void listDataChanged()
    {
        beginResetModel();
        endResetModel();

        emit signalListDataChange(); // 发送给 QListView 的修改信号，用以调用 恢复隐藏item 的函数
    }

    void gotoReedit()
    {
        emit signalReedit();
    }

    void changeNameSuccessed(bool isChapter, QString novelName, QString oldName, QString newName)
    {
        emit signalChangeName(isChapter, novelName, oldName, newName);
    }

    void deleteChapterSuccessed(QString novelName, QString chapterName)
    {
        emit signalDeleteChapter(novelName, chapterName);
    }

signals:
    void signalListDataChange();
    void signalReedit();
    void signalOpenCurrentChapter();
    void signalOpenChapter(int index);
    void signalChangeName(bool is_chapter, QString novel_name, QString old_name, QString new_name);
    void signalDeleteChapter(QString novel_name, QString chapter_name);

};

#endif // NOVELDIRMODEL_H
