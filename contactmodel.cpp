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
    return m_map.count();
}

int ContactModel::columnCount(const QModelIndex&) const
{
    return COL_VISIBLE_COUNT;
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
    auto srcRow = m_map[index.row()];
    if(srcRow == MAP_IDX_GROUP) { // if group row
        if(role == Qt::DisplayRole && index.column() == COL_first) {// just to display in first column
            Q_ASSERT(index.row() < m_map.count());// must exist at least one item if group row inserted
            return sourceModel()->data(sourceModel()->index(m_map[index.row() + 1], COL_group)).toString();
        }
        return QVariant();
    }

    if(index.column() == COL_avatar) {
        if(role == Qt::DecorationRole) {
            QString first = sourceModel()->data(sourceModel()->index(srcRow, COL_first)).toString(),
                    last = sourceModel()->data(sourceModel()->index(srcRow, COL_last)).toString(),
                    sex = sourceModel()->data(sourceModel()->index(srcRow, COL_sex)).toString();
            // TODO: move below drawing of avatar to separate function
            //       to draw any size avatar as well (not only fixed size)
            //       by setting sizes and coordinates proportionally
            //       Since it should be reused in detailed contact view
            //       with 128x128 avatar size
            QColor fillColor(sex == "FEMALE" ? "#FCD0FC" :
                             sex == "MALE"   ? "#B5E6FF" :
                                               "#E1E8ED");
            QImage img(AVATAR_CELL_SIZE, QImage::Format_RGB32);
            QPainter paint;
            paint.begin(&img);
            paint.fillRect(0, 0, AVATAR_CELL_WIDTH, AVATAR_CELL_HEIGHT, "white");
            paint.setBrush(fillColor);
            paint.setPen(fillColor);
            paint.drawEllipse(0, 0, AVATAR_CELL_WIDTH - 1, AVATAR_CELL_HEIGHT - 1);
            paint.setPen("green");
            if(!first.isEmpty()) {
                paint.drawText(8, 17, first.left(1).toUpper());
            }
            if(!last.isEmpty()) {
                paint.drawText(18, 21, last.left(1).toUpper());
            }
            paint.end();
            return img;
        } else if(role == Qt::SizeHintRole) {
            return AVATAR_CELL_SIZE;
        }
    }

    return sourceModel()->data(createIndex(srcRow, index.column()), role);
}

QModelIndex ContactModel::index(int row, int column, const QModelIndex&) const
{
    return createIndex(row, column);
}

Qt::ItemFlags ContactModel::flags(const QModelIndex &index) const
{
    if(m_map[index.row()] == MAP_IDX_GROUP) {
        return Qt::NoItemFlags;
    }

    return sourceModel()->flags(createIndex(index.row(), index.column()))
            | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void ContactModel::_updateMap()
{
    auto    pModel = sourceModel();
    int     rowCount = pModel->rowCount(),
            prevGroupOrder = MAP_IDX_GROUP;

    m_map.clear();
    // TODO: incorporate std::upper_bound here for optimization
    for(int rowN = 0; rowN < rowCount; ++ rowN) {
        auto groupOrder = pModel->data(pModel->index(rowN, COL_order)).toInt();
        if(groupOrder != prevGroupOrder) {
            prevGroupOrder = groupOrder;
            m_map.append(MAP_IDX_GROUP);
        }
        m_map.append(rowN);
    }
}

