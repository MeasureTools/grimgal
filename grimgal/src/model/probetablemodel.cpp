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
#include <QMap>
#include <QModelIndex>
#include <QTextStream>
#include <QVariant>
#include <QVector>

// Own
#include "data/configuration.h"
#include "data/measurement.h"
#include "data/project.h"
#include "model/probetablemodel.h"
#include "util/number_format.h"

// StdLib
#include <iostream>
#include <memory>

ProbeTableModel::ProbeTableModel(std::shared_ptr< Configuration > configuration,
    std::shared_ptr< Project > project, QObject* parent)
    : QAbstractTableModel(parent)
{
    this->_configuration = configuration;
    this->_project = project;
    this->_probe_resolution = 1;
}

void ProbeTableModel::projectChanged()
{
    this->_value_cache.clear();
    emit this->dataChanged(QModelIndex(),
        this->index(this->rowCount() - 1, this->columnCount() - 1));
    emit this->layoutChanged();
}

void ProbeTableModel::setResolution(int_fast32_t newResolution)
{
    this->_probe_resolution = newResolution;
    this->_value_cache.clear();
    emit this->dataChanged(QModelIndex(),
        this->index(this->rowCount() - 1, this->columnCount() - 1));
    emit this->layoutChanged();
}

void ProbeTableModel::sectionClicked(int section)
{
    emit this->goTo(
        this->_project->probes.at(static_cast< size_t >(section))->time);
}

void ProbeTableModel::addedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}
void ProbeTableModel::updatedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}
void ProbeTableModel::removedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}

void ProbeTableModel::updatedProject()
{
    this->projectChanged();
}

void ProbeTableModel::addedProbe(std::shared_ptr< Probe > p, size_t index)
{
    this->projectChanged();
}
void ProbeTableModel::updatedProbe(std::shared_ptr< Probe > p, size_t index)
{
    this->projectChanged();
}
void ProbeTableModel::removedProbe(std::shared_ptr< Probe > p, size_t index)
{
    this->projectChanged();
}

bool ProbeTableModel::setData(
    const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole || role == Qt::DisplayRole) {
        if (index.row() == 0) {
            this->_value_cache[ index.column() ].clear();
            this->_value_cache[ index.column() ][ 0 ] = value.toDouble();
            this->_project->probes[ static_cast< size_t >(index.column()) ]
                ->time = value.toDouble();
            emit this->dataChanged(QModelIndex(),
                this->index(this->rowCount() - 1, this->columnCount() - 1),
                QVector< int >(role));
            return true;
        }
    }
    return false;
}

QVariant ProbeTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {

        if (index.row() == 0) {
            return QVariant(
                util::format_time(this->_project->probes
                                      .at(static_cast< size_t >(index.column()))
                                      ->time));
        }
        else if (this->_value_cache.contains(index.column()) &&
                 this->_value_cache[ index.column() ].contains(index.row())) {
            auto value = this->_value_cache[ index.column() ][ index.row() ];
            auto unit = this->_unit_cache[ index.column() ][ index.row() ];
            return QVariant(util::format_number(value, unit));
        }
        else {
            int row = index.row() - 1;
            for (auto& measurement : this->_project->measurements) {
                if (static_cast< size_t >(row) <
                    measurement->reader->sensors().size()) {
                    size_t datumIndex = static_cast< size_t >(row);
                    double time = this->_project->probes
                                      .at(static_cast< size_t >(index.column()))
                                      ->time +
                                  measurement->offsetX.at(datumIndex);
                    auto datum = measurement->reader->sample(
                        time, int_fast32_t(this->_probe_resolution));
                    if (datum.values.size() != measurement->offsetY.size()) {
                        return QVariant("---");
                    }
                    auto value = datum.values.at(datumIndex) +
                                 measurement->offsetY.at(datumIndex);
                    this->_value_cache[ index.column() ].insert(
                        index.row(), value);
                    auto unit = QString::fromStdString(
                        measurement->reader->sensors()[ datumIndex ].unit);
                    this->_unit_cache[ index.column() ].insert(
                        index.row(), unit);
                    return QVariant(util::format_number(value, unit));
                }
                row -= measurement->reader->sensors().size();
            }
        }
        return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags ProbeTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    if (index.row() == 0) {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
}

QVariant ProbeTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            QString header;
            QTextStream(&header) << QObject::tr("Probe ") << section;
            return QVariant(header);
        }
        if (orientation == Qt::Vertical) {
            QString header;
            if (section <= 0) {
                QTextStream(&header) << QObject::tr("Time");
            }
            else {
                size_t row = 1;
                for (auto& measurement : this->_project->measurements) {
                    if (row + measurement->reader->sensors().size() >
                        static_cast< size_t >(section)) {
                        size_t index = static_cast< size_t >(section) - row;
                        QTextStream(&header)
                            << measurement->name << " - "
                            << measurement->sensorName.at(index);
                        break;
                    }
                    row += measurement->reader->sensors().size();
                }
            }
            return QVariant(header);
        }
    }
    return QVariant();
}

int ProbeTableModel::rowCount(const QModelIndex& parent) const
{
    int rows = 1;
    for (auto& measurement : this->_project->measurements) {
        rows += measurement->reader->sensors().size();
    }
    return rows;
}

int ProbeTableModel::columnCount(const QModelIndex& parent) const
{
    return static_cast< int >(this->_project->probes.size());
}
