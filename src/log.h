#ifndef LOG_H
#define LOG_H

#include <QWidget>
#include <QCloseEvent>

namespace Ui {
class Log;
}

class Log : public QWidget
{
    Q_OBJECT

public:
    explicit Log(QWidget *parent = nullptr);
    // 清除日志
    void clearLog();
    ~Log();

public slots:
    // 追加显示日志
    void appendLog(const QString &msg);

signals:
    // 停止信号
    void stopRequest();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Log *ui;
};

#endif // LOG_H
