QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    buttongroup.cpp \
    common/common.cpp \
    common/des.c \
    common/downloadlayout.cpp \
    common/downloadtask.cpp \
    common/logininfoinstance.cpp \
    common/uploadlayout.cpp \
    common/uploadtask.cpp \
    form.cpp \
    login.cpp \
    main.cpp \
    mainwindow.cpp \
    myfilewg.cpp \
    myselfWidget/dataprogress.cpp \
    myselfWidget/filepropertyinfo.cpp \
    myselfWidget/mymenu.cpp \
    rankinglist.cpp \
    sharelist.cpp \
    transfer.cpp

HEADERS += \
    buttongroup.h \
    common/common.h \
    common/des.h \
    common/downloadlayout.h \
    common/downloadtask.h \
    common/logininfoinstance.h \
    common/uploadlayout.h \
    common/uploadtask.h \
    form.h \
    login.h \
    mainwindow.h \
    myfilewg.h \
    myselfWidget/dataprogress.h \
    myselfWidget/filepropertyinfo.h \
    myselfWidget/mymenu.h \
    rankinglist.h \
    sharelist.h \
    transfer.h

FORMS += \
    buttongroup.ui \
    form.ui \
    login.ui \
    mainwindow.ui \
    myfilewg.ui \
    myselfWidget/dataprogress.ui \
    myselfWidget/filepropertyinfo.ui \
    rankinglist.ui \
    sharelist.ui \
    transfer.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
