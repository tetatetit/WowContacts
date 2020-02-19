#include "maincontroller.h"

#include "contactsfetcher.h"
#include "contactstorage.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSqlDatabase>

MainController::MainController(QObject *pParent) :
    QObject(pParent)
{
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("roster.sqlite");
    m_pStorage = new ContactStorage(db, db, "contacts");

    m_mainUI.setupUi(&m_mainWnd);
    m_mainUI.contactList->setModel(&m_contactListModel);

    connect(m_mainUI.actionUpdate, &QAction::triggered, this, &MainController::contactsDownload);
    connect(&m_netMgr, &QNetworkAccessManager::finished, this, &MainController::contactsDownloading);
    connect(m_pStorage, &ContactStorage::updated, this, &MainController::contactsUpdated);
    connect(this, &MainController::contactsFilter, m_pStorage, &ContactStorage::filter);
    connect(m_mainUI.contactFilter, &QLineEdit::textEdited, m_pStorage, &ContactStorage::filter);
    connect(m_mainUI.contactFilter, &QLineEdit::textEdited, this, &MainController::contactsFiltering);
    connect(m_pStorage, &ContactStorage::filtered, this, &MainController::contactsFiltered, Qt::BlockingQueuedConnection);

    m_mainUI.statusBar->showMessage(tr("Refreshing list ..."));
    emit contactsFilter(m_mainUI.contactFilter->text());
    m_mainWnd.show();
}

void MainController::contactsDownload()
{
    m_mainUI.statusBar->showMessage(tr("Sending request to download ..."));
    m_netMgr.get(QNetworkRequest(QUrl("https://file.wowapp.me/owncloud/index.php/s/sGOXibS0ZSspQE8/download")));
}

void MainController::contactsDownloading(QNetworkReply* pReply)
{
    m_mainUI.statusBar->showMessage(tr("Downloading and parsing ..."));
    ContactsFetcher* pFetcher = new ContactsFetcher(pReply); // self deleted and takes ownership of `reply`
    connect(pFetcher, &ContactsFetcher::fetched, m_pStorage, &ContactStorage::update);
    connect(pFetcher, &ContactsFetcher::fetched, this, &MainController::contactsDownloaded);
    pFetcher->start();
}

void MainController::contactsDownloaded(const QJsonArray&)
{
    m_mainUI.statusBar->showMessage(tr("Downloaded, updating storage ..."));
}

void MainController::contactsUpdated()
{
    m_mainUI.statusBar->showMessage(tr("Storage updated, refreshing list ..."));
    emit contactsFilter(m_mainUI.contactFilter->text());
}

void MainController::contactsFiltering(const QString&)
{
    m_mainUI.statusBar->showMessage(tr("Filtering ..."));
}

void MainController::contactsFiltered(const QSqlQuery& q) {
    m_mainUI.statusBar->showMessage("Done");
    m_contactListModel.setQuery(q);
}
