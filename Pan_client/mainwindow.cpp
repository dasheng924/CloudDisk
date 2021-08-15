#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "login.h"
#include "buttongroup.h"

#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("极速云盘");
    this->setWindowIcon(QIcon(":/images/logo.ico"));
    this->setGeometry(0,0,900,650);
    m_comm.moveToCenter(this);//moveToCenter 里面的show()已被屏蔽

    ui->stackedWidget->setCurrentWidget(ui->myfiles_page);

    //无边框的设置
    this->setWindowFlags(Qt::FramelessWindowHint|windowFlags());

    //集中开始信号的处理
    manageAllSignals();






}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showMainWindow()
{
    //居中显示
    m_comm.moveToCenter(this);

    this->show();

    //切换到我的文件页面
    ui->stackedWidget->setCurrentWidget(ui->myfiles_page);

    //刷新用户文件列表
    ui->myfiles_page->refreshFiles();

}

//集中处理信号（主要是上半部分切换页面）
void MainWindow::manageAllSignals()
{
    //最小化，关闭，最大化的操作已经在buttongroup.cpp 里面处理完成了
    //处理栈窗口的切换

    //我的文件
    connect(ui->btn_group,&ButtonGroup::sigMyFile,this,[=](){
        ui->stackedWidget->setCurrentWidget(ui->myfiles_page);
        ui->myfiles_page->refreshFiles();//刷新文件列表

    });

    //共享列表
    connect(ui->btn_group,&ButtonGroup::sigShareList,this,[=](){
        ui->stackedWidget->setCurrentWidget(ui->sharefile_page);
        // 刷新分享列表
        ui->sharefile_page->refreshFiles();
    });

    //下载排行
    connect(ui->btn_group,&ButtonGroup::sigDownload,this,[=](){
        ui->stackedWidget->setCurrentWidget(ui->ranking_page);
        // 刷新下载榜列表
        ui->ranking_page->refreshFiles();
    });

    //传输页面
    connect(ui->btn_group,&ButtonGroup::sigTransform,this,[=](){
        ui->stackedWidget->setCurrentWidget(ui->transfer_page);
    });


    //切换用户
    connect(ui->btn_group,&ButtonGroup::sigSwitchUser,this,[=](){
        qDebug()<<"切换用户";
        loginAgain();
    });

    //对于MyFileWg页面的信号
//    void gotoTransfer(TransferStatus status);//切换到传输的页面
//    void loginAgainSignal(); //发送重新登陆信号

    connect(ui->myfiles_page,&MyFileWg::gotoTransfer,this,[=](TransferStatus status){

        ui->btn_group->slotButtonClick(Page::TRANSFER);
        if(status == TransferStatus::Uplaod)
        {
            ui->transfer_page->showUpload();
        }
        else if(status == TransferStatus::Download)
        {
            ui->transfer_page->showDownload();
        }
    });


    // 信号传递
    connect(ui->sharefile_page, &ShareList::gotoTransfer, ui->myfiles_page, &MyFileWg::gotoTransfer);
}


void MainWindow::loginAgain()
{
    // 发送信号，告诉登陆窗口，切换用户
    emit changeUser();
    // 清空上一个用户的上传或下载任务
    ui->myfiles_page->clearAllTask();

    // 清空上一个用户的一些文件显示信息
    ui->myfiles_page->clearFileList();
    ui->myfiles_page->clearItems();
}

