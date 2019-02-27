#ifndef APPLICATIONUPDATETHREAD_H
#define APPLICATIONUPDATETHREAD_H

#include <QObject>
#include <QThread>
#include <QProcess>
#include <QStringList>
#include <QDesktopServices>
#include "globalvar.h"
#include "filedownloadmanager.h"
#include "netutil.h"
#include "fileutil.h"
#include "stringutil.h"

class ApplicationUpdateThread : public QThread
{
    Q_OBJECT
public:
    ApplicationUpdateThread()
    {

    }

protected:
    void run()
    {
        // 下载的文件已经存在，则不进行检测
        if (isFileExist(rt->DOWNLOAD_PATH + "update.zip"))
        {
            rt->has_new_version = 2;
            return ;
        }

        // 判断更新的时间，最多一天更新一次
        qint64 latest_timestamp = us->getLongLong("runtime/check_update", 0);
        qint64 timestamp = getTimestamp();
        if (timestamp < latest_timestamp + 3600*1000*24) // 不足一天，24小时算一天
            return ;
        us->setVal("runtime/check_update", timestamp);

        // 保存更新记录的域名
        QString version_url = rt->DOMAIN_PATH + "/download/latest_version.txt";

        // 通过网络获取最新的版本号以及更新内容
        QString content = NetUtil::getHttpSource(version_url);
        //deb(content, "latest_version.txt");
        QString latest_version = getStrMid(content, "<version>", "</version>");
        QString update_content = getStrMid(content, "<update>", "</update>");
        QString current_version = VERSION_NUMBER;
        if (current_version.startsWith("v") || current_version.startsWith("V"))
            current_version = current_version.right(current_version.length()-1);
        if (latest_version.startsWith("v") || latest_version.startsWith("V"))
            latest_version = current_version.right(latest_version.length()-1);

        // 比较两个版本之间的内容
        if (latest_version.compare(current_version) > 0) // 有更新版本
        {
            qDebug() << "需要更新";

            // 是否打开下载地址
            QString browser_url = getStrMid(content, "<browser>", "</browser>");
            if (!browser_url.isEmpty())
            {
                QDesktopServices::openUrl(QUrl(browser_url));
                return ;
            }

            // 获取下载地址并进行下载
            QString download_url = getStrMid(content, "<download>", "</download>");
            if (download_url.isEmpty())
                download_url = rt->DOMAIN_PATH + "/download/WriterFly_v" + latest_version + ".zip";
            else if (!download_url.startsWith("http"))
                download_url = rt->DOMAIN_PATH + "/download/" + download_url;
            emit signalHasNewVersion(latest_version, download_url);
            rt->has_new_version = 1;
        }

    }

signals:
    void signalHasNewVersion(QString version, QString url);
    void signalDownloadStart();
    void signalDownloadFinish();

};

#endif // APPLICATIONUPDATETHREAD_H
