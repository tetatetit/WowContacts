#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "ui_mainwindow.h"
#include "contactmodel.h"

#include <QApplication>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSharedPointer>

// TODO: error handling

class ContactStorage;

class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(int argc, char *argv[], QObject *parent = nullptr);
    ~MainController();

    int exec() { return m_app.exec(); }

signals:
    void contactsFilter(const QString& filter);
    void contactsUpdate(const QJsonArray& contacts);
    void contactDetails(const QString& user);

public slots:
    void contactsDownload();
    void contactsDownloading(QNetworkReply* reply);
    void contactsDownloaded(const QJsonArray&);
    void contactsUpdated();
    void contactsFiltering(const QString& filter);
    void contactsFiltered(const QSqlQuery& q);
    void contactDetailsShow(const QModelIndex& contactModelIndex);
    void contactDetailsReady(const ContactDetails& details);

private:
    // App
    QApplication                m_app;

    // UI
    Ui::MainWindow              m_mainUI;
    QMainWindow                 m_mainWnd;

    // UI model
    ContactModel                m_contactModel;

    // Network
    QNetworkAccessManager       m_netMgr;

    // Storage
    QThread*                    m_pStorageThread;
};

#endif // MAINCONTROLLER_H
