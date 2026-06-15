#ifndef CONNECTIONWORKER_H
#define CONNECTIONWORKER_H

#include <QObject>
#include <QString>
#include "networkconnecter.h"

class ConnectionWorker : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionWorker(QObject *parent = nullptr);

public slots:
    // 开始槽函数
    void StartWork(const QString &curlTemplate, int ICMPSeconds,bool showDetailedLog);
    // 停止槽函数
    void StopWork();


signals:
    // 日志信息
    void LogMessage(const QString &msg);

private:
    NetworkConnecter nc;
};

#endif // CONNECTIONWORKER_H
