#include "uploadtask.h"

#include "common/common.h"
#include "common/uploadlayout.h"

#include <QFileInfo>
#include <QFile>



UploadTask * UploadTask::instance = new UploadTask;
UploadTask::Garbo UploadTask::garbo ;

UploadTask::UploadTask()
{

}

UploadTask::~UploadTask()
{

}
UploadTask::UploadTask(const UploadTask&)
{

}
UploadTask & UploadTask::operator=(const UploadTask &)
{
    return *this;
}

UploadTask* UploadTask::getInstance()
{
    return UploadTask::instance;
}

bool UploadTask::isEmpty()
{
    return list.isEmpty();
}


//这个队列中有一个任务的时候，就是在上传
bool UploadTask::isUpload()
{
    //遍历list
    for(int i = 0;i<list.size();++i)
    {
        if(list.at(i)->isUpload == true)
        {
            return true;
        }
    }
    return false;
}

// 取出第0个上传任务，如果任务队列没有任务在上传，设置第0个任务上传
UploadFileInfo* UploadTask::takeTask()
{
    UploadFileInfo *tmp = list.at(0);
    list.at(0)->isUpload = true ; //标志位，设置此文件在上传

    return  tmp;
}

//清空上传列表
void UploadTask::clearList()
{
    int n = list.size();
    for(int i = 0; i < n; ++i)
    {
        UploadFileInfo *tmp = list.takeFirst();//删除第一个并返回这个
        delete tmp;
    }
}

QList<UploadFileInfo *> UploadTask::getUploadTaskList()
{
    return list;
}

//追加上传文件到上传列表中
//参数：path 上传文件路径
//失败：
//  -1: 文件大于30m
//  -2：上传的文件是否已经在上传队列中
//  -3: 打开文件失败
//  -4: 获取布局失败

//把路径作为参数
int UploadTask::appendUploadList(QString path)
{
    //------------------四个返回值情况的判断-----------
    //获取文件相关信息,超过30M就先不处理
    qint64 size = QFileInfo(path).size(); //获取文件的大小
    if(size > 50*1024*1024)
    {
        cout<<path<<"is too big!!!";
        return -1;
    }

    //遍历一下，看文件是否在上传队列
    for(int i = 0;i<list.size();++i)
    {
        if(list.at(i)->path == path)
        {
            cout<<QFileInfo(path).fileName()<<"已经在上传队列中了!";
            return -2;
        }
    }

    QFile *file = new QFile(path);
    bool ret = file->open(QIODevice::ReadOnly);
    if(!ret)
    {
        cout<<QFileInfo(path).fileName()<<"打开文件失败!";
        delete  file;
        file = nullptr;
        return -3;
    }

    //------------------准备把这些信息填入结构体当中-----------

    //获取文件信息
    QFileInfo info(path);

    Common comm;
    UploadFileInfo *tmp = new UploadFileInfo;//初始化文件信息结构体
    tmp->md5 = comm.getFileMd5(path);//文件内容转化为MD5
    tmp->file = file;//文件指针，也就是之前的new 出来的文件指针
    tmp->fileName = info.fileName(); //文件名字
    tmp->size = size;//文件大小
    tmp->path = path;//文件路径
    tmp->isUpload = false;//文件是否在上传

    //创建一个进度条
    DataProgress *p = new DataProgress;
    p->setFileName(info.fileName()); //给进度条设定一个标题
    tmp->dp = p; //进度条的初始化

    //------------------获取一个布局，把进度条放到其中-----------

    //获取一个垂直布局
    UploadLayout *pUpload = UploadLayout::getInstance();
    if(pUpload == NULL)
    {
        cout<<__FUNCTION__<<"getInstance error";
        return -4;
    }

    //cout<<"走到这里了!";
    //转化为一个垂直布局
    QVBoxLayout *layout = (QVBoxLayout*)pUpload->getUploadLayout();

    // 添加到布局, 最后一个是弹簧, 插入到弹簧上边
    //cout<<__FUNCTION__<<layout->count();
    layout->insertWidget(layout->count()-1, p);

    //-------------------任务队列里面加上一个---------
    list.append(tmp);
    cout<<tmp->fileName.toUtf8().data()<<"插入一个上传任务成功！";

    return 0;
}

//删除上传成功的任务
void UploadTask::dealUploadTask()
{
    //遍历
    for(int i = 0;i<list.size();++i)
    {
        if(list.at(i)->isUpload == true) //说明有上传任务
        {
            //删除掉这个任务
            UploadFileInfo *tmp = list.takeAt(i);

            //获取布局
            UploadLayout *pUpload = UploadLayout::getInstance();
            QLayout *layout = pUpload->getUploadLayout();
            layout->removeWidget(tmp->dp);

            //关闭打开的文件指针
            tmp->file->close();
            delete  tmp->file;

            //释放进度条的堆空间
            delete tmp->dp;

            //释放每一个task的堆空间
            delete tmp;

            return;
        }
    }

}

