#ifndef UPLOADTASK_H
#define UPLOADTASK_H

//这个类的功能就是 把要添加文件的信息整合到结构体添加到任务队列中

#include <QString>
#include <QFile>
#include "myselfWidget/dataprogress.h"


//每一个上传的文件信息，用这个结构体描述
struct UploadFileInfo
{
    QString md5;        //文件md5码
    QFile *file;        //文件指针
    QString fileName;   //文件名字
    qint64 size;        //文件大小
    QString path;       //文件路径
    bool isUpload;      //是否已经在上传
    DataProgress *dp;   //上传进度控件,这个控件是自定义的控件,用来记录每一个文件传输的进度

};

//单例模式，一个程序只能有一个上传列表


class UploadTask
{
public:
    static UploadTask* getInstance(); //获取唯一的实例

    //追加上传文件到上传列表中
    //参数：path 上传文件路径
    //返回值：成功为0
    //失败：
    //  -1: 文件大于30m
    //  -2：上传的文件是否已经在上传队列中
    //  -3: 打开文件失败
    //  -4: 获取布局失败
    int appendUploadList(QString path);
    //判断上传队列是否为空
    bool isEmpty();
    //判断是否有任务正在上传
    bool isUpload();
    //取出第0个上传任务，如果任务队列没有任务在上传，设置第0个任务上传
    UploadFileInfo * takeTask();
    //删除上传完成的任务
    void dealUploadTask();
    //清空上传列表
    void clearList();
    //获取任务列表里面的任务
    QList<UploadFileInfo*> getUploadTaskList();
private:
    UploadTask();
    ~UploadTask();
    UploadTask(const UploadTask&);
    UploadTask& operator=(const UploadTask&);

    //静态数据成员，这个是要在类外进行初始化
    static UploadTask *instance;

    class Garbo{
    public:
        ~Garbo()
        {
            if(NULL != UploadTask::instance)
            {
                UploadTask::instance->clearList();
                delete UploadTask::instance;
                UploadTask::instance = NULL;
            }

        }
    };

    static Garbo garbo;
    QList<UploadFileInfo*> list;
};

#endif // UPLOADTASK_H
