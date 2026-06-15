#include "networkconnecter.h"

NetworkConnecter::NetworkConnecter()
{
}

// std::string NetworkConnecter::executeCommand(const char *cmd)
// {
//     std::array<char, 128> buffer;
//     std::string result;

//     std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);

//     if (!pipe)
//     {
//         return "Error: 命令执行失败";
//     }

//     while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
//     {
//         result += buffer.data();
//     }
//     return result;
// }

std::string NetworkConnecter::executeCommand(const char *cmd)
{
    std::string result;
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    //创建管道，用于截获cmd的输出
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        return "[错误] 管道创建失败!";
    }
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    si.hStdError = hWrite;   //将错误输出重定向到管道
    si.hStdOutput = hWrite;  //将标准输出重定向到管道
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; //隐藏窗口
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    //_popen 默认使用 cmd.exe /c，手动拼接
    std::string fullCmd = "cmd.exe /c " + std::string(cmd);
    //创建CREATE_NO_WINDOW标志
    if (!CreateProcessA(NULL, &fullCmd[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return "[错误] 管道进程创建失败!";
    }
    //关闭写入端句柄，取消ReadFile死锁等待
    CloseHandle(hWrite);
    //读取管道中的数据
    char buffer[128];
    DWORD bytesRead;
    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }
    //清理
    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return result;
}

bool NetworkConnecter::checkConnection()
{
    std::string output = executeCommand("ping -n 1 baidu.com");

    if (output.find("TTL=") != std::string::npos || output.find("ttl=") != std::string::npos)
    {
        return true;
    }
    return false;
}

std::string NetworkConnecter::getLocalIP()
{
    std::string output = executeCommand("ipconfig");

    const static std::regex reg(R"(IPv4.*?:\s*(\d+\.\d+\.\d+\.\d+))");
    std::smatch match;

    std::string ip = "";
    if (std::regex_search(output, match, reg))
    {
        ip = match[1];
    }
    return ip;

    // std::string output = executeCommand("ipconfig");

    // size_t ipv4Pos = output.find("IPv4");
    // if(ipv4Pos == std::string::npos) return "";

    // size_t colonPos = output.find(":",ipv4Pos);
    // if(colonPos == std::string::npos) return "";

    // size_t endPos = output.find("\n", colonPos);
    // if (endPos == std::string::npos) return "";

    // std::string ip = output.substr(colonPos+1, endPos-colonPos-1);

    // ip.erase(0,ip.find_first_not_of(" \r\n\t"));
    // ip.erase(ip.find_last_not_of(" \r\n\t")+1);

    // return ip;
}

int NetworkConnecter::Working(logCalback logCb, const std::string &curlTemplate, int ICMPSeconds,bool showDetailedLog)
{
    isRunning.store(true);
    logCb("网络检测ICMP已启动...");
    while (isRunning)
    {
        if (!checkConnection())
        {
            loginNetwork(logCb, curlTemplate,showDetailedLog);
        }
        else
        {
            logCb("网络状态良好,无需操作.");
        }

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::seconds(ICMPSeconds), [this]()
                        { return !isRunning.load(); });
        }
    }

    logCb("ICMP模块退出.");
    return 0;
}

void NetworkConnecter::loginNetwork(logCalback logCb, const std::string &curlTrmplate,bool showDetailedLog)
{
    logCb("正在准备登录...");

    // 获取本机IP
    std::string currentIP = getLocalIP();
    if (currentIP.empty())
    {
        logCb("[错误] 无法获取本机 IP 地址，请检查网络适配器！");
        return;
    }

    logCb("成功抓取当前本机 IP: " + currentIP);

    // auto ipPos = curlTrmplate.find("user_ip=");
    // if (ipPos == std::string::npos)
    // {
    //     logCb("无法匹配curl格式，请检查curl格式!");
    //     return;
    // }

    // auto andPos = curlTrmplate.find("&",ipPos);
    // if (andPos == std::string::npos)
    // {
    //     logCb("无法匹配curl格式，请检查curl格式!");
    //     return;
    // }
    // auto curl = curlTrmplate;
    // curl.replace(ipPos+8,andPos-(ipPos+8),currentIP);

    const static std::regex curlReg(R"(user_ip=[^&]*)");
    std::string curlCommand = std::regex_replace(curlTrmplate, curlReg, "user_ip=" + currentIP);
    if (curlCommand == curlTrmplate)
    {
        logCb("无法匹配curl格式,请检查curl格式!");
        return;
    }

    // if (curlCommand.find("curl ") != std::string::npos && curlCommand.find("curl -s") != std::string::npos)
    // {
    //     curlCommand = "curl -s " + curlCommand;
    // }
    {
        auto curls = curlCommand.find("curl -s");
        if (curls == std::string::npos)
        {
            auto curl = curlCommand.find("curl ");
            if ( curl != std::string::npos)
            {
                curlCommand.replace(0,curl+5,"curl -s ");
            }
            else
            {
                curlCommand = "curl -s " + curlCommand;
            }
        }
    }

    logCb("正在发送登录请求...");
    std::string response = executeCommand(curlCommand.c_str());

    // 返回判断
    if (showDetailedLog)
    {
    logCb(">>服务器返回:" + response); // 测试返回
    }

    if (response.find(R"("result":"1")") != std::string::npos || response.find("认证成功") != std::string::npos)
    {
        logCb("[成功] 网络认证通过!");
    }
    else if (response.find(R"("ret_code":"2")") != std::string::npos)
    {
        logCb("[提示] 服务器返回 ret_code:2,正在确认是否已在线...");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (checkConnection())
        {
            logCb("[成功] 网络ping通过,账户在线!");
        }
        else
        {
            logCb("[失败] 联通测试失败,请检查机器!");
        }
    }
    else if (response.find(R"("result":"0")") != std::string::npos)
    {
        logCb("[失败] 网页拒绝了登录请求!");
    }
    else
    {
        logCb("[提示] 返回格式未知,正在直接确认网络连通性...");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (checkConnection())
        {
            logCb("[成功] 网络ping通过,账户在线!");
        }
        else
        {
            logCb("[失败] 网络依然未连接.");
        }
    }
}

void NetworkConnecter::stop()
{
    isRunning.store(false);
    cv.notify_all();
}