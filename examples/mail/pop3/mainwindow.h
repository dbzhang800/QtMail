#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "mailpop3.h"
#include <QMainWindow>
#include <QPair>
#include <QSettings>

class QxtMailMessage;
class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent=0);
    ~MainWindow();

private slots:
    void connectToHost();
    void newCmd();
    void saveSettings();
    void handleQuit(int code);
    void handleStat(int code);
    void handleList(int code);
    void handleRetr(int code);
    void handleDele(int code);
    void handleRset(int code);
    void connected();
    void disconnected();
    void handleSslError(const QByteArray& msg);
    void handleAuthError(const QByteArray& msg);
    void progress(int percent);

private:
    enum Encryption {
        UseSSL,
        UseStartTLS,
        Clear
    };
    QSettings* settings;
    QxtPop3 *pop;
    QxtMailMessage* msg;
    void output(const QString& text) {plainTextEdit->appendPlainText(text);}
    void printError();
    void help();
};

#endif // MAINWINDOW_H
