#ifndef LOGIN_H
#define LOGIN_H

#include <QPaintEvent>
#include <QWidget>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QByteArray>

#include "mainwindow.h"
#include "common/common.h"


namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    //服务器IP port端口设置好，保存到配置文件
    int saveWebConf(QString ip,QString port,QString path);
    //第一次自动生成配置文件
    int createWebConf(QString path);
    //根据配置文件初始化服务器设置界面
    int initWebSetting();
    //根据配置文件获取IP和port
    int getIpAndPort(const QString path,QString *ip,QString *port);
    //注册界面，用户输入的信息打包成json对象
    QByteArray getRegJson(QString user,QString nick,QString passwd,QString phone,QString email);

    // 得到http通信类对象
    static QNetworkAccessManager* getNetManager();

    //注册时，获取服务器返回的code编码
    QString getRegWebCode(QByteArray data);
    //登录时，获取服务器返回的code编码
    int getLoginWebCode(QByteArray data,QString *code,QString *token);

    //简单的base64加解密
    QString encryptBase64(QString str);
    //base64解密
    QString decodeBase64(QString str);

    //登录信息写进配置文件
    void loginConvertJson(QString username,QString passwd,bool remember,QString ip,QString port);
    //构造登录信息发送给服务器 用户名和密码
    QByteArray getLoginJson(QString username,QString passwd);
    //密码md5加密
    QString md5(QString str);
    //记住密码，默认登录
    void defaultlogin();



protected:
    void paintEvent(QPaintEvent * v) override;

private slots:
    void on_set_ok_clicked();
    void on_reg_ok_clicked();

    void on_login_ok_clicked();

signals:
    void mainWindowShow();

private:
    Ui::Login *ui;
    static QNetworkAccessManager *m_manager;
    MainWindow *m_window;
    Common m_comm;


};

#endif // LOGIN_H
