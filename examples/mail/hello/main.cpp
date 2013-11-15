#include <QtCore>
#include "mailsmtp.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QxtSmtp smtp;
    smtp.connectToSecureHost("smtp.gmail.com");
    smtp.setUsername("xxxxxxxx@gmail.com");
    smtp.setPassword("********");

    QxtMailMessage message;
    message.addRecipient("hello@debao.me");
    message.setSubject("Test");
    message.setBody("This is for test only!");
    smtp.send(message);

    QObject::connect(&smtp, SIGNAL(finished()), &a, SLOT(quit()));

    return a.exec();
}
