#include "myfilewg.h"
#include "ui_myfilewg.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QUrl>

MyFileWg::MyFileWg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyFileWg)
{
    ui->setupUi(this);


    //初始化“我的文件”的listWidget主件
    initListWidget();

    // 添加右键菜单，这里面给menu分配内存了，需要先在构造里面调用
    addActionMenu();

    //addUploadItem();//先添加进来测试，具体的调用在别的函数里面

    //获得http的管理类对象
    m_manager = m_comm.getNetManager();

    //定时检查任务队列，做对应的处理
    checkTaskList();

    //刷新用户的文件列表
    //refreshFiles();
}

MyFileWg::~MyFileWg()
{
    delete ui;
}


//初始化“我的文件”的listWidget主件
void MyFileWg::initListWidget()
{
    //给listwidget 设置显示模式
    ui->listWidget->setViewMode(QListView::IconMode);//设置图标显示
    ui->listWidget->setIconSize(QSize(60,60));//设置图标大小
    ui->listWidget->setGridSize(QSize(80,80));//设置item大小

    // 设置QLisView大小改变时，图标的调整模式，默认是固定的，可以改成自动调整
    ui->listWidget->setResizeMode(QListView::Adjust);
    // 设置列表可以拖动，如果想固定不能拖动，使用QListView::Static
    ui->listWidget->setMovement(QListView::Static);
    // 设置图标之间的间距, 当setGridSize()时，此选项无效
    ui->listWidget->setSpacing(35);

    //listWidget右键菜单的设置
    // 发出 customContextMenuRequested 信号
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget,&QListWidget::customContextMenuRequested,this,&MyFileWg::rightMenu);

    //点击文件中的上传图标显示菜单
    connect(ui->listWidget,&QListWidget::itemPressed,this,[=](QListWidgetItem *item){
        if(item->text() == "上传文件")
        {
            //添加需要上传的文件到上传任务列表
            addUploadFiles();
        }
    });
}

//======================>右键菜单<================
//右键菜单的显示
void MyFileWg::rightMenu(const QPoint &pos)
{
    QListWidgetItem * item = ui->listWidget->itemAt(pos);
    if(item == NULL) //点击了空白处
    {
        m_menuEmpty->exec(QCursor::pos());
    }
    else{
        ui->listWidget->setCurrentItem(item);
        if(item->text() == "上传文件") //点击上传文件图标，没有右键菜单
        {
            return;
        }

        m_menu->exec(QCursor::pos()); //图标上点击右键
    }

}

//右键菜单功能的添加
void MyFileWg::addActionMenu()
{
    //菜单一：点击文件处的菜单
    m_menu = new MyMenu(this);

    //动作初始化
    m_downloadAction = new QAction("下载",this);
    m_shareAction = new QAction("分享",this);
    m_delAction = new QAction("删除",this);
    m_propertyAction = new QAction("属性",this);
    //添加动作
    m_menu->addAction(m_downloadAction);
    m_menu->addAction(m_shareAction);
    m_menu->addAction(m_delAction);
    m_menu->addAction(m_propertyAction);

    //菜单一：点击空白处的菜单
    m_menuEmpty = new MyMenu(this);

    m_pvAscendingAction = new QAction("按下载量升序",this);
    m_pvDescendingAction = new QAction("按下载量降序",this);
    m_refreshAction = new  QAction("刷新",this);
    m_uploadAction = new QAction("上传",this);

    m_menuEmpty->addAction(m_pvAscendingAction);
    m_menuEmpty->addAction(m_pvDescendingAction);
    m_menuEmpty->addAction(m_refreshAction);
    m_menuEmpty->addAction(m_uploadAction);


    //===============动作对应的操作===========
    //下载
    connect(m_downloadAction,&QAction::triggered,this,[=](){
        //添加需要下载的文件到下载任务列表
        addDownloadFiles();

    });

    //分享
    connect(m_shareAction,&QAction::triggered,this,[=](){
        dealSelectdFile("分享");

    });

    //删除
    connect(m_delAction,&QAction::triggered,this,[=](){

        dealSelectdFile("删除");
    });

    //文件属性
    connect(m_propertyAction,&QAction::triggered,this,[=](){
        dealSelectdFile("属性");
    });

    //按下载量升序
    connect(m_pvAscendingAction,&QAction::triggered,this,[=](){
        refreshFiles(PvAsc);
    });

    //按下载量升序
    connect(m_pvDescendingAction,&QAction::triggered,this,[=](){
        refreshFiles(PvDesc);
    });

    //刷新
    connect(m_refreshAction,&QAction::triggered,this,[=](){
        //刷新用户的文件列表
        refreshFiles();
    });

    //上传
    connect(m_uploadAction,&QAction::triggered,this,[=](){
        //添加需要上传的文件到上传任务列表
        addUploadFiles();
        //emit m_refreshAction->triggered();
    });

}


//==========================>上传文件<================
//添加需要上传的文件到上传任务列表
void MyFileWg::addUploadFiles()
{
    //这个信号用于切换上传传输页面
//    emit gotoTransfer(TransferStatus::Uplaod);

    //获取上传列表实例
    UploadTask *uploadList = UploadTask::getInstance();
    if(uploadList == NULL)
    {
        cout<<__FUNCTION__<<"获取上传实例失败!";
        return;
    }

    //打开获取文件对话框
    QStringList list = QFileDialog::getOpenFileNames();
    if(!list.isEmpty())
    {
        //这个信号用于切换上传传输页面
        emit gotoTransfer(TransferStatus::Uplaod);

    }

    //把每一个文件添加到上传任务列表中
    for(int i = 0;i<list.size();++i)
    {
        cout<<"所选的文件是:"<<list.at(i);
        //添加任务队列的返回值
        //  -1: 文件大于30m
        //  -2：上传的文件是否已经在上传队列中
        //  -3: 打开文件失败
        //  -4: 获取布局失败
        int ret = uploadList->appendUploadList(list.at(i));
        if(ret == -1)
        {
            QMessageBox::critical(this,"错误","上传文件超过30M");
        }
        else if(ret == -2)
        {
            QMessageBox::warning(this,"警告","上传的文件已经在队列中!");
        }
        else if(ret == -3)
        {
            QMessageBox::critical(this,"错误","要上传的文件打开失败!");
        }
        else if(ret == -4)
        {
            QMessageBox::critical(this,"错误","获取进度条的布局失败!");
        }
    }

}

//把用户名，用户的token，要上传文件的md5,要上传文件的名字打包成json
QByteArray MyFileWg::setMd5Json(QString user, QString token, QString md5, QString fileName)
 {
     /*json数据如下
    {
        user:xxxx,
        token:xxxx,
        md5:xxx,
        fileName: xxx
    }
    */
     QMap<QString,QVariant> info;
     info.insert("user",QVariant(user));
     info.insert("token",QVariant(token));
     info.insert("md5",QVariant(md5));
     info.insert("fileName",QVariant(fileName));

     QJsonDocument doc = QJsonDocument::fromVariant(info);
     if(doc.isNull() || doc.isEmpty())
     {
         cout<<__FUNCTION__<<"转化为QByteArray fail!";
     }

     QByteArray data = doc.toJson();


     return data;
 }

//操作记录写进文件，进而展示到对应的界面
void MyFileWg::writeRecord(QString user, QString name, QString code, QString path)
 {
     //文件上传后，在本地留存记录
     QString fileName = path+user;
     cout<<fileName;

     //检查要放记录的目录是否存在，不存在就创建这个目录
     QDir dir(path);
     if(!dir.exists())//不存在目录
     {
         //不存在目录。创建目录
         if(dir.mkpath(path))
         {
             cout<<path<<"创建目录成功!";
         }
         else
         {
             cout<<path<<"创建目录失败!";
         }
     }

     //看对应的记录文件是否存在
     QFile file(fileName);
     QByteArray data;

     if(true == file.exists()) //要记录的文件已经存在
     {
         if(!file.open(QIODevice::ReadOnly))
         {
             cout<<"读取原来的记录文件失败!";
             return;
         }
         data = file.readAll();
         file.close();
     }

     //把这次新的记录写入文件
     if(!file.open(QIODevice::WriteOnly))
     {
         cout<<"打开准备写入的记录文件失败!";
         return;
     }

     //记录格式
     // xxx.jpg       2017年2月27日12:04:49       秒传成功

     //获取当前的系统时间
     QDateTime dateTime = QDateTime::currentDateTime();
     QString time = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz ddd ");

     /*
       秒传文件：
            文件已存在：{"code":"005"}
            秒传成功：  {"code":"006"}
            秒传失败：  {"code":"007"}
        上传文件：
            成功：{"code":"008"}
            失败：{"code":"009"}
        下载文件：
            成功：{"code":"010"}
            失败：{"code":"011"}
    */
     QString codeStr;
     if(code == "005")
     {
         codeStr = "秒传文件-文件已存在";
     }
     else if(code == "006")
     {
         codeStr = "秒传文件-秒传成功";
     }
     else if(code == "007")
     {
         codeStr = "秒传文件-秒传失败";
     }
     else if(code == "008")
     {
         codeStr = "上传文件成功";
     }
     else if(code == "009")
     {
         codeStr = "上传文件失败";
     }
     else if(code == "010")
     {
         codeStr= "下载文件成功";
     }
     else if(code == "011")
     {
         codeStr="下载文件失败";
     }

     cout<<name;
     cout<<time;
     cout<<codeStr;

     QString str = QString("[%1]\t%2\t%3\r\n").arg(name).arg(time).arg(codeStr);

     file.write(str.toLocal8Bit());//先写新内容

     if(!data.isEmpty())
     {
         file.write(data);//再写老内容
     }

     file.close();
 }

//真正的上传文件
void MyFileWg::uploadFile(UploadFileInfo *info)
{
    //取出要上传的任务，要上传的任务就是一个文件的相关信息
    QFile *file = info->file;//文件指针
    QString fileName = info->fileName;//文件的名字
    QString md5 = info->md5;//文件内容的md5
    qint64 size = info->size;//文件的大小

    DataProgress *dp = info->dp;//每一个文件对应的进度条
    QString boundary= m_comm.getBoundary();//post传输产生的分割线

    //获取登录实例额，获取用户相关的信息
    logininfoinstance *login = logininfoinstance::getInstance();

    //构建要发送的内容
    /*
    ------WebKitFormBoundary88asdgewtgewx\r\n
    Content-Disposition: form-data; user="mike"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
    Content-Type: application/octet-stream\r\n
    \r\n
    真正的文件内容\r\n
    ------WebKitFormBoundary88asdgewtgewx
    */

    QByteArray data;
    //分割线
    data.append(boundary);
    data.append("\r\n");
    //文件的信息

    data.append("Content-Disposition: form-data; ");
    data.append( QString("user=\"%1\" ").arg( login->getUserName() ) ); //上传用户
    data.append( QString("filename=\"%1\" ").arg(fileName) ); //文件名字
    data.append( QString("md5=\"%1\" ").arg(md5) ); //文件md5码
    data.append( QString("size=%1").arg(size)  );   //文件大小
    data.append("\r\n");
    //数据的格式
    data.append(QString("Content-Type: application/octet-stream"));
    data.append("\r\n\r\n");

    //要发送的真正的数据
    data.append(file->readAll());
    data.append("\r\n");
    //添加分割线
    data.append(boundary);

    //===================》准备http的发送《===================
    QNetworkRequest request;
    //准备http头
    QString url = QString("http://%1:%2/upload").arg(login->getIp()).arg(login->getPort());
    request.setUrl(QUrl(url));
    //Qt中的默认请求头
    //request.setRawHeader("Content-Type","text/html");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    cout<<"开始真正的数据传输";


    //准备发送post请求
    QNetworkReply *reply = m_manager->post(request,data);
    if(reply == NULL)
    {
        cout<<__FUNCTION__<<"发送post请求失败！";
        return;
    }


    //有数据传输的时候，我们可以活得对应的传输量
    connect(reply,&QNetworkReply::uploadProgress,this,[=](qint64 bytesRead, qint64 totalBytes){

        //cout<<bytesRead<<totalBytes;

        if(totalBytes != 0) //这个条件很重要
        {
            //cout << bytesRead/1024 << ", " << totalBytes/1024;
            dp->setProgress(bytesRead/1024, totalBytes/1024); //设置进度条
        }

    });



    //获取服务器回复的数据
    connect(reply,&QNetworkReply::finished,this,[=](){

        if(reply->error() != QNetworkReply::NoError)
        {
            cout<<__FUNCTION__<<"获取服务器数据失败!";
            cout<<reply->errorString();
            //释放资源
            reply->deleteLater();
            return;
        }

        //获取数据
        QByteArray reData = reply->readAll();
        reply->deleteLater();


        //开始解析返回的json对应的code
        QJsonDocument doc = QJsonDocument::fromJson(reData);
        QString status;
        if(doc.isObject())
        {
            QJsonObject obj = doc.object();
            status = obj.value(QString("code")).toString();
        }
        cout<<status;

        /*
    上传文件：
        成功：{"code":"008"}
        失败：{"code":"009"}
        */

        if("008" == status)
        {
            cout<<fileName<<"上传完成！";
            //把这个返回码和对应的操作写入记录文档
            m_comm.writeRecord(login->getUserName(),fileName,status);

        }
        else if("009" == status)
        {
            cout<<fileName<<"上传失败!";
            //把这个返回码和对应的操作写入记录文档
            m_comm.writeRecord(login->getUserName(),fileName,status);
        }

        //获取任务列表，把这个操作的任务进行删除
        //获取任务实例
        UploadTask *uploadTask = UploadTask::getInstance();
        if(uploadTask == NULL)
        {
            cout<<__FUNCTION__<<"获取任务实例失败!";
            return;
        }

        //删除这个已经操作的实例对象
        uploadTask->dealUploadTask();

    });



}

//取出上传任务队列的第一个，上传成功完成后，再处理下一个任务
void MyFileWg::uploadFilesAction()
 {
     //获取上传任务的列表
     UploadTask *uploadList = UploadTask::getInstance();
     if(uploadList == NULL)
     {
         cout<<__FUNCTION__<<"获取实例失败!";
         return;
     }

     //如果队列为空，就终止任务
     if(uploadList->isEmpty())
     {
         cout<<__FUNCTION__<<"队列为空!";
         return;
     }

     //检测是否有任务正在上传
     if(uploadList->isUpload())
     {
//         QString filename = uploadList->getUploadTaskList().first()->fileName;
//         cout<<__FUNCTION__<<QString("%1正在上传").arg(filename);
         return;
     }

     //获取登录实例信息
     logininfoinstance *login = logininfoinstance::getInstance();
     if(login == NULL)
     {
         cout<<__FUNCTION__<<"获取登录实例失败!";
         return;
     }

     //http 发送的相关准备
     QNetworkRequest request ;
     QString url = QString("http://%1:%2/md5").arg(login->getIp()).arg(login->getPort());
     cout<<"准备要发送的url:"<<url;
     request.setUrl(QUrl(url));
     request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

     // 取出第0个上传任务，如果任务队列没有任务在上传，设置第0个任务上传
     UploadFileInfo *info = uploadList->takeTask();
     QByteArray data = setMd5Json(login->getUserName(),login->getToken(),info->md5,info->fileName);

     //发送post请求
     QNetworkReply *reply = m_manager->post(request,data);
     if(reply == NULL)
     {
         cout<<"获取replay对象失败!";
         return;
     }

     //接受服务器返回数据
     connect(reply,&QNetworkReply::finished,this,[=](){

         if(reply->error() != QNetworkReply::NoError)
         {
             cout<<__FUNCTION__<<reply->errorString();
             reply->deleteLater();
             return;
         }

         //返回的数据
         QByteArray reData = reply->readAll();
         //返回的数据
         cout<<reData.data();

         //判断返回的code码
         /*
        秒传文件：
            文件已存在：{"code":"005"}
            秒传成功：  {"code":"006"}
            秒传失败：  {"code":"007"}
        token验证失败：{"code":"111"}

         */

         QJsonDocument doc = QJsonDocument::fromJson(reData);
         QString status;
         if(doc.isObject())
         {
             QJsonObject obj = doc.object();
             status = obj.value(QString("code")).toString();
         }
         cout<<status;

         if("005" == status)
         {
             //要上传的文件已经存在,把这个记录写进文件
             writeRecord(login->getUserName(),info->fileName,status);
             //删除任务队列中对应的任务
             uploadList->dealUploadTask();
         }
         else if("006" == status){
             //要上传的文件已经存在,把这个记录写进文件
             writeRecord(login->getUserName(),info->fileName,status);
             //删除任务队列中对应的任务
             uploadList->dealUploadTask();
         }
         else if("007" == status){
             //说明服务器上真没有这个文件，需要真正的传输
             uploadFile(info);
         }
         else if("111" == status){
             QMessageBox::warning(this, "账户异常", "请重新登陆！！！");
             emit loginAgainSignal(); //发送重新登陆信号
             return;
         }
     });

 }


 //==========================>下载文件<================
// 添加需要下载的文件到下载任务列表
void MyFileWg::addDownloadFiles()
{
     //切换到下载进度的页面
//     emit gotoTransfer(TransferStatus::Download);
     //获取点击的item
     QListWidgetItem *item = ui->listWidget->currentItem();
     if(item == NULL)
     {
         cout<<"get item error!";
         return;
     }

     //获取下载列表
     DownloadTask *p  = DownloadTask::getInstance();
     if(p == NULL)
     {
         cout<<__FUNCTION__<<"get DownloadTask fail!";
         return;
     }

     //这个时候m_fileList里面已经存了许多文件的信息
     int size = m_fileList.size();
     for(int i = 0;i<size;++i)
     {

         if(m_fileList.at(i)->item == item) //匹配到选中的item
         {
             //获取保存文件的路径
             QString filePathName = QFileDialog::getSaveFileName(this,"选择保存文件的路径",m_fileList.at(i)->filename);
             if(filePathName.isEmpty())
             {
                 cout<<__FUNCTION__<<"filePathName.isEmpty()";
                 return;
             }
             //切换到下载进度的页面
             emit gotoTransfer(TransferStatus::Download);
             cout<<"选择的路径是:"<<filePathName;

             //追加任务到下载队列
             //参数：info：下载文件信息， filePathName：文件保存路径
             //成功：0
             //失败：
             //  -1: 下载的文件是否已经在下载队列中
             //  -2: 打开文件失败
             int ret = p->appendDownloadList(m_fileList.at(i),filePathName);
             if(ret == -1)
             {
                 QMessageBox::critical(this,"错误","下载的文件是否已经在下载队列中!");

             }
             else if(ret == -2)
             {
                 QMessageBox::critical(this,"错误","打开文件失败!");
                 m_comm.writeRecord(m_fileList.at(i)->user, m_fileList.at(i)->filename, "011"); //下载文件失败，记录

             }
             break; //中断退出
         }
     }
 }

//下载文件pv（下载量）字段处理
void MyFileWg::dealFilePv(QString md5, QString filename)
{
    QNetworkRequest request;

    //获取登录实例
    logininfoinstance *login = logininfoinstance::getInstance();

    //url
    QString url = QString("http://%1:%2/dealfile?cmd=pv").arg(login->getIp()).arg(login->getPort());
    //cout<<url;
    request.setUrl(QUrl(url));

    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");


    //打包发送的json数据
    /*
    {
        "user": "yoyo",
        "token": "xxxx",
        "md5": "xxx",
        "filename": "xxx"
    }
    */
    //构造发送的json数据
    QMap<QString,QVariant> jsonData;
    jsonData.insert("user",QVariant(login->getUserName()));
    jsonData.insert("token",QVariant(login->getToken()));
    jsonData.insert("md5",QVariant(md5));
    jsonData.insert("filename",QVariant(filename));

    QJsonDocument doc = QJsonDocument::fromVariant(jsonData);
    QByteArray data = doc.toJson();

    //cout<<"要发送的数据"<<data.data();
    //post发送
    QNetworkReply *reply = m_manager->post(request,data);

    connect(reply,&QNetworkReply::finished,this,[=](){
        if(reply->error() != QNetworkReply::NoError)
        {
            cout<<__FUNCTION__<<reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray redata = reply->readAll();
        reply->deleteLater();

        /*

        下载文件pv字段处理
            成功：{"code":"016"}
            失败：{"code":"017"}


        token验证失败：{"code":"111"}

        */
            //获取返回的code
        QJsonParseError error;
        QString code;
        QJsonDocument doc = QJsonDocument::fromJson(redata,&error);
        if(error.error == QJsonParseError::NoError)
        {
            if(doc.isNull() || doc.isEmpty())
            {
                cout<<__FUNCTION__<<"isNull or isEmpty!";
                return ;
            }
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                code = obj.value("code").toString();
            }

        }
        else{
            cout<<error.errorString();
            return;
        }

        if(code == "016")
        {
            //该文件pv字段+1
            for(int i = 0; i < m_fileList.size(); ++i)
            {
                FileInfo *info = m_fileList.at(i);
                if( info->md5 == md5 && info->filename == filename)
                {
                    int pv = info->pv;
                    info->pv = pv+1;
                    cout<<filename<<pv;
                    break; //很重要的中断条件
                }
            }
        }
        else if(code =="017")
        {
            cout << "下载文件pv字段处理失败";
        }
        else if(code == "111")
        {
            QMessageBox::critical(this,"错误","账户异常，重新登录!!!");
            emit loginAgainSignal(); //发送重新登陆信号
            return ;
        }
    });

}

//真正的下载文件操作,把添加到下载列表里面的任务进行处理
void MyFileWg::downloadFilesAction()
{
    //取出任务队列
    DownloadTask *p = DownloadTask::getInstance();
    if( p == NULL )
    {
        cout<<__FUNCTION__<<"downloadFilesAction error";
        return;
    }

    //如果队列为空
    if(p->isEmpty())
    {
        cout<<__FUNCTION__<<"isEmpty";
        return;
    }
    //有任务正在下载，一次性只能下载一个任务
    if(p->isDownload())
    {
        return;
    }

    //看是否是共享文件下载任务，不是才能往下执行, 如果是，则中断程序
    //判断第一个任务是否为共享的任务
    //共享资源下载就终止
    if(p->isShareTask() == true)
    {
        return;
    }


    //取出下载任务
    DownloadInfo *tmp = p->takeTask(); //取下载任务

    //拿到对应信息
    QUrl url = tmp->url;
    QFile *file = tmp->file;
    QString md5 = tmp->md5;
    QString user = tmp->user;
    QString filename = tmp->filename;
    DataProgress *dp = tmp->dp;

    //发送get请求，活得文件内容
    QNetworkReply *reply = m_manager->get(QNetworkRequest(url));
    if(reply == NULL)
    {
        //删除任务
        p->dealDownloadTask();
        cout<<__FUNCTION__<<"get web data error!";
        //reply->deleteLater();//已经为NULL，不需要再去deleteLater()
        return;
    }

    //获取请求的数据完成时，就会发送信号SIGNAL(finished())
    connect(reply,&QNetworkReply::finished,this,[=](){

        cout<<"下载完成!";
        reply->deleteLater();

        p->dealDownloadTask(); //删除下载任务
        //"010" 代表下载成功

        m_comm.writeRecord(user, filename, "010"); //下载文件成功，记录

         //下载文件pv（下载量）字段处理
        dealFilePv(md5, filename); //下载文件pv字段处理
    });

    //当有可用数据时，reply 就会发出readyRead()信号，我们这时就可以将可用的数据保存下来
    connect(reply,&QNetworkReply::readyRead,this,[=](){

        if(file != NULL)
        {
            file->write(reply->readAll());
        }

    });

    //有可用数据更新时
    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesRead, qint64 totalBytes){
        dp->setProgress(bytesRead, totalBytes);//设置进度
    });

}

//==========================>文件item展示<================

// 清空文件列表
void MyFileWg::clearFileList()
{

    //cout<<__FUNCTION__<<"m_fileList.size()"<<m_fileList.size();
    int n = m_fileList.size();//!!!千万不要直接把这个当作循环的变量
    for(int i = 0;i< n;++i)
    {
        //cout<<"i = "<<i<<"\tn = "<<n;
        FileInfo *tmp = m_fileList.takeFirst();//takeFirst 会改变m_fileList的大小  ！！！！哭
        delete tmp;
    }
}

// 清空所有item项目
void MyFileWg::clearItems()
{
    //使用QListWidget::count()来统计ListWidget中总共的item数目

    int n = ui->listWidget->count();
    for(int i = 0;i<n;++i)
    {
        QListWidgetItem *tmp = ui->listWidget->takeItem(0);
        delete tmp;
    }
}

// 文件item展示
void MyFileWg::refreshFileItems()
{
    //清空所有item项目
    clearItems();
    //如果文件列表不为空，显示文件列表
    if(m_fileList.isEmpty() == false)
    {
        int n = m_fileList.size(); //元素个数
        for(int i = 0; i < n; ++i)
        {
            FileInfo *tmp = m_fileList.at(i);
            QListWidgetItem *item = tmp->item;
            //list widget add item
            ui->listWidget->addItem(item);
        }
    }

    //添加上传文件item
    this->addUploadItem();
}

// 添加“上传文件item”（图标）
void MyFileWg::addUploadItem(QString iconPath,QString name)
{
    ui->listWidget->addItem(new QListWidgetItem(QIcon(iconPath),name));
}

//==========================>刷新和按照下载量刷新<================
//拿到用户的文件数量
//有文件调用  getUserFilesList(cmd);
//没有文件调用 refreshFileItems();
void MyFileWg::refreshFiles(MyFileWg::Display cmd)
{

    //先拿到用户的文件个数
    m_userFilesCount = 0;

    //发送http请求
    //127.0.0.1:80/myfiles&cmd=count		//获取用户文件个数

    //获取用户登录实例
    logininfoinstance *login = logininfoinstance::getInstance();

    QString url = QString("http://%1:%2/myfiles?cmd=count").arg(login->getIp()).arg(login->getPort());

    QNetworkRequest request;

    request.setUrl(QUrl(url));//设置URL
    //设置请求头信息
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    //打包请求数据为json
    QByteArray data = setGetCountJson(login->getUserName(),login->getToken());

    //post发送数据
    QNetworkReply *replay = m_manager->post(request,data);

    connect(replay,&QNetworkReply::finished,this,[=](){

        if(replay->error() != QNetworkReply::NoError)
        {
            cout<<replay->errorString();
            replay->deleteLater();
            return;
        }

        QByteArray reData = replay->readAll();

        //cout<<"接收到的文件内容为:"<<reData.data();
        replay->deleteLater();

        //转化json ,获取服务器返回的数据
        QString code;
        QString num;
        QJsonParseError error;
        QJsonDocument doc  =   QJsonDocument::fromJson(reData,&error);
        if(error.error == QJsonParseError::NoError)
        {
            if(doc.isNull() || doc.isEmpty())
            {
                cout<<"doc isNull or isEmpty";
                return ;
            }

            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                //注意变量的生命周期，不要多次定义变量
                code = obj.value("code").toString();
                num = obj.value("num").toString();
            }
        }
        else{
            cout<<error.errorString();
            return;
        }

        // token验证失败
        if( code == "111" )
        {
            QMessageBox::warning(this, "账户异常", "请重新登陆！！！");

            emit loginAgainSignal(); //发送重新登陆信号

            return; //中断
        }

        //获取文件个数
        m_userFilesCount = num.toULong();
        cout<<"用户拥有的文件个数:"<<m_userFilesCount;

        clearFileList();//清空原有的文件列表

        //开始请求文件信息
        if(m_userFilesCount > 0)
        {
            // 说明用户有文件，获取用户文件列表
            m_start = 0;  //从0开始
            m_count = 10; //每次请求10个

            // 获取新的文件列表信息
            getUserFilesList(cmd);
        }
        else //没有文件
        {
            refreshFileItems(); //更新item
        }
    });

}
// 获取用户文件数量
QByteArray MyFileWg::setGetCountJson(QString user, QString token)
{
    /*json数据如下
    {
        user:xxxx
        token: xxxx
    }
    */
    QMap<QString,QVariant> info;
    info.insert("user",QVariant(user));
    info.insert("token",QVariant(token));

    QJsonDocument doc = QJsonDocument::fromVariant(info);

    QByteArray data = doc.toJson();

    return data;
}


// 获取用户文件列表
// cmd取值，Normal：普通用户列表，PvAsc：按下载量升序， PvDesc：按下载量降序
void MyFileWg::getUserFilesList(MyFileWg::Display cmd)
{
    cout<<"m_userFilesCount = "<<m_userFilesCount;

    //cout<<"cmd="<<cmd;
    if(m_userFilesCount <= 0)
    {
        cout<<"请求用户文件列表结束!";
        refreshFileItems();//更新item

        return;
    }
    else if(m_count > m_userFilesCount){ //当用户拥有的文件数小于m_count
        m_count = m_userFilesCount;
    }


    //获取登录实例
    logininfoinstance *login = logininfoinstance::getInstance();

    //开始请求
    QNetworkRequest request;
    // 获取用户文件信息 127.0.0.1:80/myfiles&cmd=normal
    // 按下载量升序 127.0.0.1:80/myfiles?cmd=pvasc
    // 按下载量降序127.0.0.1:80/myfiles?cmd=pvdesc
    QString url,cmdType;
    //cmd取值，Normal：普通用户列表，PvAsc：按下载量升序， PvDesc：按下载量降序
    if(cmd == Normal)
    {
        cmdType = "Normal";
    }
    else if(cmd == PvAsc)
    {
        cmdType = "PvAsc";
    }
    else if(cmd == PvDesc)
    {
        cmdType = "PvDesc";
    }

    url = QString("http://%1:%2/myfiles?cmd=%3").arg(login->getIp()).arg(login->getPort()).arg(cmdType);

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    /*
    {
        "user": "yoyo"
        "token": "xxxx"
        "start": 0
        "count": 10
    }
    */
   //------------打包数据为json
    QMap<QString,QVariant> info;
    info.insert("user",QVariant(login->getUserName()));
    info.insert("token",QVariant(login->getToken()));
    info.insert("start",QVariant(m_start));
    info.insert("count",QVariant(m_count));

    QJsonDocument doc = QJsonDocument::fromVariant(info);

    QByteArray data = doc.toJson();

    //cout<<"准备要获取文件列表信息时要发送的请求:"<<data;

    //改变文件起点位置
    m_start += m_count;
    m_userFilesCount -=m_count;

    cout<<"m_start="<<m_start;
    //post 发送请求
    QNetworkReply *reply = m_manager->post(request,data);

    connect(reply,&QNetworkReply::finished,this,[=](){
        if(reply->error() != QNetworkReply::NoError)
        {
            cout<<reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray redata = reply->readAll();
       // cout<<redata.data();

        reply->deleteLater();

        //解析json
        QJsonParseError error;
        QString code;
        QJsonDocument doc = QJsonDocument::fromJson(redata,&error);

        if(error.error == QJsonParseError::NoError)
        {
            if(doc.isNull() || doc.isEmpty())
            {
                cout<<error.errorString();
                return;
            }
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                code = obj.value("code").toString();
            }
        }
        else{
            cout<<error.errorString();
            return;
        }

        //cout<<"code="<<code;
        if("111" == code)//失败就发送 {"code":"111"}
        {
            QMessageBox::critical(this,"账户异常","请重新登录");
            emit loginAgainSignal(); //发送重新登陆信号
            return;
        }

        // 015 是错误
        if("015" != code){
           getFileJsonInfo(redata);//解析文件列表json信息，存放在文件列表中

           //继续获取用户文件列表
           getUserFilesList(cmd);//递归调用，获取
        }
    });
}
// 根据对应的cmd,取得对应的json data,解析服务器返回的json数据
void MyFileWg::getFileJsonInfo(QByteArray data)
{
    //cout<<__FUNCTION__<<"ininin";
    //每一个文件信息就是一个json数组，整个就是一个大json对象
    /*
    {
    "user": "yoyo",
    "md5": "e8ea6031b779ac26c319ddf949ad9d8d",
    "time": "2017-02-26 21:35:25",
    "filename": "test.mp4",
    "share_status": 0,
    "pv": 0,
    "url": "http://192.168.31.109:80/group1/M00/00/00/wKgfbViy2Z2AJ-FTAaM3As-g3Z0782.mp4",
    "size": 27473666,
     "type": "mp4"
    }
    */
    //-- user	文件所属用户
    //-- md5 文件md5
    //-- createtime 文件创建时间
    //-- filename 文件名字
    //-- shared_status 共享状态, 0为没有共享， 1为共享
    //-- pv 文件下载量，默认值为0，下载一次加1
    //-- url 文件url
    //-- size 文件大小, 以字节为单位
    //-- type 文件类型： png, zip, mp4……
    /*
                    //文件信息
                    struct FileInfo
                    {
                        QString md5;        //文件md5码
                        QString filename;   //文件名字
                        QString user;       //用户
                        QString time;       //上传时间
                        QString url;        //url
                        QString type;       //文件类型
                        qint64 size;        //文件大小
                        int shareStatus;    //是否共享, 1共享， 0不共享
                        int pv;             //下载量
                        QListWidgetItem *item; //list widget 的item
                    };

    {
        "user": "sungs",
        "md5": "d6a053d73678cc2974d73190bd5f714d",
        "time": "2021-07-31 17:51:56",
        "filename": "IMG_7590 2.JPG",
        "share_status": 0,
        "pv": 0,
        "url": "http://10.211.55.11:80/group1/M00/00/00/CtM3C2EFHTyAO9mxABHuTEj56Lg294.JPG",
        "size": 0,
        "type": "JPG"
    }

        */
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError)
    {
        if(doc.isNull() || doc.isEmpty())
        {
            cout<<error.errorString();
            return;
        }

        if(doc.isObject())
        {
            QJsonObject obj = doc.object();//最外层的{}
            //QJsonArray json数组，描述json数据中[]括起来部分
            QJsonArray array = obj.value("files").toArray();

            //数组的个数
            int size = array.size();
            cout<<"size"<<size;
            for(int i = 0;i<size;++i)
            {
                QJsonObject tmp = array[i].toObject();



                FileInfo *info = new FileInfo;

                info->user = tmp.value("user").toString();
                info->md5 = tmp.value("md5").toString();
                info->time = tmp.value("time").toString();
                info->filename = tmp.value("filename").toString();
                info->shareStatus = tmp.value("share_status").toInt();
                info->pv = tmp.value("pv").toInt();
                info->url = tmp.value("url").toString();
                info->size = tmp.value("size").toInt();
                info->type = tmp.value("type").toString();


                //为了构造出来item
                QString type = QString(info->type).toLower() +".png";

                //cout<<type;
                QString filetype = QString(m_comm.getFileType(type));
                //cout<<"filetype"<<filetype;

                info->item = new QListWidgetItem(QIcon(filetype),info->filename);

                //list添加节点
                //cout<<__FUNCTION__<<"m_fileList.size="<<m_fileList.size();
                //这里的m_fileList 的size()决定了文件item的个数
                //服务器返回的数据也可以在这里面找
                m_fileList.append(info);
            }
        }
    }
}

//==========================>分享。删除。属性<================
//分享，删除，文件属性统一接口
// 处理选中的文件
void MyFileWg::dealSelectdFile(QString cmd)
{
    //获取当前选中的item
    QListWidgetItem *item = ui->listWidget->currentItem();
    if(item == NULL)
    {
        return;
    }

    //查找文件列表相匹配的item
    for(int i = 0;i<m_fileList.size();++i)
    {
        if(m_fileList.at(i)->item == item)//遍历到选中的item
        {
            if(cmd == "分享")
            {
                shareFile( m_fileList.at(i) ); //分享某个文件
            }
            else if(cmd =="删除"){
                delFile( m_fileList.at(i) ); //删除某个文件
            }
            else if(cmd == "属性"){
                getFileProperty( m_fileList.at(i) ); //获取属性信息
            }

            break;
        }
    }
}

//分享某个文件
void MyFileWg::shareFile(FileInfo *info)
{
    if(info->shareStatus == 1)//1 已经共享，不再共享
    {
        QMessageBox::warning(this,"提醒","该文件已经共享!");
        return;
    }

    QNetworkRequest request;

    //获取登录实例
    logininfoinstance *login = logininfoinstance::getInstance();

    //设置请求头
    QString url = QString("http://%1:%2/dealfile?cmd=share").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    /*
    {
        "user": "yoyo",
        "token": "xxxx",
        "md5": "xxx",
        "filename": "xxx"
    }
    */

    //构造发送的json数据
    QMap<QString,QVariant> jsonData;
    jsonData.insert("user",QVariant(login->getUserName()));
    jsonData.insert("token",QVariant(login->getToken()));
    jsonData.insert("md5",QVariant(info->md5));
    jsonData.insert("filename",QVariant(info->filename));

    QJsonDocument doc = QJsonDocument::fromVariant(jsonData);
    QByteArray data = doc.toJson();

    //post发送
    QNetworkReply *reply = m_manager->post(request,data);

    connect(reply,&QNetworkReply::finished,this,[=](){
        if(reply->error() != QNetworkReply::NoError)
        {
            cout<<__FUNCTION__<<"post error!";
            reply->deleteLater();
            return;
        }

        QByteArray redata = reply->readAll();
        reply->deleteLater();

        /*
            分享文件：
                成功：{"code":"010"}
                失败：{"code":"011"}
                别人已经分享此文件：{"code", "012"}

        token验证失败：{"code":"111"}

            */
            //获取返回的code
            QJsonParseError error;
        QString code;
        QJsonDocument doc = QJsonDocument::fromJson(redata,&error);
        if(error.error == QJsonParseError::NoError)
        {
            if(doc.isNull() || doc.isEmpty())
            {
                cout<<__FUNCTION__<<"isNull or isEmpty!";
                return ;
            }
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                code = obj.value("code").toString();
            }

        }
        else{
            cout<<error.errorString();
            return;
        }

        if(code == "010")
        {
            //分享成功，修改文件的状态
            info->shareStatus = 1;
            QMessageBox::information(this,"通知",QString("%1已被成功分享!").arg(info->filename));
        }
        else if(code =="011")
        {
            QMessageBox::critical(this,"错误",QString("%1分享失败!").arg(info->filename));
        }
        else if(code == "012")
        {
            QMessageBox::warning(this,"警告",QString("%1已被别人分享过了!").arg(info->filename));
        }
        else if(code == "111")
        {
            QMessageBox::critical(this,"错误","账户异常，重新登录!!!");
            emit loginAgainSignal(); //发送重新登陆信号
            return ;
        }
    });
}

//删除某个文件
void MyFileWg::delFile(FileInfo *info)
{
    //初始化请求对象
    QNetworkRequest request;

    //获取登录实例
    logininfoinstance *login = logininfoinstance::getInstance();

    //初始化请求头信息
    QString url = QString("http://%1:%2/dealfile?cmd=del").arg(login->getIp()).arg(login->getPort());

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    /*
    {
        "user": "yoyo",
        "token": "xxxx",
        "md5": "xxx",
        "filename": "xxx"
    }
    */
    //构造发送的json数据
    QMap<QString,QVariant> jsonData;
    jsonData.insert("user",QVariant(login->getUserName()));
    jsonData.insert("token",QVariant(login->getToken()));
    jsonData.insert("md5",QVariant(info->md5));
    jsonData.insert("filename",QVariant(info->filename));

    QJsonDocument doc = QJsonDocument::fromVariant(jsonData);
    QByteArray data = doc.toJson();

    //post发送
    QNetworkReply *reply = m_manager->post(request,data);

    connect(reply,&QNetworkReply::finished,this,[=](){
        if(reply->error() != QNetworkReply::NoError)
        {
            cout<<__FUNCTION__<<"post error!";
            reply->deleteLater();
            return;
        }

        QByteArray redata = reply->readAll();
        reply->deleteLater();

        /*
            删除文件：
                成功：{"code":"013"}
                失败：{"code":"014"}

        token验证失败：{"code":"111"}

            */
            //获取返回的code
            QJsonParseError error;
        QString code;
        QJsonDocument doc = QJsonDocument::fromJson(redata,&error);
        if(error.error == QJsonParseError::NoError)
        {
            if(doc.isNull() || doc.isEmpty())
            {
                cout<<__FUNCTION__<<"isNull or isEmpty!";
                return ;
            }
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                code = obj.value("code").toString();
            }

        }
        else{
            cout<<error.errorString();
            return;
        }

        if(code == "013")
        {
            //删除成功
            QMessageBox::information(this,"通知",QString("[%1]已被成功删除!").arg(info->filename));
            //从文件列表移除该文件
            //移除该item
            int size = m_fileList.size();
            for(int i = 0;i<size;++i)
            {
                if(m_fileList.at(i) == info)
                {
                    QListWidgetItem *item = info->item;
                    //从列表视图移除此item
                    ui->listWidget->removeItemWidget(item);
                    delete item;
                    //从文件列表移除该文件
                    m_fileList.removeAt(i);
                    delete info;
                    break; //很重要的中断条件
                    //刷新用户的文件列表
                    refreshFiles();
                }
            }
        }
        else if(code =="014")
        {
            QMessageBox::critical(this,"错误",QString("%1删除失败!").arg(info->filename));
        }
        else if(code == "111")
        {
            QMessageBox::critical(this,"错误","账户异常，重新登录!!!");
            emit loginAgainSignal(); //发送重新登陆信号
            return ;
        }
    });





}

//获取属性信息
void MyFileWg::getFileProperty(FileInfo *info)
{
    FilePropertyInfo dg;
    dg.setInfo(info);
    dg.exec();//模态方式运行
}


//==================>任务队列定时处理接口<================
//定时检查任务队列，包括上传的任务队列和下载的任务队列
void MyFileWg::checkTaskList()
{
    //定时检查上传任务，每500ms
    connect(&m_uploadFileTimer,&QTimer::timeout,this,[=](){
        //上传文件处理，取出上传任务列表的队首任务，上传完后，再取下一个任务
        uploadFilesAction();
    });

    //每500ms检查一下上传的任务列表
    //一次只能上传一个任务
    m_uploadFileTimer.start(500);


    //==========》下载的任务队列的检测《==============
    // 定时检查下载队列是否有任务需要下载
    connect(&m_downloadTimer, &QTimer::timeout, [=]()
            {
                // 上传文件处理，取出上传任务列表的队首任务，上传完后，再取下一个任务
                downloadFilesAction();
            });

    // 启动定时器，500毫秒间隔
    // 每个500毫秒，检测下载任务，每一次只能下载一个文件
    m_downloadTimer.start(500);


}


//清空队列任务
void MyFileWg::clearAllTask()
{
    //获取上传列表实例
    UploadTask *uploadList = UploadTask::getInstance();
    if(uploadList == NULL)
    {
        cout << "UploadTask::getInstance() == NULL";
        return;
    }

    uploadList->clearList();

    //获取下载列表实例
    DownloadTask *p = DownloadTask::getInstance();
    if(p == NULL)
    {
        cout << "DownloadTask::getInstance() == NULL";
        return;
    }

    p->clearList();
}











