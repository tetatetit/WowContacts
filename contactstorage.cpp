
#include "contactstorage.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QThread>
#include <QJsonArray>
#include <QPainter>

#define user "user"
#define first "first"
#define last "last"
#define sex "sex"
#define country "country"
#define lang "lang"
#define birth "birth"
#define group "group_"
#define groupOrder "groupOrder"
#define avatar "avatar"

// TODO: error handling in whole file

ContactStorage::ContactStorage(const QSqlDatabase& db, const QString& table) :
    m_db(db),
    m_table(table) {
    Q_ASSERT(m_db.open());

    QSqlQuery q(m_db);
    q.prepare(QString() +
        "CREATE TABLE IF NOT EXISTS '"+m_table+"' (\n" \
        "  '" user "'        TEXT NOT NULL PRIMARY KEY,\n" \
        "  '" first "'       TEXT,\n" \
        "  '" last "'        TEXT,\n" \
        "  '" sex "'         TEXT,\n" \
        "  '" country "'     TEXT,\n" \
        "  '" lang "'        TEXT,\n" \
        "  '" birth "'       INTEGER,\n" \
        "  '" group "'       TEXT,\n" \
        "  '" groupOrder "'  INTEGER,\n" \
        "  '" avatar "'      BLOB\n" \
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
    // so when filter input is e.g. "Maria Ro" - it will not be found
    // due words are with first name firld.
    // This is more rare case and less used but Google Contacts till handle it
    // even when there are different amount of spaces beween words in the field
    // and in filter text
    // anyway it's definely fully solvable with sqlite3-pcre extension
    // or otherwise the only solution to go away from sqlite to some custom written
    QSet<QString> filter;
    {
        auto list = filter_.split(" ", QString::SkipEmptyParts);
        foreach(auto&& w, list) {
            filter.insert(w + "%");
        }
    }
    QString s =
        " SELECT " avatar ", " first ", " last ", " group ", " groupOrder ", " sex " "\
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
    // TODO: more clear decision need to be made and implemented
    //       whether to clear table before upading and then just insert
    //       all contacts from scratch or to keep current approach assuming
    //       that only new and updated contacts are provided in above
    //       source `contacts` list
    // TODO: Perform validation when adding
    QSqlQuery q(m_db);

    // Without this - executing lot of insert/update queries in batch - works extremely slower
    if(!m_db.transaction()) {
        qDebug() << m_db.lastError();
        Q_ASSERT(false);
    }
    Q_ASSERT(q.prepare(QString() +
        "INSERT OR REPLACE INTO '"+m_table+"' values (\n" \
        "  :" user ", :" first ", :" last ", :" sex ", :" country ", :" lang ", :" birth ", :" group ", :" groupOrder ", :" avatar "\n" \
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

QImage ContactStorage::generateAvatar(const QRect& rect, const QString& first_, const QString& last_, const QString& sex_)
{
      QColor fillColor(sex_ == "FEMALE" ? "#FCD0FC" :
                       sex_ == "MALE"   ? "#B5E6FF" :
                                          "#E1E8ED");
      QImage img(rect.size(), QImage::Format_RGB32);
      QPainter paint;
      const int divX = 32,
                divY = 32;
      paint.begin(&img);
      paint.fillRect(rect, "white");
      paint.setBrush(fillColor);
      paint.setPen(fillColor);
      paint.drawEllipse(rect.x(), rect.y(), rect.width() - 1, rect.height() - 1);
      paint.setPen("green");
      if(!first_.isEmpty()) {
          paint.drawText(rect.width() * 8 / divX, rect.height() * 17 / divY, first_.left(1).toUpper());
      }
      if(!last_.isEmpty()) {
          paint.drawText(rect.width() * 18 / divX, rect.height() * 21 / divY, last_.left(1).toUpper());
      }
      paint.end();
      return img;
}
