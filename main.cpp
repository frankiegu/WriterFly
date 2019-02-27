#include "mainwindow.h"
#include <QApplication>
#include "globalvar.h"

int main(int argc, char *argv[])
{
	// QApplication::setDesktopSettingsAware(true); // 据说是设置不同分辨率导致显示的字体大小不一致
    QApplication a(argc, argv);

    initGlobal();

    MainWindow w;
    w.show();

    return a.exec();
};
