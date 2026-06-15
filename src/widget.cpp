#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 美化UI，设置背景和透明度
    this->setWindowOpacity(0.95); // 设置整体透明度
    this->setAttribute(Qt::WA_StyledBackground, true); // 允许自定义QWidget使用QSS绘制背景
    this->setStyleSheet(
        "QWidget#Widget {"
        "   border-image: url(:/background/bg0.jpg);" // 设置主背景
        "}"
        "QLabel {"
        "   color: #2c3e50;"
        "   font-weight: bold;"
        "}"
        "QLineEdit, QTextEdit, QTimeEdit {"
        "   background-color: rgba(255, 255, 255, 0.6);"
        "   border: 1px solid rgba(255, 255, 255, 0.8);"
        "   border-radius: 5px;"
        "   padding: 2px;"
        "   color: #2c3e50;"
        "   font-weight: bold;"
        "}"
        "QPushButton {"
        "   background-color: rgba(255, 255, 255, 0.7);"
        "   border: 1px solid rgba(255, 255, 255, 0.9);"
        "   color: #2c3e50;"
        "   border-radius: 5px;"
        "   padding: 5px 10px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255, 255, 255, 0.9);"
        "}"
        "QCheckBox {"
        "   color: #2c3e50;"
        "   font-weight: bold;"
        "}"
    );

    // 读取自启状态
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (reg.contains("AutoNetworkConnection")) {
        ui->autoStartCheckBox->setChecked(true); //如果注册表里有，勾上
    }

    //导入QSettings设置（记住输入设置）
    QSettings settings(configPath,QSettings::IniFormat);

    auto userName = settings.value("zLineEdit","").toString();
    auto userPassword =  settings.value("mLineEdit","").toString();
    auto userTimeset = settings.value("timeEdit","").toTime();
    QString userCurl = settings.value("curlEdit","").toString();

    ui->zLineEdit->setText(userName);
    ui->mLineEdit->setText(userPassword);
    ui->timeEdit->setTime(userTimeset);
    ui->curlEdit->setText(userCurl);

    // 解析程序启动时的参数
    if (QCoreApplication::arguments().contains("-autostart")) {
        // 如果包含自启参数，我们延迟 0 毫秒（也就是等主界面事件循环加载完毕后）自动点击“开始运行”按钮
        QTimer::singleShot(0, this, [this]() {
            ui->startButton->click();
        });
    }
}

Widget::~Widget()
{
    // 关闭的时候保存输入
    QSettings settings(configPath,QSettings::IniFormat);

    settings.setValue("zLineEdit",ui->zLineEdit->text());
    settings.setValue("mLineEdit",ui->mLineEdit->text());
    settings.setValue("timeEdit",ui->timeEdit->text());
    settings.setValue("curlEdit",ui->curlEdit->toPlainText());
    delete ui;
}

void Widget::on_startButton_clicked()
{
    if(!log)
    {
        log = std::make_unique<Log>();
        log->setWindowFlag(Qt::Window);
        connect(log.get(),&Log::stopRequest,this,&Widget::stopLogInfo);
    }
    log->clearLog();
    this->hide();
    log->show();

    // 启动线程
    if(!workThread) {
    workThread = new QThread(this);
    worker = std::make_unique<ConnectionWorker>();
    worker->moveToThread(workThread);
    connect(this,&Widget::opreateWork,worker.get(),&ConnectionWorker::StartWork);
    connect(worker.get(),&ConnectionWorker::LogMessage,log.get(),&Log::appendLog);
    connect(log.get(),&Log::stopRequest,worker.get(),&ConnectionWorker::StopWork,Qt::DirectConnection);
    workThread->start();
    }
    // 获取数据
    QString userZ = ui->zLineEdit->text();
    QString userM = ui->mLineEdit->text();
    QString userCurl = ui->curlEdit->toPlainText();
    QTime userT = ui->timeEdit->time();
    // QTime baseTime(0,0,0);
    // int timemsg =baseTime.secsTo(userT);
    int timemsg = userT.hour() * 3600 + userT.minute() * 60 + userT.second();

    const static QRegularExpression userZReg(R"(user_account=[^&]+)");
    const static QRegularExpression userMReg(R"(user_password=[^&]+)");
    userCurl.replace(userZReg,"user_account="+userZ);
    userCurl.replace(userMReg,"user_password="+userM);

    bool showDetailedLog = ui->detailedLogCheckBox->isChecked();
    // 发送
    emit opreateWork(userCurl, timemsg,showDetailedLog);

}

void Widget::stopLogInfo()
{
    if(log)
    {
        log->hide();
    }
    this->show();
}

void Widget::on_parseButton_clicked()
{
    auto curl = ui->curlEdit->toPlainText();
    // 正则表达式匹配输入内容
    const static QRegularExpression nameRule("user_account=([^&]+)");
    const static QRegularExpression passwordRule("user_password=([^&]+)");

    QRegularExpressionMatch userName = nameRule.match(curl);
    QRegularExpressionMatch userPassword = passwordRule.match(curl);

    if(userName.hasMatch())
    {
        ui->zLineEdit->setText(userName.captured(1));

    }
    if(userPassword.hasMatch())
    {
         ui->mLineEdit->setText(userPassword.captured(1));
    }
}




void Widget::on_autoStartCheckBox_stateChanged(int arg1)
{
    // 获取文件路径
    QString path = QCoreApplication::applicationFilePath().replace("/","\\");
    // 写自启命令
    QString command = "\"" + path + "\" -autostart";

    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    if(arg1 == Qt::Checked)
    {
        reg.setValue("AutoNetworkConnection", command); //自启
    }
    else
    {
        reg.remove("AutoNetworkConnection"); //取消自启
    }

}

