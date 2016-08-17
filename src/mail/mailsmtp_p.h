/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project <http://libqxt.org>
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** Copyright (c) 2013 Debao Zhang <hello@debao.me>
**
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*****************************************************************************/

#ifndef MAILSMTP_P_H
#define MAILSMTP_P_H

#include "mailsmtp.h"
#include <QHash>
#include <QString>
#include <QList>
#include <QPair>
#include <QQueue>

class QxtSmtpResponse
{
public:
    inline QxtSmtpResponse() : code(0) { }

    int code;
    QList<QByteArray> textLines;

    QByteArray domain() const;
    inline bool hasGoodResponseCode() const { return code >= 200 && code < 300; }
    QByteArray singleLine() const;
};

class QxtSmtpResponseParser
{
public:
    /* in case of multiline response but not pipelining */
    enum State {
        StateStart,
        StateFirst,
        StateNext
    };

    bool usePipelining;
    State state;
    int lastIndex;
    QByteArray buffer;
    QxtSmtpResponse currentResponse;
    QQueue<QxtSmtpResponse> responses;

    inline QxtSmtpResponseParser() :
        usePipelining(false), state(StateFirst), lastIndex(0) {}
    bool feed(const QByteArray &data);  // return false on parse error
    inline bool hasResponse() const { return !responses.isEmpty(); }
    inline QxtSmtpResponse takeResponse() { return responses.dequeue(); }
};


class QxtSmtpPrivate : public QObject
{
    Q_OBJECT
public:
    QxtSmtpPrivate(QxtSmtp *q);

    Q_DECLARE_PUBLIC(QxtSmtp)
    QxtSmtp *q_ptr;

    enum SmtpState
    {
        Disconnected,
        StartState,
        EhloSent,
        EhloExtensionsReceived,
        EhloDone,
        HeloSent,
        StartTLSSent,
        AuthRequestSent,
        AuthUsernameSent,
        AuthSent,
        Authenticated,
        MailToSent,
        RcptAckPending,
        SendingBody,
        BodySent,
        Waiting,
        Resetting
    };

    bool useSecure, disableStartTLS;
    SmtpState state; // rather then an int use the enum.  makes sure invalid states are entered at compile time, and makes debugging easier
    QxtSmtp::AuthType authType;
    int allowedAuthTypes;
    QByteArray username, password;
    QxtSmtpResponseParser responseParser;
    QHash<QString, QString> extensions;
    QList<QPair<int, QxtMailMessage> > pending;
    QStringList recipients;
    int nextID, rcptNumber, rcptAck;
    bool mailAck;
    QxtSmtpResponse response;

#ifndef QT_NO_OPENSSL
    QSslSocket* socket;
#else
    QTcpSocket* socket;
#endif

    void parseEhlo();
    void startTLS();
    void authenticate();

    void authCramMD5();
    void authPlain();
    void authLogin();

    void sendNextRcpt();
    void sendBody();

public slots:
    void socketError(QAbstractSocket::SocketError err);
    void socketRead();

    void ehlo();
    void sendNext();
};

#endif // MAILSMTP_P_H
