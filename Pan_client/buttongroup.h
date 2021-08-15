#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H

#include <QWidget>
#include <QSignalMapper>
#include <QMap>
#include <QObject>

#include "common/common.h"


namespace Ui {
class ButtonGroup;
}

class QToolButton;
enum Page{MYFILE, SHARE, TRANKING, TRANSFER, SWITCHUSR};
class ButtonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit ButtonGroup(QWidget *parent = 0);
    ~ButtonGroup();


public slots:
    // 按钮处理函数
    void slotButtonClick(Page cur);
    void slotButtonClick(QString text);
    void setParent(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);

signals:
    void sigMyFile();
    void sigShareList();
    void sigDownload();
    void sigTransform();
    void sigSwitchUser();
    void closeWindow();
    void minWindow();
    void maxWindow();

private:
    Ui::ButtonGroup *ui;

    QPoint m_pos;//记录每一次固定不变的距离
    QWidget* m_parent;//记录当前窗口的父对象
    QSignalMapper* m_mapper;//信号映射的对象

    QToolButton* m_curBtn;//记录当前选中的按钮对象
    QMap<QString, QToolButton*> m_btns; //key：按钮名字 value：按钮对象本身

    QMap<Page, QString> m_pages;//key：页面枚举值 value：对应的页面切换按钮名字
    static bool min_max_flag;//窗口的最大化，复原标志
    Common m_com;//Common类的成员变量

};

#endif // BUTTONGROUP_H
