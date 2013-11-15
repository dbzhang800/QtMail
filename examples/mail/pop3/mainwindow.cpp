#include "mainwindow.h"
#include "mailpop3reply.h"
#include <QPair>
#include <QFileDialog>
#include <QFile>
#include <QSslSocket>




MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), pop(0), msg(0)
{
    setupUi(this);
    settings = new QSettings(QSettings::UserScope,"Qxt", "MailTest", this);
    pop = new QxtPop3(this);
    pop->sslSocket()->setProtocol(QSsl::TlsV1_0);
    pop->sslSocket()->setPeerVerifyMode(QSslSocket::QueryPeer);
    QString username = settings->value("username").toString();
    QString hostname = settings->value("hostname").toString();
    QString pass = settings->value("password").toString();
    int port = settings->value("port").toInt();
    bool ok;
    Encryption enc = Encryption(settings->value("encryption").toInt(&ok));
    if (!ok) enc = Clear;
    switch (enc)
    {
    case UseSSL:
        sslRadioButton->setChecked(true);
        break;
    case UseStartTLS:
        startTLSRadioButton->setChecked(true);
        break;
    case Clear:
    default:
        clearRadioButton->setChecked(true);
        break;
    }
    usernameLineEdit->setText(username);
    hostnameLineEdit->setText(hostname);
    passwordLineEdit->setText(pass);
    if (port != 0) portLineEdit->setText(QString::number(port));
    connect(connectPushButton, SIGNAL(clicked()), this, SLOT(connectToHost()));
    connect(settingsPushButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(pop, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(pop, SIGNAL(connected()), this, SLOT(connected()));
    connect(pop, SIGNAL(connectionFailed(QByteArray)), this, SLOT(handleSslError(QByteArray)));
    connect(pop, SIGNAL(encryptionFailed(QByteArray)), this, SLOT(handleSslError(QByteArray)));
    connect(pop, SIGNAL(authenticationFailed(QByteArray)), this, SLOT(handleAuthError(QByteArray)));
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(newCmd()));
    help();
}

MainWindow::~MainWindow()
{
    if (msg) delete msg;
}

void MainWindow::newCmd()
{
    QxtPop3Reply* reply = 0;
    QString line = lineEdit->text();
    lineEdit->clear();
    QStringList words = line.split(" ", QString::SkipEmptyParts);
    if (words[0] == "quit")
    {
        if (!pop->isConnected()) return;
        reply = pop->quit();
        connect(reply, SIGNAL(finished(int)), this, SLOT(handleQuit(int)));
        plainTextEdit->appendPlainText("QUIT sent");
    }
    else if (words[0] == "stat")
    {
        if (!pop->isConnected()) return;
        reply = pop->stat();
        connect(reply, SIGNAL(finished(int)), this, SLOT(handleStat(int)));
        output("STAT sent");
    }
    else if (words[0] == "list")
    {
        if (!pop->isConnected()) return;
        reply = pop->messageList();
        connect(reply, SIGNAL(finished(int)), this, SLOT(handleList(int)));
        output("LIST sent");
    }
    else if (words[0] == "retr")
    {
        if (!pop->isConnected()) return;
        if (words.length() < 2)
        {
            output("Usage: retr <which>");
            return;
        }
        int which = words[1].toInt();
        reply = pop->retrieveMessage(which);
        connect(reply, SIGNAL(finished(int)), this, SLOT(handleRetr(int)));
        connect(reply, SIGNAL(progress(int)), this, SLOT(progress(int)));
        output("RETR sent");
    }
    else if (words[0] == "dele")
    {
        if (!pop->isConnected()) return;
        if (words.length() < 2)
        {
            output("Usage: dele <which>");
            return;
        }
        int which = words[1].toInt();
        reply = pop->deleteMessage(which);
        connect(reply, SIGNAL(finished(int)), this, SLOT(handleDele(int)));
        output("DELE sent");
    }
    else if (words[0] == "reset")
    {
        if (!pop->isConnected()) return;
        reply = pop->reset();
        connect(reply, SIGNAL(finished(int)), this, SLOT(handleRset(int)));
        output("RSET sent");
    }
    else if (words[0] == "help")
    {
        help();
    }
    else if (words[0] == "saveatt")
    {
        if (msg == 0)
        {
            output("no current message.");
            return;
        }
        if (words.length() < 2)
        {
            output("Usage: saveatt <attachment name>");
            return;
        }
        if (!msg->attachments().contains(words[1]))
        {
            output(QString("no such attachment: ") + words[1]);
            return;
        }
        QxtMailAttachment att = msg->attachment(words[1]);
        QString filename = QFileDialog::getSaveFileName(this, "Save attachment", QString("./") + words[1]);
        if (!filename.isEmpty())
        {
            QFile file(filename);
            QIODevice::OpenMode flags = QIODevice::WriteOnly;
            if (att.isText()) flags |= QIODevice::Text;
            file.open(flags);
            file.write(att.rawData());
            file.close();
            output("Attachment saved");
        }
    }
    else
    {
        output("unhandled command");
    }
}


    void MainWindow::handleQuit(int code)
    {
        switch (code)
        {
        case QxtPop3Reply::OK:
            output("QUIT: OK received");
            break;
        case QxtPop3Reply::Aborted:
            printError();
            output("QUIT aborted.");
            break;
        case QxtPop3Reply::Timeout:
            output("QUIT: time out.");
            break;
        default:
            break;
        }
    }

    void MainWindow::handleStat(int code)
    {
        switch (code)
        {
        case QxtPop3Reply::OK:
            {
                QxtPop3StatReply* reply = dynamic_cast<QxtPop3StatReply*>(sender());
                if (reply == 0)
                {
                    qWarning("MainWindow::handleStat: sender is not a QxtPop3StatReply !");
                    return;
                }
                output(QString("STAT: %1 messages, %2 bytes").arg(reply->count()).arg(reply->size()));
            }
            break;
        case QxtPop3Reply::Aborted:
            printError();
            output("STAT aborted.");
            break;
        case QxtPop3Reply::Timeout:
            output("STAT: time out.");
            break;
        default:
            break;
        }
    }

    void MainWindow::handleList(int code)
    {
        switch (code)
        {
        case QxtPop3Reply::OK:
            output("LIST: OK received");
            {
                QxtPop3ListReply* reply = dynamic_cast<QxtPop3ListReply*>(sender());
                if (reply == 0)
                {
                    qWarning("MainWindow::handleList: sender is not a QxtPop3ListReply !");
                    return;
                }
                foreach (QxtPop3Reply::MessageInfo msginfo, reply->list())
                {
                    output(QString("message %1: %2 bytes").arg(msginfo.id).arg(msginfo.size));
                }
            }
            break;
        case QxtPop3Reply::Aborted:
            printError();
            output("LIST aborted.");
            break;
        case QxtPop3Reply::Timeout:
            output("LIST: time out.");
            break;
        default:
            break;
        }
    }

    void MainWindow::handleRetr(int code)
    {
        switch (code)
        {
        case QxtPop3Reply::OK:
            output("RETR: OK received");
            {
                QxtPop3RetrReply* reply = dynamic_cast<QxtPop3RetrReply *>(sender());
                if (reply == 0)
                {
                    qWarning("MainWindow::handleRetr: sender is not a QxtPop3RetrReply !");
                    return;
                }
                if (msg)
                {
                    delete msg;
                }
                msg = reply->message();
                output("------Message Headers:");
                foreach(QString key, msg->extraHeaders().keys())
                {
                    output(QString("%1: %2").arg(key, msg->extraHeader(key)));
                }
                output("------Message Body:");
                output(msg->body());
                if (msg->attachments().size() > 0)
                {
                    output("------Attachments:");
                    foreach(QString filename, msg->attachments().keys())
                    {
                        output(QString("%1: Content-Type = %2, %3 bytes")
                               .arg(filename)
                               .arg(msg->attachment(filename).contentType())
                               .arg(msg->attachment(filename).rawData().length()));
                    }
                }
                output("------End of Message\n");
            }
            break;
        case QxtPop3Reply::Aborted:
            printError();
            output("RETR aborted.");
            break;
        case QxtPop3Reply::Timeout:
            output("RETR: time out.");
            break;
        default:
            break;
        }
    }

    void MainWindow::handleDele(int code)
    {
        switch (code)
        {
        case QxtPop3Reply::OK:
            output("DELE: OK received");
            break;
        case QxtPop3Reply::Aborted:
            printError();
            output("DELE aborted.");
            break;
        case QxtPop3Reply::Timeout:
            output("DELE: time out.");
            break;
        default:
            break;
        }
    }

    void MainWindow::handleRset(int code)
    {
        switch (code)
        {
        case QxtPop3Reply::OK:
            output("RSET: OK received");
            break;
        case QxtPop3Reply::Aborted:
            printError();
            output("RSET aborted.");
            break;
        case QxtPop3Reply::Timeout:
            output("RSET: time out.");
            break;
        default:
            break;
        }
    }

    void MainWindow::printError()
    {
        QObject* s = sender();
        if (s != 0)
        {
            QxtPop3Reply* reply = qobject_cast<QxtPop3Reply*>(s);
            if (reply->status() == QxtPop3Reply::Error)
            {
                output(reply->error());
            }
        }
    }

    void MainWindow::help()
    {
        output("Available commands:");
        output("help: print this help message\n"
               "quit\n"
               "stat\n"
               "list\n"
               "retr <nb>\n"
               "dele <nb>\n"
               "reset\n"
               "saveatt <attachment name>: save attachment of last retrieved message\n");
    }

    void MainWindow::connectToHost()
    {
        QString hostname = hostnameLineEdit->text();
        bool useSSL = sslRadioButton->isChecked();
        if (startTLSRadioButton->isChecked())
        {
            pop->setStartTlsDisabled(false);
        }
        else
        {
            pop->setStartTlsDisabled(true);
        }
        bool ok;
        int port = portLineEdit->text().toInt(&ok);
        if (!ok || port == 0)
        { // default port
            if (useSSL)
            {
                port = 995;
            }
            else
            {
                port = 110;
            }
        }
        QString username = usernameLineEdit->text();
        QString pass = passwordLineEdit->text();
        pop->setUsername(username.toLocal8Bit());
        pop->setPassword(pass.toLocal8Bit());
        if (useSSL)
        {
            pop->connectToSecureHost(hostname.toLocal8Bit(), port);
        }
        else
        {
            pop->connectToHost(hostname.toLocal8Bit(), port);
        }
    }

    void MainWindow::saveSettings()
    {
        QString hostname = hostnameLineEdit->text();
        settings->setValue("hostname", hostname);
        Encryption enc;
        if(sslRadioButton->isChecked())
        {
            enc = UseSSL;
        }
        else if (startTLSRadioButton->isChecked())
        {
            enc = UseStartTLS;
        }
        else
        {
            enc = Clear;
        }
        settings->setValue("Encryption", enc);
        bool ok;
        int port = portLineEdit->text().toInt(&ok);
        if (ok && port != 0) settings->setValue("port", port);
        QString username = usernameLineEdit->text();
        settings->setValue("username", username);
        QString pass = passwordLineEdit->text();
        settings->setValue("password", pass);
        settings->sync();
    }

    void MainWindow::connected()
    {
        connectPushButton->setDisabled(true);
        output("Connected to host");
    }

    void MainWindow::disconnected()
    {
        connectPushButton->setEnabled(true);
        output("Disconnected from Host");
        pop->clearReplies();
    }

    void MainWindow::handleSslError(const QByteArray& msg)
    {
        output(QString("Ssl handshake error: ") + msg);
    }

    void MainWindow::handleAuthError(const QByteArray& msg)
    {
        output(QString("Authentication error: ") + msg);
    }

    void MainWindow::progress(int percent)
    {
        output(QString("Progress: %1%").arg(percent));
    }
