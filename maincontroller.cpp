#include "maincontroller.h"

#include "contactfetcher.h"
#include "contactstorage.h"
#include "ui_contactdetails.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSqlDatabase>
#include <QThread>
#include <QHeaderView>

MainController::MainController(int argc, char *argv[], QObject *pParent) :
    QObject(pParent),
    m_app(argc, argv)
{
    // DB connection
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("roster.sqlite");

    // Storage
    auto pStorage = new ContactStorage(db, "contacts");
    m_pStorageThread = new QThread;
    // self-deleted on thread exit, all futher communications via signals/slots only
    pStorage->moveToThread(m_pStorageThread);
    connect(m_pStorageThread, &QThread::finished, pStorage, &QObject::deleteLater);
    m_pStorageThread->start();

    // TODO: move all UI initialization/interatction/connection from here
    //       to separate class just initialized from here and connected with signals

    // UI init/setup
    m_mainUI.setupUi(&m_mainWnd);
    m_mainUI.contactList->setModel(&m_contactModel);
    m_mainUI.contactList->horizontalHeader()->setSectionResizeMode(ContactModel::COL_avatar, QHeaderView::ResizeToContents);
    m_mainUI.contactList->horizontalHeader()->setSectionResizeMode(ContactModel::COL_first, QHeaderView::Stretch);
    m_mainUI.contactList->horizontalHeader()->setSectionResizeMode(ContactModel::COL_last, QHeaderView::Stretch);

    // UI <=> Controller
    connect(m_mainUI.actionUpdate, &QAction::triggered, this, &MainController::contactsDownload);
    connect(m_mainUI.contactFilter, &QLineEdit::textEdited, this, &MainController::contactsFiltering);
    connect(m_mainUI.contactList, &QAbstractItemView::clicked, this, &MainController::contactDetailsShow);

    // Storage <=> Controller
    connect(this, &MainController::contactsUpdate, pStorage, &ContactStorage::update);
    connect(pStorage, &ContactStorage::updated, this, &MainController::contactsUpdated);
    connect(this, &MainController::contactsFilter, pStorage, &ContactStorage::filter);
    connect(pStorage, &ContactStorage::filtered, this, &MainController::contactsFiltered);
    connect(this, &MainController::contactDetails, pStorage, &ContactStorage::details);
    connect(pStorage, &ContactStorage::detailsReady, this, &MainController::contactDetailsReady);

    // Network <=> Controller
    connect(&m_netMgr, &QNetworkAccessManager::finished, this, &MainController::contactsDownloading);

    // UI apply initial filter
    m_mainUI.statusBar->showMessage(tr("Refreshing list ..."));
    emit contactsFilter(m_mainUI.contactFilter->text());

    // UI show
    m_mainWnd.show();
}

MainController::~MainController()
{
    // Don't let QThread object to be destructed with still running thread
    m_pStorageThread->exit(0);
    // Wait 30 seconds to allow thread complete all write operations (if any)
    // to avoid asert in debug config, abnormal termination
    // and with very low chance sqlite DB corruption
    // TODO: in most cases it's very fast and non blocking, but still implement
    // above and below to be called from non-GUI thread while GUI showing status
    // something like "Waiting for storage operations ended ..."
    m_pStorageThread->wait(30000);
    delete m_pStorageThread;
}

void MainController::contactsDownload()
{
    m_mainUI.statusBar->showMessage(tr("Sending request to download ..."));
    m_netMgr.get(QNetworkRequest(QUrl("https://file.wowapp.me/owncloud/index.php/s/sGOXibS0ZSspQE8/download")));
}

void MainController::contactsDownloading(QNetworkReply* pReply)
{
    m_mainUI.statusBar->showMessage(tr("Downloading and parsing ..."));
    ContactFetcher* pFetcher = new ContactFetcher(pReply); // self deleted and takes ownership of `reply`
    connect(pFetcher, &ContactFetcher::fetched, this, &MainController::contactsDownloaded);
    pFetcher->start();
}

void MainController::contactsDownloaded(const QJsonArray& contacts)
{
    m_mainUI.statusBar->showMessage(tr("Downloaded, updating storage ..."));
    emit contactsUpdate(contacts);
}

void MainController::contactsUpdated()
{
    m_mainUI.statusBar->showMessage(tr("Storage updated, refreshing list ..."));
    emit contactsFilter(m_mainUI.contactFilter->text());
}

void MainController::contactsFiltering(const QString& filter)
{
    m_mainUI.statusBar->showMessage(tr("Filtering ..."));
    emit contactsFilter(filter);
}

void MainController::contactsFiltered(const QSqlQuery& q)
{
    m_mainUI.statusBar->showMessage(tr("Done"));
    m_contactModel.setQuery(q);
}

void MainController::contactDetailsShow(const QModelIndex& contactModelIndex)
{
    if(!(contactModelIndex.flags() & Qt::ItemIsEnabled))// if group
        return;
    emit contactDetails(m_contactModel.getUser(contactModelIndex));
}

void MainController::contactDetailsReady(const ContactDetails& details)
{
    Ui_Dialog dlgUI;
    QDialog dlg;

    dlgUI.setupUi(&dlg);
    dlg.exec();
}
