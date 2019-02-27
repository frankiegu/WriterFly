#ifndef CUSTOMTITLEBAR_H
#define CUSTOMTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include "globalvar.h"
#include "anivlabel.h"

class MyTitleBar : public QWidget
{
        Q_OBJECT
public:
        MyTitleBar(QWidget* parent);
        ~MyTitleBar();

        void setBackgroundColor(int r, int g, int b, bool isTransparent = false); // 设置标题栏背景色及是否设置标题栏背景色透明
        void setTitleIcon(QString filePath, QSize IconSize = QSize(25, 25));      // 设置标题栏图标
        void setTitleContent(QString titleContent, int titleFontSize = 9);        // 设置标题内容
        void setTitleContent2(QString titleContent, int titleFontSize = 9);       // 设置标题内容
        void setTitleWidth(int width);                                            // 设置标题栏宽度

        void saveRestoreInfo(const QPoint point, const QSize size); // 保存/获取 最大化前窗口的位置和大小
        void getRestoreInfo(QPoint& point, QSize& size);
        void setRestoreInfo(QPoint point, QSize size);
        bool isMaxxing();
        bool simulateMax();

        QRect getBtnRect();

private:
        void paintEvent(QPaintEvent* event);
        void mouseDoubleClickEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);

        void initControl();                            // 初始化控件
        void initConnections();                        // 信号槽的绑定
        void initStyle();                              // 编辑样式
        void loadStyleSheet(const QString& sheetName); // 加载样式文件

signals:
        // 按钮触发的信号
        void signalButtonMinClicked();
        void signalButtonRestoreClicked();
        void signalButtonMaxClicked();
        void signalButtonCloseClicked();
        void signalButtonSidebarClicked();
        void signalButtonSettingsClicked();
        // 位置变化的信号
        void windowChanged(QPoint point, QSize size);
        void changeFinished(QPoint point, QSize size);

private slots:
        // 按钮触发的槽
        void onButtonMinClicked();
        void onButtonRestoreClicked();
        void onButtonMaxClicked();
        void onButtonCloseClicked();
        void onButtonSidebarClicked();
        void onButtonSettingsClicked();
        void onRollTitle();

private:
        QLabel* titlebar_icon;                  // 标题栏图标
        AniVLabel* title_content_widget;         // 标题栏内容
        QPushButton* min_btn;        // 最小化按钮
        QPushButton* restore_btn;    // 最大化还原按钮
        QPushButton* max_btn;        // 最大化按钮
        QPushButton* close_btn;      // 关闭按钮
        QPushButton* sidebar_btn;    // 侧边栏按钮
        QPushButton* settings_btn;   // 设置按钮

        int color_R, color_G, color_B; // 标题栏背景色

        QPoint restore_pos;              // 最大化，最小化变量
        QSize restore_size;

        bool is_moved;                     // 是否被移动
        bool is_pressed;                 // 移动窗口的变量
        QPoint start_move_pos;

        QString title_content;           // 标题栏内容
        bool is_transparent;             // 标题栏是否透明

        QRect button_rect;                  // 按钮坐标
};

#endif // CUSTOMTITLEBAR_H
