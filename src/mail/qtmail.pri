INCLUDEPATH += $$PWD
DEPENDEPATH += $$PWD
QT += network

!build_mail_lib:DEFINES += MAIL_NO_LIB

HEADERS += \
    $$PWD/mailhmac.h \
    $$PWD/mailutility_p.h \
    $$PWD/mailattachment.h \
    $$PWD/mailmessage.h \
    $$PWD/mailsmtp.h \
    $$PWD/mailsmtp_p.h \
    $$PWD/mailglobal.h \
    $$PWD/mailpop3.h \
    $$PWD/mailpop3_p.h \
    $$PWD/mailpop3listreply.h \
    $$PWD/mailpop3reply.h \
    $$PWD/mailpop3reply_p.h \
    $$PWD/mailpop3retrreply.h \
    $$PWD/mailpop3statreply.h

SOURCES += \
    $$PWD/mailhmac.cpp \
    $$PWD/mailattachment.cpp \
    $$PWD/mailmessage.cpp \
    $$PWD/mailsmtp.cpp \
    $$PWD/mailpop3.cpp \
    $$PWD/mailpop3reply.cpp
