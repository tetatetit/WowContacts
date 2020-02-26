#include "contactmodel.h"

#include <QSize>
#include <QImage>
#include <QPainter>
#include <QSqlQueryModel>
#include <QDebug>

#define AVATAR_CELL_WIDTH   32
#define AVATAR_CELL_HEIGHT  32
#define AVATAR_CELL_SIZE QSize(AVATAR_CELL_WIDTH, AVATAR_CELL_HEIGHT)

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
    return m_rowMapToSrc.count();
}

int ContactModel::columnCount(const QModelIndex&) const
{
    return COL_COUNT;
}

QVariant ContactModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section == COL_avatar && role == Qt::SizeHintRole) {
        return AVATAR_CELL_SIZE;
    }
    return sourceModel()->headerData(section, orientation, role);
}

QVariant ContactModel::data(const QModelIndex &index, int role) const
{
    if(!(index.isValid())) {
        return QVariant();
    }
    Q_ASSERT(isValid(index.row(), index.column()));

    auto srcRow = m_rowMapToSrc[index.row()];
    if(srcRow == ROW_MAP_TO_SRC_group) { // if group
        if(role == Qt::DisplayRole && index.column() == COL_first) {// just to display in first column
            return sourceModel()->data(sourceModel()->index(m_rowMapToSrc[index.row() + 1], ContactStorage::FILTER_COL_group)).toString();
        }
        return QVariant();
    }

    if(index.column() == COL_avatar) {
        if(role == Qt::DecorationRole) {
            // TODO: check if avatar available in source model (provided by storage, e.g. contact photo)
            //       and return it in that case instead of generated
            return ContactStorage::generateAvatar(
                        QRect(0, 0, AVATAR_CELL_WIDTH, AVATAR_CELL_HEIGHT),
                        sourceModel()->data(sourceModel()->index(srcRow, ContactStorage::FILTER_COL_first)).toString(),
                        sourceModel()->data(sourceModel()->index(srcRow, ContactStorage::FILTER_COL_last)).toString(),
                        sourceModel()->data(sourceModel()->index(srcRow, ContactStorage::FILTER_COL_sex)).toString()
                        );
        } else if(role == Qt::SizeHintRole) {
            return AVATAR_CELL_SIZE;
        }
        return QVariant();
    }            
    return sourceModel()->data(createIndex(srcRow, colToSrc(index.column())), role);
}

bool ContactModel::isValid(int row, int col) const
{
    if(col >= 0 && col < COL_COUNT
        && row >=0 && row < m_rowMapToSrc.count())
    {
        Q_ASSERT(m_rowMapToSrc[row] != ROW_MAP_TO_SRC_group
                    // group must not be empty, i.e. at least one contact row
                    // must exists after it in the map
                    || row + 1 < m_rowMapToSrc.count());
        return true;
    }
    return false;
}

int ContactModel::colToSrc(int col)
{
    Q_ASSERT(col < COL_COUNT);

    static const int mapColToSrc[] =
    {
        ContactStorage::FILTER_COL_avatar,
        ContactStorage::FILTER_COL_first,
        ContactStorage::FILTER_COL_last
    };
    static_assert(sizeof(mapColToSrc) / sizeof(*mapColToSrc) == COL_COUNT, "Not all columns mapped");
    return mapColToSrc[col];
}

QModelIndex ContactModel::index(int row, int column, const QModelIndex&) const
{
    if(!isValid(row, column))
        return QModelIndex();
    return createIndex(row, column);
}

Qt::ItemFlags ContactModel::flags(const QModelIndex &index) const
{
    if(!(index.isValid() && isValid(index.row(), index.column()))) {
        return Qt::NoItemFlags;
    }
    auto srcRow = m_rowMapToSrc[index.row()];
    if(srcRow == ROW_MAP_TO_SRC_group) {
        return Qt::NoItemFlags;
    }

    return sourceModel()->flags(createIndex(srcRow, colToSrc(index.column())))
            | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QString ContactModel::getUser(const QModelIndex& index) const
{
    if(!(index.isValid() && isValid(index.row(), index.column()))) {
        return "";
    }

    return sourceModel()->data(sourceModel()->index(m_rowMapToSrc[index.row()], ContactStorage::FILTER_COL_user)).toString();
}

void ContactModel::_updateMap()
{
    auto    pModel = sourceModel();
    int     rowCount = pModel->rowCount(),
            prevGroupOrder = ROW_MAP_TO_SRC_group;

    m_rowMapToSrc.clear();
    // TODO: incorporate std::upper_bound here for optimization
    for(int rowN = 0; rowN < rowCount; ++ rowN) {
        auto groupOrder = pModel->data(pModel->index(rowN, ContactStorage::FILTER_COL_order)).toInt();
        if(groupOrder != prevGroupOrder) {
            prevGroupOrder = groupOrder;
            m_rowMapToSrc.append(ROW_MAP_TO_SRC_group);
        }
        m_rowMapToSrc.append(rowN);
    }
}
