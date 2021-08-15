#ifndef LOGININFOINSTANCE_H
#define LOGININFOINSTANCE_H

#include <QString>



//登录界面的单例模式
//为了在整个程序里面就存在一个登录界面，不浪费资源

class logininfoinstance
{
public:
    //静态公共接口，访问唯一对象
    static logininfoinstance * getInstance();
    //释放堆空间
    static void destroy();
    //设置登陆信息
    void setLoginInfo( QString tmpUser, QString tmpIp, QString tmpPort,  QString token="");
    //提供get接口
    QString getUserName()const;
    QString getToken() const;
    QString getIp() const;
    QString getPort() const;


private:
    //构造，析构，拷贝构造，赋值都被设定为私有
    //留有公共接口，访问单例对象
    logininfoinstance();
    ~logininfoinstance();
    logininfoinstance(const logininfoinstance &obj);
    logininfoinstance& operator=(const logininfoinstance &obj);

    //它的唯一工作就是在析构函数中删除Singleton的实例
    class Garbo
    {
    public:
        ~Garbo()
        {
            //释放堆区空间
            logininfoinstance::destroy();
        }
    };

private:
    static logininfoinstance *logininfo;
    static Garbo garbo;

    QString m_username; //当前登陆用户
    QString m_token; //登陆token
    QString m_ip; //web服务器ip
    QString m_port; //web服务器端口

};

#endif // LOGININFOINSTANCE_H
