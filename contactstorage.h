#ifndef CONTACTSTORAGE_H
#define CONTACTSTORAGE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

class QJsonArray;

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

class ContactStorage : public QObject
{
    Q_OBJECT
public:    
    explicit ContactStorage(const QSqlDatabase& db, const QString& table);
    ~ContactStorage();

public slots:
    void filter(const QString& filter);
    void update(const QJsonArray& contacts);

signals:
    // Only Qt::BlockingQueuedConnection works with QSqlQuery, otherwise
    // signal not received
    void filtered(const QSqlQuery& q);
    void updated();

private:
    QSqlQuery _query(const QString& filter_);
    void _update(const QJsonArray& contacts);

    QSqlDatabase                m_db;
    const QString               m_table;
};

#endif // CONTACTSTORAGE_H
