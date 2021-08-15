#ifndef TRANSFER_H
#define TRANSFER_H

#include <QWidget>

#include "common/common.h"

namespace Ui {
class Transfer;
}

class Transfer : public QWidget
{
    Q_OBJECT

public:
    explicit Transfer(QWidget *parent = nullptr);
    ~Transfer();
public:
    //显示数据传输记录
    void dispayDataRecord(QString path=RECORDDIR);
    //显示上传记录
    void showUpload();
    //显示下载记录
    void showDownload();

signals:
    void currentTabSignal(QString);//告诉主界面，当前是那个tab

private:
    Ui::Transfer *ui;
};

#endif // TRANSFER_H
