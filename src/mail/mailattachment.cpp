
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
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
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

/*!
 * \class QxtMailAttachment
 * \inmodule QxtNetwork
 * \brief The QxtMailAttachment class represents an attachement to a QxtMailMessage
 */




#include "mailattachment.h"
#include "mailutility_p.h"
#include <QTextCodec>
#include <QBuffer>
#include <QPointer>
#include <QFile>
#include <QtDebug>

class QxtMailAttachmentPrivate : public QSharedData
{
public:
    QHash<QString, QString> extraHeaders;
    QByteArray boundary; // in case of embedded multipart
    QMap<QString, QxtMailAttachment> attachments; // in case of embedded multipart.  QMap because order makes sense
    QString contentType;
    // those two members are mutable because they may change in the const rawData() method of QxtMailAttachment,
    // while caching the raw data for the attachment if needed.
    mutable QPointer<QIODevice> content;
    mutable bool deleteContent;

    QxtMailAttachmentPrivate()
    {
        content = 0;
        deleteContent = false;
        contentType = QStringLiteral("text/plain");
    }

    ~QxtMailAttachmentPrivate()
    {
        if (deleteContent && content)
            content->deleteLater();
        deleteContent = false;
        content = 0;
    }
};

QxtMailAttachment::QxtMailAttachment()
{
    qxt_d = new QxtMailAttachmentPrivate;
}

QxtMailAttachment::QxtMailAttachment(const QxtMailAttachment& other) : qxt_d(other.qxt_d)
{
    // trivial copy constructor
}

QxtMailAttachment::QxtMailAttachment(const QByteArray& content, const QString& contentType)
{
    qxt_d = new QxtMailAttachmentPrivate;
    setContentType(contentType);
    setContent(content);
}

QxtMailAttachment::QxtMailAttachment(QIODevice* content, const QString& contentType)
{
    qxt_d = new QxtMailAttachmentPrivate;
    setContentType(contentType);
    setContent(content);
}

QxtMailAttachment& QxtMailAttachment::operator=(const QxtMailAttachment & other)
{
    qxt_d = other.qxt_d;
    return *this;
}

QxtMailAttachment::~QxtMailAttachment()
{
    // trivial destructor
}

QIODevice* QxtMailAttachment::content() const
{
    return qxt_d->content;
}

void QxtMailAttachment::setContent(const QByteArray& content)
{
    if (qxt_d->deleteContent && qxt_d->content)
        qxt_d->content->deleteLater();
    qxt_d->content = new QBuffer;
    setDeleteContent(true);
    static_cast<QBuffer*>(qxt_d->content.data())->setData(content);
}

void QxtMailAttachment::setContent(QIODevice* content)
{
    if (qxt_d->deleteContent && qxt_d->content)
        qxt_d->content->deleteLater();
    qxt_d->content = content;
}

bool QxtMailAttachment::deleteContent() const
{
    return qxt_d->deleteContent;
}

void QxtMailAttachment::setDeleteContent(bool enable)
{
    qxt_d->deleteContent = enable;
}

QString QxtMailAttachment::contentType() const
{
    return qxt_d->contentType;
}

void QxtMailAttachment::setContentType(const QString& contentType)
{
    qxt_d->contentType = contentType;
    if (contentType.startsWith("multipart", Qt::CaseInsensitive)) {
        QRegExp re(QStringLiteral("boundary=([^ ;\\r]+)"));
        if (re.indexIn(contentType) != -1)
            qxt_d->boundary = re.capturedTexts()[1].toLatin1();
        else {
            qxt_d->boundary = qxt_gen_boundary();
            qxt_d->contentType += (QStringLiteral("; boundary=") + qxt_d->boundary);
        }
    }
}

QHash<QString, QString> QxtMailAttachment::extraHeaders() const
{
    return qxt_d->extraHeaders;
}

QString QxtMailAttachment::extraHeader(const QString& key) const
{
    return qxt_d->extraHeaders[key.toLower()];
}

bool QxtMailAttachment::hasExtraHeader(const QString& key) const
{
    return qxt_d->extraHeaders.contains(key.toLower());
}

void QxtMailAttachment::setExtraHeader(const QString& key, const QString& value)
{
    if (key.compare(QStringLiteral("Content-Type"), Qt::CaseInsensitive) == 0)
        setContentType(value);
    else
        qxt_d->extraHeaders[key.toLower()] = value;
}

void QxtMailAttachment::setExtraHeaders(const QHash<QString, QString>& a)
{
    qxt_d->extraHeaders.clear();
    foreach(const QString& key, a.keys())
    {
        setExtraHeader(key, a[key]);
    }
}

void QxtMailAttachment::removeExtraHeader(const QString& key)
{
    qxt_d->extraHeaders.remove(key.toLower());
}

QMap<QString, QxtMailAttachment> QxtMailAttachment::attachments() const
{
    return qxt_d->attachments;
}

QxtMailAttachment QxtMailAttachment::attachment(const QString& filename) const
{
    return qxt_d->attachments[filename];
}

void QxtMailAttachment::addAttachment(const QString& filename, const QxtMailAttachment& attach)
{
    if (qxt_d->attachments.contains(filename))
    {
        qWarning() << "QxtMailMessage::addAttachment: " << filename << " already in use";
        int i = 1;
        while (qxt_d->attachments.contains(filename + QLatin1Char('.') + QString::number(i)))
        {
            i++;
        }
        qxt_d->attachments[filename+QLatin1Char('.')+QString::number(i)] = attach;
    }
    else
    {
        qxt_d->attachments[filename] = attach;
    }
}

void QxtMailAttachment::removeAttachment(const QString& filename)
{
    qxt_d->attachments.remove(filename);
}

QByteArray QxtMailAttachment::mimeData()
{
    bool isMultipart = false;
    QTextCodec* latin1 = QTextCodec::codecForName("latin1");

    if (qxt_d->attachments.count()) {
        if (!qxt_d->contentType.startsWith("multipart/", Qt::CaseInsensitive))
            setExtraHeader(QStringLiteral("Content-Type"), QStringLiteral("multipart/mixed"));
    }

    QByteArray rv = "Content-Type: " + qxt_d->contentType.toLatin1() + "\r\n";
    if (qxt_d->contentType.startsWith("multipart/", Qt::CaseInsensitive)) {
        isMultipart = true;
    }
    else {
        rv += "Content-Transfer-Encoding: base64\r\n";
    }

    foreach(const QString& r, qxt_d->extraHeaders.keys())
    {
        rv += qxt_fold_mime_header(r, extraHeader(r), latin1);
    }
    rv += "\r\n";

    const QByteArray& d = rawData();
    int chars = isMultipart? 73 : 57; // multipart preamble supposed to be 7bit latin1
    for (int pos = 0; pos < d.length(); pos += chars)
    {
        if (isMultipart) {
            rv += d.mid(pos, chars) + "\r\n";
        } else {
            rv += d.mid(pos, chars).toBase64() + "\r\n";
        }
    }

    if (isMultipart) {
        QMutableMapIterator<QString, QxtMailAttachment> it(qxt_d->attachments);
        while (it.hasNext()) {
            rv += "--" + qxt_d->boundary + "\r\n";
            rv += it.next().value().mimeData();
        }
        rv += "--" + qxt_d->boundary + "--\r\n";
    }

    return rv;
}

const QByteArray& QxtMailAttachment::rawData() const
{
    if (qxt_d->content == 0)
    {
        qWarning("QxtMailAttachment::rawData(): Content not set!");
        static QByteArray defaultRv;
        return defaultRv;
    }
    if (qobject_cast<QBuffer *>(qxt_d->content.data()) == 0)
    {
        // content isn't hold in a buffer but in another kind of QIODevice
        // (probably a QFile...). Read the data and cache it into a buffer
        static QByteArray empty;
        QIODevice* c = content();
        if (!c->isOpen() && !c->open(QIODevice::ReadOnly))
        {
            qWarning() << "QxtMailAttachment::rawData(): Cannot open content for reading";
            return empty;
        }
        QBuffer* cache = new QBuffer();
        cache->open(QIODevice::WriteOnly);
        char buf[1024];
        while (!c->atEnd())
        {
            cache->write(buf, c->read(buf, 1024));
        }
        if (qxt_d->deleteContent && qxt_d->content)
        qxt_d->content->deleteLater();
        qxt_d->content = cache;
        qxt_d->deleteContent = true;
    }
    return qobject_cast<QBuffer *>(qxt_d->content.data())->data();
}


// gives only a hint, based on content-type value.
// return true if the content-type corresponds to textual data (text/*, application/xml...)
// and false if unsure, so don't interpret a 'false' response as 'it's binary data...
bool QxtMailAttachment::isText() const
{
    return isTextMedia(contentType());
}

bool QxtMailAttachment::isMultipart() const
{
    return qxt_d->attachments.count() || qxt_d->contentType.startsWith("multipart/", Qt::CaseInsensitive);
}

QxtMailAttachment QxtMailAttachment::fromFile(const QString& filename)
{
    QxtMailAttachment rv(new QFile(filename));
    rv.setDeleteContent(true);
    return rv;
}
