#ifndef DATAPROGRESS_H
#define DATAPROGRESS_H

#include <QWidget>

namespace Ui {
class DataProgress;
}

class DataProgress : public QWidget
{
    Q_OBJECT

public:
    explicit DataProgress(QWidget *parent = nullptr);
    ~DataProgress();

public:
    void setFileName(const QString name);
    void setProgress(int value =0,int max= 100);



private:
    Ui::DataProgress *ui;
};

#endif // DATAPROGRESS_H
