#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H


#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <stdio.h>
#include <JlCompress.h>
#include <QProcess>
#include <QTime>
#include "fileutil.h"

QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class DownloadManager: public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager;
    QList<QNetworkReply *> currentDownloads;

public:
    DownloadManager();
    void doDownload(const QUrl &url);
    QString saveFileName(const QUrl &url);
    bool saveToDisk(const QString &filepath, QIODevice *data);
    bool checkDownloadedFiles();
    bool checkNewVersion();

public slots:
    void execute();
    void downloadFinished(QNetworkReply *reply);
    void downloadProcess(qint64 current, qint64 total);
    void sslErrors(const QList<QSslError> &errors);

private:
    QString saved_file_name;
    QString download_folder;
};

#endif // DOWNLOADMANAGER_H
