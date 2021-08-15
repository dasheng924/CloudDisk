#include "login.h"
#include "ui_login.h"
#include "form.h"
#include "common/logininfoinstance.h"


#include <QPainter>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QCryptographicHash>

//HTTP相关的类
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

QNetworkAccessManager* Login::m_manager = new QNetworkAccessManager;
Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    // 此处无需指定父窗口
    m_window = new MainWindow;

    //去边框
    this->setWindowFlags(Qt::FramelessWindowHint|windowFlags());

    // 设置当前窗口的字体信息
    this->setFont(QFont(".AppleSystemUIFont", 12, QFont::Bold, false));
    //设置栈窗口的默认显示
    ui->stackedWidget->setCurrentIndex(0);
    ui->login_username->setFocus();


    //右上角按钮的实现，通过连接form发出的信号，实现对应的功能
    //0 login
    //1 reg
    //2 set

    //----------------------设置界面---------------------
    //实现设置按钮的页面切换
    connect(ui->titleWg,&Form::showSetting,this,[=](){
        ui->stackedWidget->setCurrentIndex(2);
        //服务器设置界面，软件启动后根据配置文件自动设置
        //自动生成配置文件
        createWebConf(CONF_PATH);
        //把配置文件的信息读取出来，用来初始化设置界面
        int ret = initWebSetting();
        if(ret != 0)
        {
            QMessageBox::critical(this,"错误","初始化服务器设置界面失败!");
        }
    });


    //---------------------关闭按钮------------------------
    //实现关闭按钮
    connect(ui->titleWg,&Form::closeWidget,this,[=](){
        if(ui->stackedWidget->currentIndex() == 1 ){

            //清空注册页面的信息
            ui->reg_username->clear();
            ui->reg_nick->clear();
            ui->reg_passwd->clear();
            ui->reg_repasswd->clear();
            ui->reg_phone->clear();
            ui->reg_email->clear();

            //切换窗口
            ui->stackedWidget->setCurrentIndex(0);
            //login_username 获得焦点
            ui->login_username->setFocus();


        }
        else if(ui->stackedWidget->currentIndex() == 2 ){

            ui->set_ip->clear();
            ui->set_port->clear();

            ui->stackedWidget->setCurrentIndex(0);
            ui->login_username->setFocus();
        }
        else{
            this->close();
        }
    });

    //---------------------登录界面-------------------------
    //登录页面的没有账户，立即注册
    connect(ui->goLogin,&QToolButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(1);
    });
    ui->login_passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    //---------------------注册页面------------------------
    //设置注册页面。密码填写栏的模式
    ui->reg_passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->reg_repasswd->setEchoMode(QLineEdit::PasswordEchoOnEdit);


    // 数据的格式提示
    ui->login_username->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 3~16");
    ui->reg_username->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 3~16");
    ui->reg_nick->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 3~16");
    ui->login_passwd->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");
    ui->reg_passwd->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");
    ui->reg_repasswd->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");

    //记住密码，默认登录
    defaultlogin();

    // 加载图片信息 - 显示文件列表的时候用，在此初始化
    m_comm.getFileTypeList();

    // 切换用户 - 重新登录
    connect(m_window, &MainWindow::changeUser, [=](){
                m_window->hide();
                this->show();
    });

}

Login::~Login()
{
    delete ui;
}



//绘制背景图
void Login::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPixmap pixmap("://images/login_bk.jpg");
    //初始化画家
    QPainter p(this); //参数绘图设备
    p.drawPixmap(0,0,width(),height(),pixmap);
}

//设置服务器信息
void Login::on_set_ok_clicked()
{
    //获取控件数据
    QString ip = ui->set_ip->text();
    QString port = ui->set_port->text();
    //正则表达式校验
    QRegExp ex(IP_REG);
    if(!ex.exactMatch(ip))
    {
        QMessageBox::critical(this,"输入IP错误","请输入正确的IP");
        //清空IP输入栏
        ui->set_ip->clear();
        //设置焦点
        ui->set_ip->setFocus();
        return;
    }
    ex.setPattern(PORT_REG);
    if(!ex.exactMatch(port))
    {
        QMessageBox::critical(this,"输入PORT错误","请输入正确的PORT");
        //清空IP输入栏
        ui->set_port->clear();
        //设置焦点
        ui->set_port->setFocus();
        return;
    }
    //把信息保存到配置文件
    int ret = saveWebConf(ip,port,CONF_PATH);
    if(ret == 0)
    {
        QMessageBox::information(this,"提示","设置服务器信息成功!");
    }
    else if(ret == -1)
    {
        QMessageBox::critical(this,"错误","打开配置文件失败!");
    }
    else if(ret == -2)
    {
        QMessageBox::critical(this,"错误","读取JSON文件失败!");
    }

}

//要先读取原来的配置文件，拿到数据后，替换掉原来的，生成新的
int Login::saveWebConf(QString ip, QString port, QString path)
{
    //1.先读文件path
    QFile file(path);
    bool ret = file.open(QIODevice::ReadOnly);
    if(!ret)
    {
        QMessageBox::critical(this,"错误","配置打开错误!");
    }

#if 0
    //第一次是没有这个文件的
    bool isFirst = false;
    if(ret == false)
    {
        isFirst = true;
        //文件不存在
        file.close();

        //创建文件
        ret = createWebConf(path);
        if(static_cast<int>(ret) == -1)
        {
            QMessageBox::critical(this,"错误","配置文件不存在，创建时错误!");
        }
        else if(static_cast<int>(ret) == 0)
        {
            //文件创建完毕，继续读取文件，进而修改文件
            QMessageBox::information(this,"Good","第一次执行软件@^@");
        }
    }

    if(isFirst)
        file.open(QIODevice::ReadOnly);
#endif

    QByteArray data = file.readAll();
    //读完信息，关闭文件
    file.close();

    //先读取原来的配置文件
    QJsonDocument doc  = QJsonDocument::fromJson(data);
    if(!doc.isObject())
    {
        //不是一个对象
        return -2;
    }
    //是一个对象,做如下的操作
    QJsonObject obj = doc.object();
    //取出login信息
    QMap<QString,QVariant>loginInfo;
    QJsonValue login_value = obj.value("login");
    if(login_value.isObject())
    {
        QJsonObject sub = login_value.toObject();
        QString old_pwd = sub.value("pwd").toString();
        QString old_remember = sub.value("remember").toString();
        QString old_user = sub.value("user").toString();
        //取出值后，保存信息
        loginInfo.insert("pwd",QVariant(old_pwd));
        loginInfo.insert("remember",QVariant(old_remember));
        loginInfo.insert("user",QVariant(old_user));
    }
    //取出type_path信息
    QMap<QString,QVariant>type_path_Info;
    QJsonValue type_path_value = obj.value("type_path");
    if(type_path_value.isObject())
    {
        QJsonObject sub = type_path_value.toObject();
        QString old_path = sub.value("path").toString();

        type_path_Info.insert("path",QVariant(old_path));
    }

    //取出web_server信息
    QMap<QString,QVariant>web_server_info;
    QJsonValue web_server_value = obj.value("web_server");
    if(web_server_value.isObject())
    {
        QJsonObject sub = web_server_value.toObject();
        QString old_ip = sub.value("ip").toString();
        QString old_port = sub.value("port").toString();

        //ip 和port 设置成最新指定的值
        web_server_info.insert("ip",ip);
        web_server_info.insert("port",port);

    }


    //把三部分信息合成一部分
    QMap<QString,QVariant>all_info;
    all_info.insert("login",loginInfo);
    all_info.insert("type_path",type_path_Info);
    all_info.insert("web_server",web_server_info);

    //将组合后的信息写入文件
    doc = QJsonDocument::fromVariant(QVariant(all_info));

    //QJsonObject--->QString
    data = doc.toJson();  //返回值是QByteArray
    if(!file.open(QIODevice::WriteOnly))
    {
        return -1 ;
    }
    file.write(data);
    file.close();
    return 0;
}

//第一次没配置文件的默认生成动作
int Login::createWebConf(QString path)
{

    QJsonObject obj;

    QJsonObject sub1,sub2,sub3;
    sub1.insert("pwd",QJsonValue("wqq2b4Ild/w="));
    sub1.insert("remember",QJsonValue("yes"));
    sub1.insert("user",QJsonValue("Mi/CvL0kLkQ="));

    sub2.insert("path",QJsonValue(TYPE_PATH));

    //自动生成的IP和PORT
    sub3.insert("ip",QJsonValue("10.211.55.11"));
    sub3.insert("port",QJsonValue("80"));

    obj.insert("login",QJsonValue(sub1));
    obj.insert("type_path",QJsonValue(sub2));
    obj.insert("web_server",QJsonValue(sub3));

    //转化为QString
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    //写入文件
    QFile file(path);
    int ret = file.open(QIODevice::WriteOnly);
    if(!ret)
    {
        return -1;
    }
    file.write(data);
    file.close();
    return 0;
}
//根据配置文件初始化服务器设置界面
int Login::initWebSetting()
{
    QString ip;
    QString port;
    int ret = getIpAndPort(CONF_PATH,&ip,&port) ;
    if(ret == -1)
    {
        QMessageBox::critical(this,"错误","initWebSetting 初始化服务器界面失败!");
        qDebug()<<"----"<<ip<<port;
        return -1;
    }

    ui->set_ip->setText(ip);
    ui->set_port->setText(port);
    return 0;
}

int Login::getIpAndPort(const QString path,QString *ip,QString *port)
{
    QFile file(path);
    bool ret = file.open(QIODevice::ReadOnly);
    if(!ret)
    {
        QMessageBox::critical(this,"错误","getIpAndPort open初始化服务器设置界面失败!");
        return -1;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError)
    {
        if(doc.isNull() || doc.isEmpty())
        {
            QMessageBox::critical(this,"错误","getIpAndPort isEmpty转换错误!");
            return -1;
        }
        if(doc.isObject())
        {
            QJsonObject obj = doc.object();
            QJsonValue value = obj.value(QString("web_server"));
            if(value.isObject())
            {
                QJsonObject sub = value.toObject();
                *ip = sub.value("ip").toString();
                *port = sub.value("port").toString();
                //qDebug()<<*ip<<*port;
            }

        }
    }
    else
    {
        QMessageBox::critical(this,"错误","initWebSetting JSON 转换错误!");
        return -1;
    }
    return 0;
}


//注页面获取信息，打包成JSON，反馈出来
QByteArray Login::getRegJson(QString username, QString nick, QString passwd, QString phone, QString email)
{
    QJsonObject obj;
    obj.insert("userName",QJsonValue(username));
    obj.insert("nickName",QJsonValue(nick));
    obj.insert("firstPwd",QJsonValue(passwd));
    obj.insert("phone",QJsonValue(phone));
    obj.insert("email",QJsonValue(email));

    QJsonDocument doc(obj);

    QByteArray data = doc.toJson();

#if 1
    //验证发给服务器的json是正确的
    QFile file("/Users/sunguosheng/Code/Yun/Pan/regJsonCheck.json");
    bool ret=file.open(QIODevice::ReadWrite);
    if(!ret)
    {
        QMessageBox::critical(this,"验证JSON","发给服务器的json是错误的!");
        return QByteArray();
    }
    file.write(data);
    file.close();
#endif

    return data;
}

QNetworkAccessManager *Login::getNetManager()
{
    return m_manager;
}

QString Login::getRegWebCode(QByteArray jsonData)
{
        /*
         * {"code":"002"} 成功
         * {"code":"003"} 用户已存在
         * {"code":"004"} 失败
         *
         */
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        QString status;
        if(doc.isObject())
        {
            QJsonObject obj = doc.object();
            status = obj.value(QString("code")).toString();
        }
        qDebug()<<status;
        return status;
}

int Login::getLoginWebCode(QByteArray data, QString *code, QString *token)
{
    QString tmp_code;
    QString tmp_token;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError)
    {
        if(doc.isNull() || doc.isEmpty())
        {
            QMessageBox::critical(this,"错误","login json error");
            return -1;
        }
        if(doc.isObject())
        {
            QJsonObject obj = doc.object();
            tmp_code = obj.value("code").toString();
            tmp_token = obj.value("token").toString();
            *code = tmp_code;
            *token = tmp_token;
        }
    }
    else{
        qDebug()<<error.errorString();
        return -1;
    }
    return 0;
}

//base64加密
QString Login::encryptBase64(QString str)
{
    QByteArray ret = str.toUtf8().toBase64(QByteArray::Base64Encoding);
    return QString(ret);
}
//base64解密
QString Login::decodeBase64(QString str)
{
    QByteArray ret = str.toUtf8().fromBase64(str.toUtf8());
    return QString(ret);
}

void Login::loginConvertJson(QString username, QString passwd, bool remember, QString ip, QString port)
{
    QMap<QString,QVariant> web_server_info;
    web_server_info.insert("ip",QVariant(ip));
    web_server_info.insert("port",QVariant(port));

    QMap<QString,QVariant> login_info;
    login_info.insert("pwd",QVariant(passwd));
    QString real_remember = (remember?"yes":"no");
    login_info.insert("remember",QVariant(real_remember));
    login_info.insert("user",QVariant(username));

    QMap<QString,QVariant> type_path_info;
    type_path_info.insert("path",QVariant(TYPE_PATH));

    QMap<QString,QVariant> all_info;
    all_info.insert("login",QVariant(login_info));
    all_info.insert("type_path",QVariant(type_path_info));
    all_info.insert("web_server",QVariant(web_server_info));

    QJsonDocument doc = QJsonDocument::fromVariant(QVariant(all_info));

    QByteArray data = doc.toJson();

    QFile file(CONF_PATH);
    bool ret = file.open(QIODevice::WriteOnly);
    if(!ret)
    {
        QMessageBox::critical(this,"错误","打开配置文件失败!");
    }
    file.write(data);
    file.close();
}

QByteArray Login::getLoginJson(QString username, QString passwd)
{
    QJsonObject obj;
    obj.insert("user",QJsonValue(username));
    obj.insert("pwd",QJsonValue(passwd));

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();
    return data;
}

QString Login::md5(QString str)
{
    QByteArray ret = QCryptographicHash::hash(str.toUtf8(),QCryptographicHash::Md5);
    return QString(ret.toHex());
}

//记住密码，默认登录
void Login::defaultlogin()
{
    //读取配置文件的用户名，密码，是否记住密码
    QString username,passwd,remember;
    QFile file(CONF_PATH);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data,&error);
    if(error.error == QJsonParseError::NoError)
    {
        if(doc.isNull() || doc.isEmpty())
        {
            return;
        }
        if(doc.isObject())
        {
            QJsonObject obj =doc.object();
            QJsonValue value = obj.value("login");
            if(value.isObject())
            {
                QJsonObject sub = value.toObject();
                username = sub.value("user").toString();
                passwd = sub.value("pwd").toString();
                remember = sub.value("remember").toString();
            }
        }
    }
    else
    {
        QMessageBox::critical(this,"错误",error.errorString());
        return;
    }

    //用户名和密码需要base64解码
    username = decodeBase64(username);
    passwd = decodeBase64(passwd);
    //qDebug()<<username<<passwd<<remember;

    if("yes" == remember)
    {
        ui->login_username->setText(username);
        ui->login_passwd->setText(passwd);
        ui->login_checkBox->setCheckState(Qt::Checked);
    }


}

//注册页面的实现
void Login::on_reg_ok_clicked()
{
    //取数据
    QString username = ui->reg_username->text();
    QString nick = ui->reg_nick->text();
    QString passwd = ui->reg_passwd->text();
    QString repasswd = ui->reg_repasswd->text();
    QString phone = ui->reg_phone->text();
    QString email = ui->reg_email->text();

    QString ip = ui->set_ip->text();
    QString port = ui->set_port->text();
    //数据的校验
    QRegExp ex(USER_REG);
    if(!ex.exactMatch(username))
    {
        QMessageBox::critical(this,"错误",QString("你输入的用户名不符合规范!"));
        ui->reg_username->clear();
        ui->reg_username->setFocus();
        return;
    }

    ex.setPattern(USER_REG);
    if(!ex.exactMatch(nick))
    {
        QMessageBox::critical(this,"错误",QString("你输入的昵称不符合规范!"));
        ui->reg_nick->clear();
        ui->reg_nick->setFocus();
        return;
    }

    ex.setPattern(PASSWD_REG);
    if(!ex.exactMatch(passwd))
    {
        QMessageBox::critical(this,"错误",QString("你输入的密码不符合规范!"));
        ui->reg_passwd->clear();
        ui->reg_passwd->setFocus();
        return;
    }

    ex.setPattern(PHONE_REG);
    if(!ex.exactMatch(phone))
    {
        QMessageBox::critical(this,"错误",QString("你输入的手机号不符合规范!"));
        ui->reg_phone->clear();
        ui->reg_phone->setFocus();
        return;
    }

    ex.setPattern(EMAIL_REG);
    if(!ex.exactMatch(email))
    {
        QMessageBox::critical(this,"错误",QString("你输入的邮箱不符合规范!"));
        ui->reg_email->clear();
        ui->reg_email->setFocus();
        return;
    }

    //两次密码不一样，给出反馈
    if(passwd != repasswd)
    {
        QMessageBox::critical(this,"错误",QString("两次输入的密码不一样!"));
        ui->reg_passwd->clear();
        ui->reg_repasswd->clear();
        ui->reg_passwd->setFocus();
        return;
    }


    qDebug()<<"用户注册信息"<<username<<nick<<passwd<<repasswd<<phone<<email;

    //需要知道发送给server的数据
    //用户输入的信息打包成json对象
    QByteArray postData = getRegJson(username,nick,passwd,phone,email);

    //发送HTTP请求，这个对象由静态函数获取，整个项目里面只有一个对象
    //QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    QNetworkAccessManager * manager = Login::getNetManager();
    //HTTP 的协议头
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
    QString url = QString("http://%1:%2/reg").arg(ip).arg(port);//直接用ip port的前提条件是有配置文件的初始化
    request.setUrl(QUrl(url));

    //发送http数据
    QNetworkReply  *reply = manager->post(request,postData);
    //接受服务器返回的数据
    connect(reply,&QNetworkReply::finished,this,[=](){
        QByteArray jsonData = reply->readAll();
        /*
         * {"code":"002"} 成功
         * {"code":"003"} 用户已存在
         * {"code":"004"} 失败
         */
        //解析返回的JSON
        QString status = getRegWebCode(jsonData);
        //打印得到的json串
        qDebug()<<"注册服务器返回的信息:"<<status;
        //解析字符串，得倒一个数字
        if("002" == status)
        {
            QMessageBox::information(this,"提示","注册成功!");
            // 将当前的用户信息填写到登录页面，跳转到登录页面
            ui->login_username->setText(username);
            ui->login_passwd->setText(passwd);
            ui->stackedWidget->setCurrentIndex(0);
            //注册页面的信息要清空
            ui->reg_username->clear();
            ui->reg_nick->clear();
            ui->reg_passwd->clear();
            ui->reg_repasswd->clear();
            ui->reg_phone->clear();
            ui->reg_email->clear();
        }
        else if("003" == status)
        {
            QMessageBox::critical(this,"提示","该注册用户已经存在!");
        }
        else{
            QMessageBox::critical(this,"提示","注册失败!");
        }
    });

}

//登录界面的操作
void Login::on_login_ok_clicked()
{
    //1.先获取输框的内容
    QString username = ui->login_username->text();
    QString old_username = username;
    QString passwd = ui->login_passwd->text();
    QString old_passwd = passwd;
    bool remember = ui->login_checkBox->isChecked();
    //2.验证
    QRegExp ex(USER_REG);
    if(!ex.exactMatch(username))
    {
        QMessageBox::critical(this,"错误","输入的用户名错误!");
        ui->login_username->clear();
        ui->login_username->setFocus();
        return;
    }
    ex.setPattern(PASSWD_REG);
    if(!ex.exactMatch(passwd))
    {
        QMessageBox::critical(this,"错误","输入的密码错误!");
        ui->login_passwd->clear();
        ui->login_passwd->setFocus();
        return;
    }
    //加密username,passwd；同时保存到配置文件
    //先获取IP和PORT
    QString ip,port;
    int ret = getIpAndPort(CONF_PATH,&ip,&port);
    if(ret == -1)
    {
        QMessageBox::critical(this,"错误","获取IP和PORT失败!");
        return;
    }

    //配置文件里的密码和用户是base64转码
    //把用户名和密码进行加密
    username = encryptBase64(username);
    passwd  = encryptBase64(passwd);

    qDebug()<<username<<passwd<<remember<<ip<<port;
    //登录信息写入配置文件
    loginConvertJson(username,passwd,remember,ip,port);

    //要发送出去的密码还需要再经过md5的加密
    //passwd = md5(passwd);
    qDebug()<<"密码md5加密后"<<passwd;
    //服务器端只去关系 username 和 passed
    QByteArray data = getLoginJson(old_username,old_passwd);

    //发送HTTP请求，这个对象由静态函数获取，整个项目里面只有一个对象
    QNetworkAccessManager * manager = Login::getNetManager();
    //HTTP 的协议头
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,data.size());
    QString url = QString("http://%1:%2/login").arg(ip).arg(port);//直接用ip port的前提条件是有配置文件的初始化
    request.setUrl(QUrl(url));

    //发送http数据
    QNetworkReply  *reply = manager->post(request,data);
    //接受服务器返回的数据
    connect(reply,&QNetworkReply::finished,this,[=]()
    {
        QByteArray jsonData = reply->readAll();
        /*
         * //成功
         * {
         *     "code":"000",
         *     "token":"xxx"
         * }
         * //失败
         * {
         *     "code":"001",
         *     "token":"xxx"
         * }
         *
         */
        //解析返回的JSON
        QString code;
        QString token;
        int ret = getLoginWebCode(jsonData,&code,&token);
        if(ret == -1)
        {
            return;
        }
        //打印得到的json串
        cout<<"注册服务器返回的信息:"<<code<<token;
        if("000" == code)
        {
            //成功
            //QMessageBox::information(this,"提示","登录成功!");
            //保存登录的信息
            logininfoinstance * instance = logininfoinstance::getInstance();
            instance->setLoginInfo(old_username,ip,port,token);

            //login 界面隐藏
            this->hide();

            //显示出主界面
            m_window->showMainWindow();

        }
        else if("001" == code)
        {
            QMessageBox::critical(this,"提示","用户名或者密码错误!");
        }
     });

}
