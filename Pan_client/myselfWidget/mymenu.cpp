#include "mymenu.h"

MyMenu::MyMenu(QWidget *parent) : QMenu(parent)
{
    setStyleSheet(
            "background-color:rgba(202, 245, 238, 80);"
            "color:rgb(255, 255, 0);"
            "font: 14pt \".AppleSystemUIFont\";"
    );
}
