/**
 * Copyright (c) 2016-2017, Daniel "Dadie" Korner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Neither the source code nor the binary may be used for any military use.
 *
 * THIS SOFTWARE IS PROVIDED BY Daniel "Dadie" Korner ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Daniel "Dadie" Korner BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

// Qt
#include <QAbstractItemModel>
#include <QIcon>
#include <QModelIndex>
#include <QPixmap>
#include <QVariant>

// Own
#include "data/configuration.h"
#include "data/measurement.h"
#include "data/probe.h"
#include "data/project.h"
#include "model/measurementtreemodel.h"

// StdLib
#include <iostream>
#include <memory>

MeasurementTreeModel::MeasurementTreeModel(
    std::shared_ptr< Configuration > configuration,
    std::shared_ptr< Project > project, QObject* parent)
    : QAbstractItemModel(parent)
{
    this->_configuration = configuration;
    this->_project = project;
}

bool MeasurementTreeModel::setData(
    const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::CheckStateRole) {
        return false;
    }

    size_t pos = 0;
    for (auto& measurement : this->_project->measurements) {
        if (measurement.get() == index.internalPointer()) {
            // Found a Measurement!
            bool isAnyInvisible = false;
            for (const auto visible : measurement->visible) {
                isAnyInvisible |= !visible;
            }
            for (size_t i = 0; i < measurement->visible.size(); ++i) {
                measurement->visible[ i ] = isAnyInvisible;
            }
            emit dataChanged(
                index, this->index(measurement->visible.size(), 3, index));
            this->_project->updatedMeasurement(measurement, pos);
            return true;
        }
        if (measurement->reader.get() == index.internalPointer()) {
            // Found a Reader
            measurement->visible[ static_cast< size_t >(index.row()) ] =
                !measurement->visible[ static_cast< size_t >(index.row()) ];
            emit dataChanged(index.parent(), index);
            this->_project->updatedMeasurement(measurement, pos);
            return true;
        }
        ++pos;
    }
    return false;
}

QVariant MeasurementTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    // Search for Object from QModelIndex
    for (size_t i = 0; i < this->_project->measurements.size(); ++i) {
        auto measurement = this->_project->measurements.at(i);
        if (measurement.get() == index.internalPointer()) {
            // FOUND it!
            if (role == Qt::CheckStateRole) {
                bool anyVisible = false;
                bool anyInvisible = false;
                for (const auto visible : measurement->visible) {
                    anyInvisible |= !visible;
                    anyVisible |= visible;
                }
                if (!anyVisible) {
                    return QVariant(Qt::Unchecked);
                }
                if (anyInvisible) {
                    return QVariant(Qt::PartiallyChecked);
                }
                return QVariant(Qt::Checked);
            }
            else if (role == Qt::DisplayRole) {
                return QVariant(measurement->name);
            }
            else {
                return QVariant();
            }
        }
        if (measurement->reader.get() == index.internalPointer()) {
            // FOUND it!
            if (role == Qt::CheckStateRole && index.column() == 0) {
                if (measurement->visible.at(
                        static_cast< size_t >(index.row()))) {
                    return QVariant(Qt::Checked);
                }
                return QVariant(Qt::Unchecked);
            }
            else if (role == Qt::DisplayRole) {
                return QVariant(measurement->sensorName.at(
                    static_cast< size_t >(index.row())));
            }
            else if (role == Qt::DecorationRole) {
                QPixmap pixmap(10, 10);
                {
                    pixmap.fill(measurement->color.at(
                        static_cast< size_t >(index.row())));
                }
                return QVariant(QIcon(pixmap));
            }
            return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags MeasurementTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;
}

QVariant MeasurementTreeModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return QVariant(QObject::tr("Measurement"));
    }
    return QVariant();
}

QModelIndex MeasurementTreeModel::index(
    int row, int column, const QModelIndex& parent) const
{
    if (!this->hasIndex(row, column, parent)) {
        // Invalid Index, return invalid Index!
        return QModelIndex();
    }

    if (!parent.isValid()) {
        return this->createIndex(row, column,
            this->_project->measurements.at(static_cast< size_t >(row)).get());
    }
    else {
        // Its probably a Sensor (to be accessed)
        Measurement* measurement =
            static_cast< Measurement* >(parent.internalPointer());
        if (static_cast< size_t >(row) >=
            measurement->reader->sensors().size()) {
            // Invalid Index, cant access the requested row
            return QModelIndex();
        }
        return this->createIndex(row, column, measurement->reader.get());
    }
}

QModelIndex MeasurementTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    // Search for Object from QModelIndex
    for (size_t i = 0; i < this->_project->measurements.size(); ++i) {
        auto measurement = this->_project->measurements.at(i);
        if (measurement.get() == index.internalPointer()) {
            // FOUND it! Parent is sadly invalid (root)
            return QModelIndex();
        }
        if (measurement->reader.get() == index.internalPointer()) {
            // FOUND it! Parent is measurement index :D
            return this->createIndex(i, 0, measurement.get());
        }
    }
    // No parent found oô?! The fuq?!
    return QModelIndex();
}

int MeasurementTreeModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return this->_project->measurements.size();
    }
    // Search for Object from QModelIndex
    for (auto& measurement : this->_project->measurements) {
        if (measurement.get() == parent.internalPointer()) {
            // FOUND it!
            return measurement->reader->sensors().size();
        }
        if (measurement->reader.get() == parent.internalPointer()) {
            // FOUND it!
            return 0;
        }
    }
    return this->_project->measurements.size();
}

int MeasurementTreeModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

void MeasurementTreeModel::selectedChanged()
{
    emit dataChanged(QModelIndex(), QModelIndex());
}

void MeasurementTreeModel::projectChanged()
{
    emit this->dataChanged(QModelIndex(),
        this->index(this->rowCount() - 1, this->columnCount() - 1));
    emit this->layoutChanged();
}

// Project / Measurment / Probes Slots
void MeasurementTreeModel::addedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}
void MeasurementTreeModel::updatedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}
void MeasurementTreeModel::removedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}

void MeasurementTreeModel::updatedProject()
{
    this->projectChanged();
}

void MeasurementTreeModel::addedProbe(std::shared_ptr< Probe > p, size_t index)
{
    this->projectChanged();
}
void MeasurementTreeModel::updatedProbe(
    std::shared_ptr< Probe > p, size_t index)
{
    this->projectChanged();
}
void MeasurementTreeModel::removedProbe(
    std::shared_ptr< Probe > p, size_t index)
{
    this->projectChanged();
}
