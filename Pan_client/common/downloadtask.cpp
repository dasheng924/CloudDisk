#include "downloadtask.h"
#include "common/downloadlayout.h"

//静态成员变量的初始化
DownloadTask* DownloadTask::instance ;
DownloadTask::Garbo DownloadTask::garbo;

DownloadTask::DownloadTask()
{

}

DownloadTask::~DownloadTask()
{

}

DownloadTask::DownloadTask(const DownloadTask &)
{

}

DownloadTask& DownloadTask::operator=(const DownloadTask &)
{
    return *this;
}

DownloadTask* DownloadTask::getInstance()
{
    //判断是否第一次调用
    if(instance == NULL)
    {
        DownloadTask::instance = new DownloadTask;
    }
    return DownloadTask::instance;
}

//清空上传列表
void DownloadTask::clearList()
{
    int n = list.size();
    for(int i = 0;i<n;++i)
    {
        DownloadInfo *tmp = list.takeFirst();
        delete  tmp;
    }
}

//判断上传队列是否为空
bool DownloadTask::isEmpty()
{
    return list.isEmpty();
}

//是否有文件正在下载
bool DownloadTask::isDownload()
{
    int size = list.size();

    for(int i = 0;i<size;++i)
    {
        if(list.at(i)->isDownload == true) //说明有下载任务，不能添加新任务
        {
            return true;
        }
    }
    return false;
}

//判断第一个任务是否为共享的任务
bool DownloadTask::isShareTask()
{
    if( isEmpty() )
    {
        return NULL;
    }

    return list.at(0)->isShare;
}

//取出第0个下载任务，如果任务队列没有任务在下载，设置第0个任务下载
DownloadInfo * DownloadTask::takeTask()
{
    if( isEmpty() )
    {
        return NULL;
    }

    list.at(0)->isDownload = true; //标志为在下载
    return list.at(0);
}

//删除下载完成的任务
void DownloadTask::dealDownloadTask()
{
    int n = list.size();
    for(int i = 0;i<n;++i)
    {
        if( list.at(i)->isDownload == true) //说明有下载任务
        {
            //移除此文件，因为已经上传完成了
            DownloadInfo *tmp = list.takeAt(i);

            //获取布局
            DownloadLayout *downloadLayout = DownloadLayout::getInstance();
            QLayout *layout = downloadLayout->getDownloadLayout();

            layout->removeWidget(tmp->dp); //从布局中移除控件
            delete tmp->dp;

            QFile *file = tmp->file;
            file->close();  //关闭文件
            delete file;    //释放空间

            delete tmp; //释放空间
            return;
        }

    }

}

//追加任务到下载队列
//参数：info：下载文件信息， filePathName：文件保存路径
//成功：0
//失败：
//  -1: 下载的文件是否已经在下载队列中
//  -2: 打开文件失败
int DownloadTask::appendDownloadList(FileInfo *info, QString filePathName, bool isShare)
{
    //遍历查看一下，下载的文件是否已经在下载队列中
    int size = list.size();
    for(int i = 0;i<size;++i)
    {
        if(list.at(i)->user == info->user && list.at(i)->filename == info->filename)
        {
            cout << info->filename << " 已经在下载队列中 ";
            return -1;
        }
    }

    QFile *file = new QFile(filePathName);
    if(!file->open(QIODevice::WriteOnly))
    {
        //如果打开文件失败，则删除 file，并使 file 指针为 NULL，然后返回
        cout << "file open error";

        delete file;
        file = NULL;
        return -2;
    }

    //动态创建节点
    DownloadInfo *tmp = new DownloadInfo;
    tmp->user = info->user;   //用户
    tmp->file = file;         //文件指针
    tmp->filename = info->filename; //文件名字
    tmp->md5 = info->md5;           //文件md5
    tmp->url = info->url;           //下载网址
    tmp->isDownload = false;        //没有在下载
    tmp->isShare = isShare;         //是否为共享文件下载

    DataProgress *p = new DataProgress;
    p->setFileName(tmp->filename);

    //获取布局
    DownloadLayout *downloadLayout = DownloadLayout::getInstance();
    QVBoxLayout *layout = (QVBoxLayout*)downloadLayout->getDownloadLayout();

    tmp->dp = p;
    // 添加到布局, 最后一个是弹簧, 插入到弹簧上边
    layout->insertWidget(layout->count()-1, p);
    cout << info->url << "已经添加到下载列表";

    //插入节点
    list.append(tmp);

    return 0;
}
