#ifndef CONTACTMODEL_H
#define CONTACTMODEL_H

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

private slots:
    void _updateMap();

private:
    int rowCount(const QModelIndex&) const final override;
    int columnCount(const QModelIndex&) const final override;
    QModelIndex parent(const QModelIndex&) const final override { return QModelIndex(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final ;
    void fetchMore(const QModelIndex& parent) final override { sourceModel()->fetchMore(parent); _updateMap(); }
    QModelIndex index(int row, int column, const QModelIndex& parent) const final override;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    enum ColN {
        COL_first,
        COL_last,
        COL_VISIBLE_COUNT,
        COL_group = COL_VISIBLE_COUNT,
        COL_order
    };
    enum {
        MAP_IDX_GROUP = -1
    };

    QVector<int> m_map;
};

#endif // CONTACTMODEL_H
