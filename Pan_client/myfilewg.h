#ifndef MYFILEWG_H
#define MYFILEWG_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QTimer>


#include "myselfWidget/mymenu.h"
#include "common/common.h"
#include "common/uploadtask.h"
#include "common/uploadlayout.h"
#include "common/downloadlayout.h"
#include "common/downloadtask.h"
#include "common/logininfoinstance.h"
#include "myselfWidget/filepropertyinfo.h"

namespace Ui {
class MyFileWg;
}

class MyFileWg : public QWidget
{
    Q_OBJECT

public:
    explicit MyFileWg(QWidget *parent = nullptr);
    ~MyFileWg();
public:

    //==========>我的文件初始化操作<==============
    void initListWidget();

    //==========>右键菜单的设置<==============
    void addActionMenu();//右键菜单的添加
    void rightMenu(const QPoint &pos);//右键菜单的显示

    //==========>上传文件处理<==============
    // 添加需要上传的文件到上传任务列表
    void addUploadFiles();
    //取出上传任务队列的第一个，上传成功完成后，再处理下一个任务
    void uploadFilesAction();
    //把用户名，用户的token，要上传文件的md5,要上传文件的名字打包成json
    // 设置md5信息的json包
    QByteArray setMd5Json(QString user, QString token, QString md5, QString fileName);
    //真正的上传
    void uploadFile(UploadFileInfo *info);

    //==========>文件item的展示<==============
    // 添加“上传文件item”（图标）
    void addUploadItem(QString iconPath=":/images/upload.png", QString name="上传文件");
    // 清空文件列表
    void clearFileList();
    // 清空所有item项目
    void clearItems();
    // 文件item展示
    void refreshFileItems();

    //==========>显示用户的文件列表<==============
    // desc是descend 降序意思
    // asc 是ascend 升序意思
    // Normal：普通用户列表，PvAsc：按下载量升序， PvDesc：按下载量降序

    enum Display{Normal, PvAsc, PvDesc};//用来表示刷新的模式
    QByteArray setGetCountJson(QString user, QString token);//查询用户拥有的文件数时，发送的json数据包
    void refreshFiles(Display cmd=Normal);//刷新用户文件列表，主要是获得了拥有的文件的数量

    // 获取用户文件列表
    void getUserFilesList(Display cmd=Normal);//获取到用户文件列表
    // 解析文件列表json信息，存放在文件列表中
    void getFileJsonInfo(QByteArray data);//解析获得的文件列表json




    //==========>分享，删除，文件属性<==============
    //分享，删除，文件属性统一接口
    // 处理选中的文件
    void dealSelectdFile(QString cmd="分享");
    void shareFile(FileInfo *); //分享某个文件
    void delFile(FileInfo *); //删除某个文件
    void getFileProperty(FileInfo *); //获取属性信息



    //==========>下载文件处理<==============
    void addDownloadFiles();
    void downloadFilesAction();
    void dealFilePv(QString md5, QString filename);





    //==========>定时检查处理任务队列中的任务<==============
    //上传任务和下载任务的触发都是由这个定时器控制的
    void checkTaskList();
    void clearAllTask();//清空所有任务队列
    //==========>操作记录写到文件<==============
    // 传输数据记录到本地文件，user：操作用户，name：操作的文件, code: 操作码， path: 文件保存的路径
    void writeRecord(QString user, QString name, QString code, QString path = RECORDDIR);



signals:
    void gotoTransfer(TransferStatus status);//切换到传输的页面
    void loginAgainSignal(); //发送重新登陆信号

private:
    Ui::MyFileWg *ui;

    //==============菜单相关的变量============
    MyMenu   *m_menu;           // 菜单1
    QAction *m_downloadAction; // 下载
    QAction *m_shareAction;    // 分享
    QAction *m_delAction;      // 删除
    QAction *m_propertyAction; // 属性

    //点击空白处的菜单
    MyMenu   *m_menuEmpty;          // 菜单2
    QAction *m_pvAscendingAction;  // 按下载量升序
    QAction *m_pvDescendingAction; // 按下载量降序
    QAction *m_refreshAction;      // 刷新
    QAction *m_uploadAction;       // 上传

    //用户拥有的文件信息
    long m_userFilesCount;        //用户文件数目
    int m_start;                  //文件位置起点
    int m_count;                  //每次请求文件个数


    //定时器
    QTimer m_uploadFileTimer;       //定时检查上传队列是否有任务需要上传
    QTimer m_downloadTimer;         //定时检查下载队列是否有任务需要下载

    QNetworkAccessManager *m_manager; //http管理类对象

    Common m_comm; //通用接口类的对象

    QList<FileInfo*> m_fileList; //保存每一个文件的信息
};

#endif // MYFILEWG_H
