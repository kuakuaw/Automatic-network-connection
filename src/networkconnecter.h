#ifndef NETWORKCONNECTER_H
#define NETWORKCONNECTER_H

#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <array>
#include <thread>
#include <chrono>
#include <windows.h>
#include <regex>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>

class NetworkConnecter
{
public:
    using logCalback = std::function<void(const std::string&)>;

    NetworkConnecter();
    // 联网检测
    bool checkConnection();
    // 执行登录
    void loginNetwork(logCalback logCb, const std::string& curlTemplate,bool showDetailedLog);
    // 开始工作
    int Working(logCalback logCb,const std::string& curlTemplate, int ICMPSeconds, bool showDetailedLog);
    // 停止
    void stop();

private:
    // 操作控制台
    std::string executeCommand(const char* cmd);
    // 获取本机IPv4地址
    std::string getLocalIP();

private:
    std::atomic<bool> isRunning{false};
    std::mutex mtx;
    std::condition_variable cv;


};

#endif // NETWORKCONNECTER_H
