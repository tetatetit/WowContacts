#include "contactstorage.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QThread>
#include <QJsonArray>
#include <QSet>

#define user "user"
#define first "first"
#define last "last"
#define sex "sex"
#define country "country"
#define lang "lang"
#define birth "birth"
#define group "group"
#define groupOrder "groupOrder"

// TODO: error handling

ContactStorage::ContactStorage(const QSqlDatabase& db, const QString& table) :
    m_db(db),
    m_table(table) {
    Q_ASSERT(m_db.open());

    QSqlQuery q(m_db);
    q.prepare(QString() +
        "CREATE TABLE IF NOT EXISTS '"+m_table+"' (\n" \
        "  '" user "'        TEXT NOT NULL PRIMARY KEY,\n" \
        "  '" first "'       TEXT NOT NULL,\n" \
        "  '" last "'        TEXT NOT NULL,\n" \
        "  '" sex "'         TEXT NOT NULL,\n" \
        "  '" country "'     TEXT NOT NULL,\n" \
        "  '" lang "'        TEXT NOT NULL,\n" \
        "  '" birth "'       INTEGER NOT NULL,\n" \
        "  '" group "'       TEXT NOT NULL,\n" \
        "  '" groupOrder "'  INTEGER NOT NULL\n" \
        ");"
    );
    Q_ASSERT(q.exec());
}

ContactStorage::~ContactStorage()
{
    m_db.close();
}

void ContactStorage::filter(const QString& filter)
{
    emit filtered(_query(filter));
}
void ContactStorage::update(const QJsonArray& contacts)
{
    _update(contacts);
    emit updated();
}


QSqlQuery ContactStorage::_query(const QString& filter_)
{
    //TODO: improve so it would handle mitple words in the field
    // e.g. people might have double/multiple first and and last names
    // (typlically for Spain), e.g.
    // first: "Maria Rosa"
    // last: "Servantes Garcia"
    // This is more rare and less used but Google Contacts till handle it
    // even when there are different amount of spaces beween words in the field
    // and in filter text
    // (thought for Chineese and Japanesse athere might be still another story :)
    QSet<QString> filter;
    {
        auto list = filter_.split(" ", QString::SkipEmptyParts);
        foreach(auto&& w, list) {
            filter.insert(w + "%");
        }
    }
    QString s =
        " SELECT " first ", " last " " \
        "   FROM ";s += m_table;
    auto wordCount = filter.size();
    if(wordCount) {
        s += " WHERE ";
    }
    for(decltype(wordCount) n = 0; n < wordCount; ++ n) {
        if(n) {
            s += " AND ";
        }
        s += "( ( " first " LIKE ? ) OR ( " last " LIKE ? ) OR ( " user " LIKE ? ) )";
    }
    s +=
        "  ORDER BY " \
        "  " groupOrder " ASC, " \
        "  " first " ASC, " \
        "  " last " ASC ";

    QSqlQuery q(m_db);
    if(!q.prepare(s)) {
        qDebug() << q.lastError();
        Q_ASSERT(false);
    }

    int wordN = 0;
    foreach(auto&& w, filter) {
        for(int colN = 0; colN < 3; ++ colN) {
            q.bindValue(colN + wordN * 3, w);
        }
        ++ wordN;
    }

    if(!q.exec()) {
        qDebug() << q.lastError();
        Q_ASSERT(false);
    }
    return q;
}

void ContactStorage::_update(const QJsonArray& contacts)
{
    QSqlQuery q(m_db);

    // Without this - executing lot of insert/update queries in batch - works extremely slower
    if(!m_db.transaction()) {
        qDebug() << m_db.lastError();
        Q_ASSERT(false);
    }
    Q_ASSERT(q.prepare(QString() +
        "INSERT OR REPLACE INTO '"+m_table+"' values (\n" \
        "  :" user ", :" first ", :" last ", :" sex ", :" country ", :" lang ", :" birth ", :" group ", :" groupOrder "\n" \
        ");"
    ));
    auto pThread = QThread::currentThread();
    foreach(auto c, contacts) {
        auto ac = c["account"];
        q.bindValue(":" user, ac["username"]);
        q.bindValue(":" first, ac["firstName"]);
        q.bindValue(":" last, ac["lastName"]);
        q.bindValue(":" sex, ac["sex"]);
        q.bindValue(":" country, ac["country"]);
        q.bindValue(":" lang, ac["language"]);
        q.bindValue(":" birth, static_cast<qint64>(ac["birthday"].toDouble()));
        q.bindValue(":" group, c["group"]);
        q.bindValue(":" groupOrder, c["groupOrder"].toInt());
        if(!q.exec()) {
            qDebug() << q.lastError();
            Q_ASSERT(false);
        }
        if(pThread->isInterruptionRequested()) {
            qDebug() << "Update/insert interrupteion requested for working thread";
            break;
        }
    }
    q.clear();
    if(!m_db.commit()) {
        qDebug() << q.lastError();
        Q_ASSERT(false);
    }
}
