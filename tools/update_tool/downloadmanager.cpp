#include "downloadmanager.h"

bool CheckAppRunningStatus(const QString &appName)
{
#ifdef Q_OS_WIN
    QProcess* process = new QProcess;
    process->start("tasklist" ,QStringList()<<"/FI"<<"imagename eq " +appName);
    process->waitForFinished();
    QString outputStr = QString::fromLocal8Bit(process->readAllStandardOutput());
    return (outputStr.contains(appName));
#endif
}

void sleep(unsigned int msec)
{
    QTime reachTime = QTime::currentTime().addMSecs(msec);
    while ( QTime::currentTime() < reachTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}


DownloadManager::DownloadManager()
{
    download_folder = QCoreApplication::applicationDirPath() + "/download/";
    QDir dir(download_folder);
    if (!dir.exists()) {
        printf("create dir %s\n", qPrintable(download_folder));
        dir.mkpath(download_folder);
    }

    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(downloadFinished(QNetworkReply*)));

}

void DownloadManager::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
    //connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT( downloadProgress(qint64 ,qint64 )));
#ifndef QT_NO_SSL
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
#endif

    currentDownloads.append(reply);
    printf("start download, please wait a mintue\n");
}

QString DownloadManager::saveFileName(const QUrl &url)
{
    if (!saved_file_name.isEmpty())
        return saved_file_name;

    QString path = url.path();
    QString basename = QFileInfo(path).fileName();
    if (basename.isEmpty())
        basename = "download";

    /*if (QFile::exists(basename)) {
        // already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }*/

    return basename;
}

bool DownloadManager::saveToDisk(const QString &filepath, QIODevice *data)
{
    QFile file(filepath);

    if (!file.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "can't open file %s to write: %s\n",
                qPrintable(filepath),
                qPrintable(file.errorString()));
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

bool DownloadManager::checkDownloadedFiles()
{
    FileUtil::DeleteDir(download_folder + "unzip_temp");

    QFile zip_file(download_folder + "update.zip");
    if (!zip_file.exists()) {
        printf("no update.zip exist\n");
        return false;
    }
    printf("find update.zip, unzip to update writerfly\n");

    int times = 0;
    while (CheckAppRunningStatus("WriterFly.cn"))
    {
        printf("wait WriterFly to exit");
        sleep(5000);
        if (++times > 10)
            return false;
    }

    QString unzip_dir = download_folder + "/unzip_temp";
    QDir dir(unzip_dir);
    if (!dir.exists()) {
        dir.mkdir(unzip_dir);
    }

    JlCompress::extractDir(download_folder+"update.zip", unzip_dir);

    QString exe_dir = unzip_dir;
    {
        QFile exe_file(exe_dir + "/WriterFly.exe");
        if (!exe_file.exists()) {
            QDir dir(unzip_dir);
            dir.setFilter(QDir::Dirs);
            int count = dir.count();
            for (int i = 0; i < count; i++)
            {
                QString filename = dir[i];
                printf("====%s\n", qPrintable(filename));
                QString exe_dir2 = unzip_dir + "/" + filename;
                QFile exe_file2(exe_dir + "/WriterFly.exe");
                if (exe_file2.exists())
                {
                    exe_dir = exe_dir2;
                    break;
                }
            }
        }
    }
    printf("files dir: %s\n", qPrintable(exe_dir));

    FileUtil::copyWithOverride(exe_dir, QCoreApplication::applicationDirPath());

    dir.remove(unzip_dir);
    FileUtil::DeleteDir(unzip_dir);
    zip_file.remove();
    QDir download_dir(download_folder);
    download_dir.remove(download_folder); // 如果是空文件夹的话，则可以进行移除。
    printf("delete downloaded files\n");

    qDebug() << "《写作天下》升级完毕";
    qDebug() << "《写作天下》升级完毕";
    qDebug() << "《写作天下》升级完毕";
    qDebug() << "（重要的事情说三遍）";

    return true;
}

bool DownloadManager::checkNewVersion()
{
    QString str;
    QString download_path;
    //printf("没有做手动检查更新的功能。这点儿小事，自动即可，就不麻烦大家啦~\n");
    //printf("Ru Guo Shang Mian Zhe Ju Luan Ma, Qing Kan Qia Mian De Fan Yi:\n");
    printf("Didn't add manual check update function. It should be automatical, without bothering everybody ~\n");
    return false;
}

void DownloadManager::execute()
{
    QStringList args = QCoreApplication::instance()->arguments();
    args.takeFirst();           // skip the first argument, which is the program's name
    if (args.isEmpty()) {
        printf("no cmd param, check downloaded files\n");
        if (!checkDownloadedFiles())
        {
            printf("start to check new version on http://writerfly.cn/\n");
            checkNewVersion();
        };
        QCoreApplication::instance()->quit();
        return;
    }

    saved_file_name = args.size() > 1 ? args.at(1) : "";
    if (!saved_file_name.isEmpty()) {
        QFile file(download_folder + saved_file_name);
        if (file.exists()) {
            printf("remove old file\n");
            file.remove();
        }
        file.close();
    }

    QString arg = args.at(0);
    QUrl url = QUrl::fromEncoded(arg.toLocal8Bit());
    doDownload(url);
}

void DownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_SSL
    foreach (const QSslError &error, sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

void DownloadManager::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    if (reply->error()) {
        fprintf(stderr, "download %s fail: %s\n",
                url.toEncoded().constData(),
                qPrintable(reply->errorString()));
    } else {
        QString filename = saveFileName(url);
        QString filepath = download_folder + filename;
        if (saveToDisk(filepath, reply))
        {
            printf("download %s success (save to %s)\n",
                   url.toEncoded().constData(), qPrintable(filename));
            printf("Auto update after <WriterFly> closed\n");
        }
    }

    currentDownloads.removeAll(reply);
    reply->deleteLater();

    //checkDownloadedFiles();

    if (currentDownloads.isEmpty())
        // all downloads finished
        QCoreApplication::instance()->quit();
}

void DownloadManager::downloadProcess(qint64 current, qint64 total)
{
    printf("download process : %d / %d Bytes\n", (int)current, (int)total);
}
