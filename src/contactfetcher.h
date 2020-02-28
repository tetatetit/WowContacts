#ifndef CONTACTSFETCHER_H
#define CONTACTSFETCHER_H

#include <QThread>
#include <QSqlDatabase>

// TODO: error handling

class ContactsModel;
class QIODevice;

// Self deleted thread, always create it with just `new`,
// never in stack and never keep it in smart pointer
class ContactFetcher : public QThread
{
Q_OBJECT
public:
    // Takes ownership on reader by connecting it QObject::deleteLater()
    // to this QThread::finished()
    explicit ContactFetcher(QIODevice* pReader);
signals:
    void fetched(const QJsonArray& contats);
private:
    void run() final override;

    QIODevice* m_pReader;
};


#endif // CONTACTSFETCHER_H
