QT       += core gui widgets
CONFIG   += c++11 console
CONFIG   -= app_bundle
TEMPLATE  = app
TARGET    = lab1_tests

INCLUDEPATH += ..
SOURCES += test_main.cpp \
           ../mainwindow.cpp \
           ../calculator.cpp
HEADERS += ../mainwindow.h \
           ../calculator.h \
           ../tokens.h
FORMS   += ../mainwindow.ui
RESOURCES += ../resources.qrc
