#include "transfer.h"
#include "ui_transfer.h"

#include "common/uploadlayout.h"
#include "common/downloadlayout.h"
#include "common/logininfoinstance.h"

#include <QFile>
#include <QVBoxLayout>

Transfer::Transfer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Transfer)
{
    ui->setupUi(this);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(0,0,0,0);
    ui->upload_scroll->setLayout(vlayout);


    QVBoxLayout *vlayout2 = new QVBoxLayout;
    vlayout2->setContentsMargins(0,0,0,0);
    ui->download_scroll->setLayout(vlayout2);

    //设置上传布局
    UploadLayout *layout = UploadLayout::getInstance();
    //cout<<ui->upload_scroll->layout();

    layout->setUploadLayout(ui->upload_scroll);

    //设置下载布局
    DownloadLayout *downloadLayout = DownloadLayout::getInstance();
    downloadLayout->setDownloadLayout(ui->download_scroll);


    //切换tab
    connect(ui->tabWidget,&QTabWidget::currentChanged,this,[=](int index){

        if(index == 0)
        {
            emit currentTabSignal("正在上传");
        }
        else if(index == 1)
        {
            emit currentTabSignal("正在下载");
        }
        else {
            emit currentTabSignal("传输记录");
            dispayDataRecord();
        }
    });

    // 设置样式 tabWidget
    ui->tabWidget->tabBar()->setStyleSheet(
        "QTabBar::tab{"
        "background-color: rgb(182, 202, 211);"
        "border-right: 1px solid gray;"
        "padding: 6px"
        "}"
        "QTabBar::tab:selected, QtabBar::tab:hover {"
        "background-color: rgb(20, 186, 248);"
        "}"
        );

    //清空记录
    connect(ui->clearBtn,&QToolButton::clicked,this,[=](){

        logininfoinstance *login = logininfoinstance::getInstance();
        QString filename = RECORDDIR+login->getUserName();

        if(QFile::exists(filename))
        {
            QFile::remove(filename);
            ui->record_msg->clear();
        }

    });

    ui->tabWidget->setCurrentWidget(ui->record);

}

Transfer::~Transfer()
{
    delete ui;
}

void Transfer::dispayDataRecord(QString path)
{
    logininfoinstance *login = logininfoinstance::getInstance();
    QString filename = path+login->getUserName();

    QFile file(filename);

    int ret = file.open(QIODevice::ReadOnly);
    if(!ret)
    {
        cout<<__FUNCTION__<<"open fail";
        return;
    }

    QByteArray data =file.readAll();
    file.close();

#ifdef TARGET_OS_WIN32
    ui->record_msg->setText(QString::fromLocal8Bit(data));
#else
    ui->record_msg->setText(data);
#endif

}

void Transfer::showUpload()
{
    ui->tabWidget->setCurrentWidget(ui->upload);
}

void Transfer::showDownload()
{
    ui->tabWidget->setCurrentWidget(ui->download);
}
