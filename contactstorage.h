#ifndef CONTACTSTORAGE_H
#define CONTACTSTORAGE_H

#include <QObject>
#include <QThread>

class ContactStorageRead;
class ContactStorageWrite;
class QSqlDatabase;
class QSqlQuery;

class ContactStorage : public QObject
{
    Q_OBJECT
public:
    explicit ContactStorage(const QSqlDatabase& dbRd, const QSqlDatabase& dbWr, const QString& table);
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
    QThread                     m_thread;
    ContactStorageWrite* const  m_pStorWr;
    ContactStorageRead* const   m_pStorRd;
};

#endif // CONTACTSTORAGE_H
