TEMPLATE = app
QT = gui core
CONFIG += qt warn_on console debug
DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
FORMS = ui/mainwindow.ui ui/valuesdialog.ui
HEADERS = src/mainwindowimpl.h \
 src/valuesdialogimpl.h \
 src/qddlistwidget.h \
 src/filedata.h \
 src/qsuperedit.h
SOURCES = src/mainwindowimpl.cpp \
 src/main.cpp \
 src/valuesdialogimpl.cpp \
 src/qddlistwidget.cpp \
 src/filedata.cpp \
 src/qsuperedit.cpp
LIBS = -lnetcdf -lnetcdf_c++
RESOURCES += mainres.qrc
