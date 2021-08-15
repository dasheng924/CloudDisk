#include "form.h"
#include "ui_form.h"

#include <QPainter>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    m_widget = parent;


    //设置LOGO图片
    ui->logoIcon->setPixmap(QPixmap(":/images/logo.ico").scaled(30,30));

    //去边框，鼠标放上去才会有边框
    ui->setting->setAutoRaise(true);
    ui->min->setAutoRaise(true);
    ui->close->setAutoRaise(true);

    //设置3个按钮的背景
    ui->setting->setIcon(QPixmap(":/images/login_setting.png"));
    ui->setting->setIconSize(QSize(15,15));

    ui->min->setIcon(QPixmap(":/images/tile_min.png"));
    ui->min->setIconSize(QSize(15,15));

    ui->close->setIcon(QPixmap(":/images/login_close.png"));
    ui->close->setIconSize(QSize(15,15));

    //设置样式表
    ui->logoText->setStyleSheet("color:rgb(255,255,255)");

    //设置3个按钮的功能实现
    connect(ui->setting,&QToolButton::clicked,this,[=](){
        emit showSetting();
        });

    connect(ui->min,&QToolButton::clicked,this,[=](){
        m_widget->showMinimized();
    });

    connect(ui->close,&QToolButton::clicked,this,[=](){
        emit closeWidget();
    });

}

Form::~Form()
{
    delete ui;
}


//允许拖着标题，上半部分就会移动窗口
void Form::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_p = event->globalPos() - m_widget->geometry().topLeft();
    }
}

void Form::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton) //一个持续的动作 用buttons & 来判断
    {
        //保存鼠标点到窗口左上角的距离
        m_widget->move(event->globalPos() - m_p) ;
    }
}




