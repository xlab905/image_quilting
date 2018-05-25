#-------------------------------------------------
#
# Project created by QtCreator 2017-06-30T15:34:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = image_quilting
TEMPLATE = app

INCLUDEPATH += /home/kevin/research/texture/image_quilting/ext

INCLUDEPATH += /usr/local/opencv-2-4-10/include
LIBS += -L/usr/local/opencv-2-4-10/lib -lopencv_core -lopencv_highgui -lopencv_imgproc

SOURCES += main.cpp\
        imagequilting.cpp \
    io.cpp

HEADERS  += imagequilting.h \
    io.h

FORMS    +=

CONFIG += c++11
