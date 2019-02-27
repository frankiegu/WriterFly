#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <QString>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

class FileUtil
{
public:
    static void copyWithOverride(QString old_folder, QString new_folder)
    {
        qDebug() << "traverse folder " << old_folder << " to " << new_folder;
        if (!old_folder.endsWith("/")) old_folder += "/";
        if (!new_folder.endsWith("/")) new_folder += "/";

        QDir dir(old_folder);
        //dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks);
        int count = (int)dir.count();
        qDebug() << "folder files count: " << count;
        for (int i = 0; i < count; i++)
        {
            QString filename = dir[i];
            if (filename == "." || filename == "..") continue;
            filename = old_folder + filename;
            QFileInfo info(filename);
            if (info.isFile())
            {
                QFile old_file(filename);
                QString new_path = new_folder + info.fileName();
                qDebug() << "filename:" << info.fileName();
                QFile new_file(new_path);
                if (new_file.exists())
                    new_file.remove();
                old_file.rename(new_path);
                qDebug() << "move "  << filename << " to " << new_path;
            }
            else
            {
                QDir new_dir(new_folder + info.fileName());
                if (!new_dir.exists())
                    new_dir.mkdir(new_folder + info.fileName());
                copyWithOverride(filename, new_folder + info.fileName());
                new_dir.remove(new_folder + info.fileName());
                //qDebug() << "remove:" << new_folder + info.fileName();
            }
        }

    }

    static bool DeleteDir(const QString &path)
    {
        if (path.isEmpty())
        {
            return false;
        }

        QDir dir(path);
        if(!dir.exists())
        {
            return true;
        }

        dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        QFileInfoList fileList = dir.entryInfoList();
        foreach (QFileInfo fi, fileList)
        {
            if (fi.isFile())
            {
                fi.dir().remove(fi.fileName());
            }
            else
            {
                DeleteDir(fi.absoluteFilePath());
            }
        }
        return dir.rmpath(dir.absolutePath());
    }
};

#endif // FILEUTIL_H
