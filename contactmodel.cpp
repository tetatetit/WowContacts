#include "contactmodel.h"

#include <QSqlQueryModel>
#include <QDebug>

ContactModel::ContactModel()
{
    setSourceModel(new QSqlQueryModel(this));//self deleted source model
    connect(this, &QAbstractItemModel::modelReset, this, &ContactModel::_updateMap);
}

void ContactModel::setQuery(const QSqlQuery &query)
{
    dynamic_cast<QSqlQueryModel*>(sourceModel())->setQuery(query);
}

int ContactModel::rowCount(const QModelIndex&) const
{
    return m_map.count();
}

int ContactModel::columnCount(const QModelIndex&) const
{
    return COL_VISIBLE_COUNT;
}

QVariant ContactModel::data(const QModelIndex &index, int role) const
{
    auto srcRow = m_map[index.row()];
    if(srcRow == -1) {
        if(role == Qt::DisplayRole && index.column() == COL_first) {// just to display in first column
            Q_ASSERT(index.row() < m_map.count());// must exist at least one item if group row inserted
            return sourceModel()->data(sourceModel()->index(m_map[index.row() + 1], COL_group)).toString();
        }
    }

    return sourceModel()->data(createIndex(srcRow, index.column()), role);
}

QModelIndex ContactModel::index(int row, int column, const QModelIndex&) const
{
    return createIndex(row, column);
}

void ContactModel::_updateMap()
{
    auto    pModel = sourceModel();
    int     rowCount = pModel->rowCount(),
            prevGroupOrder = -1;

    m_map.clear();
    // TODO: incorporate std::upper_bound here for optimization
    for(int rowN = 0; rowN < rowCount; ++ rowN) {
        auto groupOrder = pModel->data(pModel->index(rowN, COL_order)).toInt();
        if(groupOrder != prevGroupOrder) {
            prevGroupOrder = groupOrder;
            m_map.append(-1);
        }
        m_map.append(rowN);
    }
}

