
#include "contactstorage.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QThread>
#include <QJsonArray>
#include <QPainter>

#define C_USER "user"
#define C_FIRST "first"
#define C_LAST "last"
#define C_SEX "sex"
#define C_COUNTRY "country"
#define C_LANG "lang"
#define C_BIRTH "birdth"
#define C_GROUP "group_"
#define C_GROUP_ORDER "groupOrder"
#define C_AVATAR "avatar"

// TODO: error handling in whole file
//       that execution of routine should not continue
//       and "error" signal should be emited which should
//       be handled by controller appropriately

Q_DECLARE_METATYPE(QSqlQuery);// to allow it to be passed throught signal/slot

ContactStorage::ContactStorage(const QSqlDatabase& db, const QString& table) :
    m_db(db),
    m_table(table) {
    Q_ASSERT(m_db.open());
    // to allow them to be passed throught signal/slot
    qRegisterMetaType<ContactDetails>("ContactDetails");
    qRegisterMetaType<QSqlQuery>("ContactDetails");

    QSqlQuery q(m_db);
    q.prepare(QString() +
        "CREATE TABLE IF NOT EXISTS '"+m_table+"' (\n" \
        "  '" C_USER "'        TEXT NOT NULL PRIMARY KEY,\n" \
        "  '" C_FIRST "'       TEXT,\n" \
        "  '" C_LAST "'        TEXT,\n" \
        "  '" C_SEX "'         TEXT,\n" \
        "  '" C_COUNTRY "'     TEXT,\n" \
        "  '" C_LANG "'        TEXT,\n" \
        "  '" C_BIRTH "'       INTEGER,\n" \
        "  '" C_GROUP "'       TEXT,\n" \
        "  '" C_GROUP_ORDER "'  INTEGER,\n" \
        "  '" C_AVATAR "'      BLOB\n" \
        ");"
    );
    Q_ASSERT(q.exec());
}

ContactStorage::~ContactStorage()
{
    m_db.close();
}

void ContactStorage::filter(const QString& filter_)
{
    //TODO: improve so it would handle mitple words in the field
    // e.g. people might have double/multiple C_FIRST and and C_LAST names
    // (typlically for Spain), e.g.
    // C_FIRST: "Maria Rosa"
    // C_LAST: "Servantes Garcia"
    // so when filter input is e.g. "Maria Ro" - it will not be found
    // due words are with C_FIRST name firld.
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
    // when changing SELECT columns order - make sure to
    // update order in FilterColumn enum appropriately
    QString s =
        " SELECT " C_AVATAR ", " C_FIRST ", " C_LAST ", " C_GROUP ", " C_GROUP_ORDER ", " C_SEX ", " C_USER " "\
        "   FROM ";s += m_table;
    auto wordCount = filter.size();
    if(wordCount) {
        s += " WHERE ";
    }
    for(decltype(wordCount) n = 0; n < wordCount; ++ n) {
        if(n) {
            s += " AND ";
        }
        s += "( ( " C_FIRST " LIKE ? ) OR ( " C_LAST " LIKE ? ) OR ( " C_USER " LIKE ? ) )";
    }
    s +=
        "  ORDER BY " \
        "  " C_GROUP_ORDER " ASC, " \
        "  " C_FIRST " ASC, " \
        "  " C_LAST " ASC ";

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

    emit filtered(q);
}

void ContactStorage::details(const QString& user_)
{
    QSqlQuery q(m_db);

    Q_ASSERT(q.prepare(
        "SELECT " C_FIRST ", " C_LAST ", " C_SEX ", " C_COUNTRY ", " C_LANG ", " C_BIRTH \
        "   FROM " + m_table +
        "   WHERE " C_USER "=?"
    ));
    q.bindValue(0, user_);
    Q_ASSERT(q.exec());

    q.first();
    emit detailsReady(ContactDetails{
        q.value(0).toString(),
        q.value(1).toString(),
        q.value(2).toString(),
        q.value(3).toString(),
        q.value(4).toString(),
        static_cast<time_t>(q.value(5).toDouble())
    });
}

void ContactStorage::update(const QJsonArray& contacts)
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
        "  :" C_USER ", :" C_FIRST ", :" C_LAST ", :" C_SEX ", :" C_COUNTRY ", :" C_LANG ", :" C_BIRTH ", :" C_GROUP ", :" C_GROUP_ORDER ", :" C_AVATAR "\n" \
        ");"
    ));
    auto pThread = QThread::currentThread();
    foreach(auto c, contacts) {
        auto ac = c["account"];
        q.bindValue(":" C_USER, ac["username"]);
        q.bindValue(":" C_FIRST, ac["firstName"]);
        q.bindValue(":" C_LAST, ac["lastName"]);
        q.bindValue(":" C_SEX, ac["sex"]);
        q.bindValue(":" C_COUNTRY, ac["country"]);
        q.bindValue(":" C_LANG, ac["language"]);
        q.bindValue(":" C_BIRTH, static_cast<qint64>(ac["birthday"].toDouble()));
        q.bindValue(":" C_GROUP, c["group"]);
        q.bindValue(":" C_GROUP_ORDER, c["groupOrder"].toInt());
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
    emit updated();
}

QImage ContactStorage::generateAvatar(const QRect& rect, const QString& first, const QString& last, const QString& sex)
{
      QColor fillColor(sex == "FEMALE" ? "#FCD0FC" :
                       sex == "MALE"   ? "#B5E6FF" :
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
      if(!first.isEmpty()) {
          paint.drawText(rect.width() * 8 / divX, rect.height() * 17 / divY, first.left(1).toUpper());
      }
      if(!last.isEmpty()) {
          paint.drawText(rect.width() * 18 / divX, rect.height() * 21 / divY, last.left(1).toUpper());
      }
      paint.end();
      return img;
}
