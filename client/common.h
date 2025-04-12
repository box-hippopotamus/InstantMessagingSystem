#pragma once

#include <QString>
#include <QIcon>
#include <QUuid>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QFileInfo>

namespace IM
{
    // 从文件路径提取文件名
    static inline QString getFileName(const QString& path)
    {
        QFileInfo fileInfo(path);
        return fileInfo.fileName();
    }

    // 日志宏
    #define TAG QString("[%1:%2]: ").arg(IM::getFileName(__FILE__), QString::number(__LINE__))
    #define LOG() qDebug().noquote() << TAG

    // 格式化时间戳
    static inline QString formatTime(int64_t timestamp)
    {
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
        return dateTime.toString("MM-dd HH:mm:ss");
    }

    // 获取当前时间
    static inline int64_t getTime()
    {
        return QDateTime::currentSecsSinceEpoch();
    }

    // QByteArray 转 QIcon
    static inline QIcon makeIcon(const QByteArray& byteArray)
    {
        QPixmap pixmap;
        pixmap.loadFromData(byteArray);
        QIcon icon(pixmap);
        return icon;
    }

    // 读取文件,获取 QByteArray
    static inline QByteArray readFileToByteArray(const QString& path)
    {
        QFile file(path);
        bool ok = file.open(QFile::ReadOnly);
        if (!ok)
        {
            LOG() << path << " 文件打开失败!";
            return QByteArray();
        }

        QByteArray content = file.readAll();
        file.close();
        return content;
    }

    // 写入文件
    static inline void writeByteArrayToFile(const QString& path, const QByteArray& content)
    {
        QFile file(path);
        bool ok = file.open(QFile::WriteOnly);
        if (!ok)
        {
            LOG() << path << " 文件打开失败!";
            return;
        }

        file.write(content);
        file.flush();
        file.close();
    }

    // 从指定文件中, 读取所有的二进制内容. 得到一个 QByteArray
    static inline QByteArray loadFileToByteArray(const QString& path)
    {
        QFile file(path);
        bool ok = file.open(QFile::ReadOnly);
        if (!ok)
        {
            LOG() << "文件打开失败!";
            return QByteArray();
        }

        QByteArray content = file.readAll();
        file.close();
        return content;
    }
}
