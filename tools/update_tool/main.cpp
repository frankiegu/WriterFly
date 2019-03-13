
#include <QCoreApplication>
#include "downloadmanager.h"
#include <JlCompress.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    DownloadManager manager;
    QTimer::singleShot(0, &manager, SLOT(execute()));
    //manager.execute();

    app.exec();
}

