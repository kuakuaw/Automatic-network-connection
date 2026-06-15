#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSettings>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QThread>
#include <memory>
#include <QTimer>
#include "log.h"
#include "connectionworker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

signals:
    // 给线程的开始信号
    void opreateWork(const QString &curlTemplate, int ICMPSeconds,bool showDetailedLog);

private slots:

    void on_startButton_clicked();
    void stopLogInfo();
    void on_parseButton_clicked();

    void on_autoStartCheckBox_stateChanged(int arg1);

private:
    Ui::Widget *ui;
    // Log *log = nullptr;
    std::unique_ptr<Log> log;
    QString configPath = QCoreApplication::applicationDirPath()+R"(\config.ini)";

    // 线程部分
    QThread *workThread = nullptr;
    // ConnectionWorker *worker = nullptr;
    std::unique_ptr<ConnectionWorker> worker;

};
#endif // WIDGET_H
