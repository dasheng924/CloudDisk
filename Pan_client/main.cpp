#include "mainwindow.h"
#include "form.h"
#include "login.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    //w.show();

    Login l;
    l.show();




    a.setWindowIcon(QIcon(":/images/logo.ico"));
    return a.exec();
}
