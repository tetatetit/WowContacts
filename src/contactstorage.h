#ifndef CONTACTSTORAGE_H
#define CONTACTSTORAGE_H

#include <QObject>
#include <QPixmap>
#include <QSqlDatabase>
#include <QSqlQuery>

class QJsonArray;
class QSize;

// This class always operates in own dedicated thread
// communicating with the world only via signals/slots
// what also ensure all calls/communications are
// async, queued and always in the thread of receiver
// i.e. completely thread-safe that nothing inside this
// class is called/accessed simultaneously concurrently,
// and non-blocking for both sides except filtered(...)
// signal of this class which blocks only this class thread
// which is acceptable since handling of this signal by receiver
// usually is very fast since involves just displaying of already
// calculated results in GUI
// NOTE: Never call slots of this class directly but always via
// emiting signal connected to it

struct ContactDetails
{
    QString first, last, sex, country, lang;
    time_t birth;
};
Q_DECLARE_METATYPE(ContactDetails);

class ContactStorage : public QObject
{
    Q_OBJECT
public:    
    explicit ContactStorage(const QSqlDatabase& db, const QString& table);
    ~ContactStorage();

    enum FilterColumn {
         FILTER_COL_avatar,
         FILTER_COL_first,
         FILTER_COL_last,
         FILTER_COL_group,
         FILTER_COL_order,
         FILTER_COL_sex,
         FILTER_COL_id
    };

    static QPixmap generateAvatar(const QSize& size, const QString& first, const QString& last, const QString& sex);

public slots:
    void filter(const QString& filter);
    void update(const QJsonArray& contacts);
    void details(const QString& id);

signals:
    void filtered(const QSqlQuery& q);
    void detailsReady(const ContactDetails& details);
    void updated();

private:
    QSqlDatabase                m_db;
    const QString               m_table;
};

#endif // CONTACTSTORAGE_H
