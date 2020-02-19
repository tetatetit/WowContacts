#ifndef CONTACTSFETCHER_H
#define CONTACTSFETCHER_H

#include <QThread>
#include <QSqlDatabase>


class ContactsModel;
class QIODevice;

// Self deleted thread, always create it with just `new`,
// never in stack and never keep it in smart pointer
class ContactsFetcher : public QThread
{
Q_OBJECT
public:
    // Takes ownership on reader by connecting it QObject::deleteLater()
    // to this QThread::finished()
    explicit ContactsFetcher(QIODevice* pReader);
signals:
    void fetched(const QJsonArray& contats);
private:
    void run() final override;

    QIODevice* m_pReader;
};


#endif // CONTACTSFETCHER_H
