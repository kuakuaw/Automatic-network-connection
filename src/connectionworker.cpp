#include "connectionworker.h"

ConnectionWorker::ConnectionWorker(QObject *parent)
    : QObject{parent}
{


}



// 开始槽函数
void ConnectionWorker::StartWork(const QString &curlTemplate, int ICMPSeconds, bool showDetailedLog)
{

    // nc->Working([this](const std::string &cmsg){
    //     QString msg = QString::fromStdString(cmsg);
    //     emit ConnectionWorker::LogMessage(msg);
    // },curlTemplate.toStdString(),ICMPSeconds);

    NetworkConnecter::logCalback callback = [this](const std::string& cmsg)
    {
        QString msg = QString::fromStdString(cmsg);
        emit LogMessage(msg);
    };
    auto curlMsg = curlTemplate.toStdString();

    nc.Working(callback,curlMsg,ICMPSeconds,showDetailedLog);


}


// 停止槽函数
void ConnectionWorker::StopWork()
{
    nc.stop();
}
