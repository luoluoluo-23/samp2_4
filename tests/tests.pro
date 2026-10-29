QT       += core gui widgets
CONFIG   += c++11 console
CONFIG   -= app_bundle
TEMPLATE  = app
TARGET    = lab1_tests

INCLUDEPATH += ..
SOURCES += test_main.cpp \
           ../mainwindow.cpp
HEADERS += ../mainwindow.h
FORMS   += ../mainwindow.ui
