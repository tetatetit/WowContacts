#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

#include "contactstorage.h"

#include <QIdentityProxyModel>
#include <QVector>

class QSqlQuery;

class ContactModel : public QIdentityProxyModel
{
Q_OBJECT
typedef ContactModel base;
public:
    ContactModel();

    void setQuery(const QSqlQuery &query);
    QString getUser(const QModelIndex& index) const;

    enum ColN
    {
        COL_avatar,
        COL_first,
        COL_last,
        COL_COUNT,
        COL_INVALID
    };

private slots:
    void _updateMap();

private:
    int rowCount(const QModelIndex& parent = QModelIndex()) const final override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QModelIndex parent(const QModelIndex&) const final override { return QModelIndex(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final ;
    void fetchMore(const QModelIndex& parent) final override { sourceModel()->fetchMore(parent); _updateMap(); }
    QModelIndex index(int row, int column, const QModelIndex& parent) const final override;
    Qt::ItemFlags flags(const QModelIndex &index) const final override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const final override;

    bool isValid(int row, int col) const;
    static int colToSrc(int col);

    enum ROW_MAP_TO_SRC
    {
        ROW_MAP_TO_SRC_group = -1,
    };

    QVector<int> m_rowMapToSrc;
};

#endif // CONTACTMODEL_H
