#ifndef NETUTIL_H
#define NETUTIL_H

#include <QObject>
#include <QThread>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QTextCodec>

class NetUtil
{
public:
    static QString getHttpSource(QString uri)
    {
        QUrl url(uri);
        QNetworkAccessManager manager;
        QEventLoop loop;
        //QTextCodec *codec;
        QNetworkReply *reply;

        reply = manager.get(QNetworkRequest(url));
        //请求结束并下载完成后，退出子事件循环
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        //开启子事件循环
        loop.exec();

        QString code_content = reply->readAll();
        //codec = QTextCodec::codecForHtml(code_content.toLatin1());
        //code_content = codec->toUnicode(code_content.toLatin1());

        return code_content;
    }
};

#endif // NETUTIL_H
