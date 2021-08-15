#include "logininfoinstance.h"

logininfoinstance* logininfoinstance::logininfo = new logininfoinstance;
logininfoinstance::Garbo logininfoinstance::garbo;

logininfoinstance *logininfoinstance::getInstance()
{
    return logininfoinstance::logininfo;
}

void logininfoinstance::destroy()
{
    if(logininfo != nullptr)
    {
        delete logininfoinstance::logininfo;
        logininfoinstance::logininfo = nullptr;
    }
}

void logininfoinstance::setLoginInfo(QString tmpUser, QString tmpIp, QString tmpPort, QString token)
{
    m_username = tmpUser;
    m_token = token;
    m_ip = tmpIp;
    m_port = tmpPort;
}

QString logininfoinstance::getUserName() const
{
    return m_username;
}

QString logininfoinstance::getToken() const
{
    return m_token;
}

QString logininfoinstance::getIp() const
{
    return m_ip;
}

QString logininfoinstance::getPort() const
{
    return m_port;
}

logininfoinstance::logininfoinstance()
{

}

logininfoinstance::~logininfoinstance()
{

}

logininfoinstance::logininfoinstance(const logininfoinstance &obj)
{
    Q_UNUSED(obj);
}

logininfoinstance &logininfoinstance::operator=(const logininfoinstance &obj)
{
    Q_UNUSED(obj);
    return *this;
}
