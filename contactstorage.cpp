#include "contactstorage.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QThread>
#include <QJsonArray>

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

class ContactStorageBase
{
public:
    explicit ContactStorageBase(const QSqlDatabase& db, const QString& table) :
        m_db(db),
        m_table(table) {
        Q_ASSERT(m_db.open());
    }
protected:
    QSqlDatabase            m_db;
    const QString           m_table;
};

class ContactStorageRead : public ContactStorageBase
{
public:
    using ContactStorageBase::ContactStorageBase;

    QSqlQuery query(const QString& filter_)
    {
        auto filter = filter_.split(" ", QString::SkipEmptyParts);
        QString s;
        const auto wordsCount = filter.count();

        for(int n = 0; n < wordsCount; ++ n) {
            if(n == 0) {
                s += " WHERE ";
            } else {
                s += " AND ";
            }
            s += "( ( " first " LIKE ? ) OR ( " last " LIKE ? ) OR ( " user " LIKE ? ) )";
        }

        QSqlQuery q(m_db);
        QString s1 = QString() +
            "SELECT " first ", " last " " \
            "  FROM "+m_table+" " \
            + s + " " \
            "  ORDER BY " \
            "  " groupOrder " ASC, " \
            "  " first " ASC, " \
            "  " last " ASC ";
        if(!q.prepare(s1)) {
            qDebug() << q.lastError();
            Q_ASSERT(false);
        }

        for(int n = 0; n < wordsCount; ++ n) {
            for(int m = 0; m < 3; ++ m) {
                q.bindValue(n * 3 + m, filter[n] + '%');
            }
        }

        if(!q.exec()) {
            qDebug() << q.lastError();
            qDebug() << s1;
            Q_ASSERT(false);
        }
        return q;
    }
};

class ContactStorageWrite : public ContactStorageBase
{
public:
    ContactStorageWrite(const QSqlDatabase& db, const QString& table) :
        ContactStorageBase(db, table)
    {
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

    void update(const QJsonArray& contacts)
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
};

ContactStorage::ContactStorage(const QSqlDatabase& dbRd, const QSqlDatabase& dbWr, const QString& table) :
    m_pStorWr(new ContactStorageWrite(dbWr, table)),
    m_pStorRd(new ContactStorageRead(dbRd, table))
{
    moveToThread(&m_thread);
    m_thread.start();
}

ContactStorage::~ContactStorage()
{
    // Don't let QThread object to be destructed with still running thread
    m_thread.exit(0);
    // Wait 30 seconds to allow thread complete all write operations (if any)
    // to avoid asert in debug config, abnormal termination
    // and with very low chance sqlite DB corruption
    // TODO: in most cases it's very fast and non blocking - still implement
    // move above and below  to separate method to be called from non-GUI thread
    m_thread.wait(30000);

    delete m_pStorRd;
    delete m_pStorWr;
}

void ContactStorage::filter(const QString& filter)
{
    QSqlQuery q = m_pStorRd->query(filter);
    emit filtered(q);
}
void ContactStorage::update(const QJsonArray& contacts)
{
    m_pStorWr->update(contacts);
    emit updated();
}
