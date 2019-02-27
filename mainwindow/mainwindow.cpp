#include "mainwindow.h"
MainWindow::MainWindow(QMainWindow *parent) :
    BaseWindow(parent)
{
    initWindow();
    initData();
    initFirstOpen();

    rt->is_initing = false;

    // 检查更新
    ApplicationUpdateThread* update_thread = new ApplicationUpdateThread();
    connect(update_thread, SIGNAL(signalHasNewVersion(QString,QString)), this, SLOT(slotHasNewVersion(QString,QString)));
    update_thread->start();
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isFileExist(rt->DOWNLOAD_PATH + "update.zip"))
    {
        qDebug() << "进行更新";
        QProcess process;
        process.startDetached("UpdateTool.exe");
    }

    return BaseWindow::closeEvent(event);
}

void MainWindow::initWindow()
{
    // 设置窗口图标
    setWindowIcon(QIcon(":/icons/appicon"));
    setWindowTitle(APPLICATION_NAME);
}

void MainWindow::initData()
{
    dir_layout->getListView()->initRecent();

    FrisoThread* friso_thread = new FrisoThread();
    friso_thread->start();

    lexicon_AI->setUS(us);
    LexiconsThread* lexicons_thread = new LexiconsThread();
    lexicons_thread->start();
}

void MainWindow::initFirstOpen()
{
    bool first = us->getBool("runtime/first_open", true);
    if (first) // 第一次打开
    {
        addLinkToDeskTop(QCoreApplication::applicationFilePath(), "WriterFly");

        us->setVal("runtime/first_open", false);
    }

}

void MainWindow::slotHasNewVersion(QString version, QString url)
{
    Q_UNUSED(version);
    QProcess process(this);
    QStringList list;
    list << url << "update.zip";
    process.startDetached("UpdateTool.exe", list);
}
