#include "log.h"
#include "ui_log.h"

Log::Log(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Log)
{
    ui->setupUi(this);

    // 美化UI，设置背景和透明度
    this->setWindowOpacity(0.95);
    this->setAttribute(Qt::WA_StyledBackground, true); // 允许自定义QWidget使用QSS绘制背景
    this->setStyleSheet(
        "QWidget#Log {"
        "   border-image: url(:/background/bg1.jpg);" // 使用另一张图片作为日志背景
        "}"
        "QLabel {"
        "   color: #2c3e50;"
        "   font-weight: bold;"
        "}"
        "QTextBrowser {"
        "   background-color: rgba(255, 255, 255, 0.6);"
        "   border: 1px solid rgba(255, 255, 255, 0.8);"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "   color: #2c3e50;"
        "   font-family: Consolas, 'Microsoft YaHei', monospace;"
        "   font-size: 10pt;"
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
    );

    connect(ui->stopButton,&QPushButton::clicked,this,&Log::stopRequest);
}

Log::~Log()
{
    delete ui;
}


void Log::closeEvent(QCloseEvent *event)
{
    emit stopRequest();
    event->ignore();
}

void Log::appendLog(const QString &msg)
{
    ui->logBrowser->append(msg);
}

void Log::clearLog()
{
    ui->logBrowser->clear();
}