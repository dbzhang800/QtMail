TARGET = QtMail

#QMAKE_DOCS = $$PWD/doc/qtmail.qdocconf

load(qt_module)
QT -= gui

CONFIG += build_mail_lib
include(qtmail.pri)
