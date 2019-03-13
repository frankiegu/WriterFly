#include "mainwindow.h"
MainWindow::MainWindow(QMainWindow *parent) :
    BaseWindow(parent)
{
    initWindow();
    initFirstOpen();
    initData();

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

        refreshAppPath(); // 刷新软件运行位置

        us->setVal("runtime/first_open", false);
    }
    else
    {
        QString app_path = us->getStr("runtime/APP_PATH");
        if (app_path != rt->APP_PATH) // 文件路径变了
        {
            refreshAppPath();
        }
    }
}

void MainWindow::refreshAppPath()
{
    qDebug() << "已修改文件路径" << us->getStr("runtime/APP_PATH") << " >> " << rt->APP_PATH;
    us->setVal("runtime/APP_PATH", rt->APP_PATH);

    // 修改 Friso 的属性
    QString friso_ini = readTextFile(rt->APP_PATH+"tools/friso/friso.ini");
    if (!friso_ini.isEmpty())
    {
        int start_pos = friso_ini.indexOf("friso.lex_dir");
        if (start_pos == -1)
            return ;
        start_pos += QString("friso.lex_dir").length();
        while (start_pos < friso_ini.length() && (friso_ini.mid(start_pos, 1) == " " || friso_ini.mid(start_pos, 1) == "="))
            start_pos++;
        int end_pos = friso_ini.indexOf("\n", start_pos);
        if (end_pos == -1)
            end_pos = friso_ini.length();
        end_pos = friso_ini.lastIndexOf("tools/friso/dict/UTF-8/", end_pos);
        if (end_pos == -1)
            return ;
        QString friso_path = rt->APP_PATH;
        friso_ini = friso_ini.left(start_pos) + friso_path + friso_ini.right(friso_ini.length() - end_pos);
        writeTextFile(rt->APP_PATH+"tools/friso/friso.ini", friso_ini, "UTF-8");
    }
    else
    {
        qDebug() << "无法读取Friso分词配置文件【" + rt->APP_PATH+"tools/friso/frisi.ini" + "】";
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
