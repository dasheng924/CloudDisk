#include "buttongroup.h"
#include "ui_buttongroup.h"
#include "mainwindow.h"
#include <QToolButton>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>

bool ButtonGroup::min_max_flag = false;


ButtonGroup::ButtonGroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ButtonGroup)
{
    ui->setupUi(this);

    setParent(parent->parentWidget());//其实这个函数是在MainWindow里面获取的
    //不过这里也可以这样获取

    //初始化信号映射的对象
    m_mapper = new QSignalMapper(this);

    //记录当前的点击的按钮
    m_curBtn = ui->myfile;
    //按钮下面的文字会变成红色（点击后）
    m_curBtn->setStyleSheet("color:red");

    // key:value == 按钮显示内容：按钮指针
    m_btns.insert(ui->myfile->text(), ui->myfile);
    m_btns.insert(ui->sharelist->text(), ui->sharelist);
    m_btns.insert(ui->download->text(), ui->download);
    m_btns.insert(ui->transform->text(), ui->transform);
    m_btns.insert(ui->switchuser->text(), ui->switchuser);

    m_pages.insert(Page::MYFILE, ui->myfile->text());
    m_pages.insert(Page::SHARE, ui->sharelist->text());
    m_pages.insert(Page::TRANKING, ui->download->text());
    m_pages.insert(Page::TRANSFER, ui->transform->text());
    m_pages.insert(Page::SWITCHUSR, ui->switchuser->text());

    // 设置按钮信号映射
    //迭代器的初始化
    QMap<QString, QToolButton*>::iterator it = m_btns.begin();

    //遍历每一个按钮
    for(; it != m_btns.end(); ++it)
    {
        //添加一个映射，识别点是按钮的名字
        m_mapper->setMapping(it.value(), it.value()->text());
        //map() 该插槽根据哪个对象向其发送信号来发出信号。
        connect(it.value(), SIGNAL(clicked(bool)), m_mapper, SLOT(map()));
    }
    connect(m_mapper, SIGNAL(mapped(QString)), this, SLOT(slotButtonClick(QString)));

    // 关闭
    connect(ui->close, &QToolButton::clicked, [=]()
    {
        //emit closeWindow();
        m_parent->close();
    });
    // 最大化和恢复
    connect(ui->max, &QToolButton::clicked, [=]()
    {
        if(!min_max_flag)
        {
            ui->max->setIcon(QIcon(":/images/title_normal.png"));
            m_parent->showMaximized();
            min_max_flag = true;
        }
        else{
             ui->max->setIcon(QIcon(":/images/title_max.png"));
             m_parent->setGeometry(0,0,900,650);
             m_com.moveToCenter(m_parent);
             min_max_flag = false;
        }

    });
    // 最小化
    connect(ui->min, &QToolButton::clicked, [=]()
    {
        //emit minWindow();
        m_parent->showMinimized();
    });
}

ButtonGroup::~ButtonGroup()
{
    delete ui;
}

void ButtonGroup::slotButtonClick(Page cur)
{
    QString text = m_pages[cur];
    slotButtonClick(text);
}

void ButtonGroup::slotButtonClick(QString text)
{
    qDebug() << "+++++++++++++++" << text;
    //-----------------------为了更改按钮的状态和更新当前按钮对象--------
    QToolButton* btn = m_btns[text];//根据传入的名字，得出对应的对象

    //如果这个按钮是默认选中的就什么也不做
    /*记录当前的点击的按钮
    m_curBtn = ui->myfile;

     */
    if(btn == m_curBtn && btn != ui->switchuser)
    {
        return;
    }

    //如果不是默认的按钮，就需要进行切换页面
    m_curBtn->setStyleSheet("color:black");//原有的按钮文字变成黑色
    btn->setStyleSheet("color:red");//新切换的按钮变成红色
    m_curBtn = btn;//把当前的按钮对象进行更新保存
    //-------------------------------------------------
    // 发信号
    if(text == ui->myfile->text())
    {
        emit sigMyFile();
    }
    else if(text == ui->sharelist->text())
    {
        emit sigShareList();
    }
    else if(text == ui->download->text())
    {
        emit sigDownload();
    }
    else if(text == ui->transform->text())
    {
        emit sigTransform();
    }
    else if(text == ui->switchuser->text())
    {
        emit sigSwitchUser();
    }
}

void ButtonGroup::setParent(QWidget *parent)
{
    m_parent = parent;
}


//更换背景
void ButtonGroup::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QPixmap bk(":/images/title_bk.jpg");
    painter.drawPixmap(0, 0, width(), height(), bk);
}

//获取鼠标的位置
void ButtonGroup::mousePressEvent(QMouseEvent *event)
{
    // 如果是左键, 计算窗口左上角, 和当前按钮位置的距离
    if(event->button() == Qt::LeftButton)
    {
        // 计算和窗口左上角的相对位置
        m_pos = event->globalPos() - m_parent->geometry().topLeft();
    }
}

//移动鼠标，界面跟着移动
void ButtonGroup::mouseMoveEvent(QMouseEvent *event)
{
    // 移动是持续的状态, 需要使用buttons
    if(event->buttons() & Qt::LeftButton)
    {
        QPoint pos = event->globalPos() - m_pos;
        m_parent->move(pos);
    }
}
