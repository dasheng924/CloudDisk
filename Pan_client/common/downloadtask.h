#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QFile>
#include <QUrl>
#include <QList>

#include "myselfWidget/dataprogress.h"
#include "common/common.h"

//单例模式---下载任务列表
//下载文件信息
struct DownloadInfo
{
    QFile *file;        //文件指针
    QString user;       //下载用户
    QString filename;   //文件名字
    QString md5;        //文件md5
    QUrl url;           //下载网址
    bool isDownload;    //是否已经在下载
    DataProgress *dp;   //下载进度控件
    bool isShare;       //是否为共享文件下载
};



class DownloadTask
{
public:

    static DownloadTask *getInstance();

    void clearList(); //清空上传列表
    bool isEmpty();   //判断上传队列是否为空
    bool isDownload(); //是否有文件正在下载
    bool isShareTask();//第一个任务是否为共享文件的任务
    DownloadInfo *takeTask();//取出第0个下载任务，如果任务队列没有任务在下载，设置第0个任务下载
    void dealDownloadTask(); //删除下载完成的任务


    //追加任务到下载队列
    //参数：info：下载文件信息， filePathName：文件保存路径, isShare: 是否为共享文件下载, 默认为false
    //成功：0
    //失败：
    //  -1: 下载的文件是否已经在下载队列中
    //  -2: 打开文件失败
    int appendDownloadList( FileInfo *info, QString filePathName, bool isShare = false);





private:
    DownloadTask();
    ~DownloadTask();
    DownloadTask(const DownloadTask &);
    DownloadTask& operator=(const DownloadTask &);

    static DownloadTask * instance;

    class Garbo{
    public:
        ~Garbo(){
            if(DownloadTask::instance != nullptr)
            {
                DownloadTask::instance->clearList();
                delete  DownloadTask::instance;
                DownloadTask::instance  = nullptr;
            }
        }
    };

    //定义一个静态成员变量，程序结束时，系统会自动调用它的析构函数
    //static类的析构函数在main()退出后调用
    static Garbo garbo;

    QList<DownloadInfo*> list; //保存文件信息的list，核心的存储结构
};

#endif // DOWNLOADTASK_H
