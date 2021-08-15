#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;


signals:
    void showSetting(); //设置按钮
    void closeWidget(); //关闭按钮


private:
    Ui::Form *ui;
    QPoint m_p;
    QWidget * m_widget;
};

#endif // FORM_H
