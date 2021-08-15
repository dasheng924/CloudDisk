#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "common/common.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 显示主窗口
    void showMainWindow();

    //集中处理信号（主要是上半部分切换页面）
    void manageAllSignals();

    void loginAgain();//切换用户

signals:
    // 切换用户按钮信号
    void changeUser();


protected:

private:
    Ui::MainWindow *ui;
    Common m_comm;
};
#endif // MAINWINDOW_H
