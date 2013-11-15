INCLUDEPATH += $$PWD
DEPENDEPATH += $$PWD
QT += network

!build_mail_lib:DEFINES += MAIL_NO_LIB

HEADERS += \
    $$PWD/qxthmac.h \
    $$PWD/qxtmail_p.h \
    $$PWD/qxtmailattachment.h \
    $$PWD/qxtmailmessage.h \
    $$PWD/qxtsmtp.h \
    $$PWD/qxtsmtp_p.h \
    $$PWD/qxtglobal.h \
    $$PWD/qxtpop3.h \
    $$PWD/qxtpop3_p.h \
    $$PWD/qxtpop3listreply.h \
    $$PWD/qxtpop3reply.h \
    $$PWD/qxtpop3reply_p.h \
    $$PWD/qxtpop3retrreply.h \
    $$PWD/qxtpop3statreply.h

SOURCES += \
    $$PWD/qxthmac.cpp \
    $$PWD/qxtmailattachment.cpp \
    $$PWD/qxtmailmessage.cpp \
    $$PWD/qxtsmtp.cpp \
    $$PWD/qxtpop3.cpp \
    $$PWD/qxtpop3reply.cpp
