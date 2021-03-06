#include "contactfetcher.h"

#include <QIODevice>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

ContactFetcher::ContactFetcher(QIODevice* pReader) :
    m_pReader(pReader)
{
    connect(this, &QThread::finished, m_pReader, &QObject::deleteLater);
    connect(this, &QThread::finished, this, &QObject::deleteLater);// self delete
}

void ContactFetcher::run()
{
    // TODO: implement error handling (emiting error signal)
    auto d = m_pReader->readAll();
    Q_ASSERT(d.size());

    QJsonDocument json = QJsonDocument::fromJson(d);
    Q_ASSERT(!json.isNull());

    emit fetched(json["roster"].toArray());
}
