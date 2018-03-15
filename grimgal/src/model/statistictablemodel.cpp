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
#include <QMetaType>
#include <QModelIndex>
#include <QTextStream>
#include <QVariant>

// Own
#include "data/configuration.h"
#include "data/measurement.h"
#include "data/project.h"
#include "model/statistictablemodel.h"
#include "util/number_format.h"
#include <rlib/common/event_data.h>

// StdLib
#include <algorithm>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

StatisticTableModel::StatisticTableModel(
    std::shared_ptr< Configuration > configuration,
    std::shared_ptr< Project > project, QObject* parent)
    : QAbstractTableModel(parent)
{
    this->_configuration = configuration;
    this->_project = project;
}

void StatisticTableModel::projectChanged()
{
    emit this->dataChanged(QModelIndex(),
        this->index(this->rowCount() - 1, this->columnCount() - 1));
    emit this->layoutChanged();
}

void StatisticTableModel::addedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}
void StatisticTableModel::updatedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}
void StatisticTableModel::removedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->projectChanged();
}

void StatisticTableModel::updatedProject()
{
    this->projectChanged();
}

void StatisticTableModel::addedProbe(std::shared_ptr< Probe > p, size_t index)
{
}
void StatisticTableModel::updatedProbe(std::shared_ptr< Probe > p, size_t index)
{
}
void StatisticTableModel::removedProbe(std::shared_ptr< Probe > p, size_t index)
{
}

QVariant StatisticTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        int row = 0;
        for (auto& measurement : this->_project->measurements) {
            if (row + measurement->sensorName.size() > index.row()) {
                int datumIndex = index.row() - row;
                if (index.column() == 0) {
                    return QVariant(measurement->name + " - " +
                                    measurement->sensorName[ datumIndex ]);
                }

                rlib::common::statistic_data statisticColumn;
                switch (index.column()) {
                    case 1:
                        statisticColumn =
                            rlib::common::statistic_data::MIN_VALUE;
                        break;
                    case 2:
                        statisticColumn =
                            rlib::common::statistic_data::MAX_VALUE;
                        break;
                    case 3:
                        statisticColumn =
                            rlib::common::statistic_data::AVG_VALUE;
                        break;
                    case 4:
                        statisticColumn =
                            rlib::common::statistic_data::MEDIAN_VALUE;
                        break;
                    case 5:
                        statisticColumn =
                            rlib::common::statistic_data::VAR_VALUE;
                        break;
                    default:
                        return QVariant("");
                }
                auto statistics =
                    measurement->reader->statistic(statisticColumn);
                if (statistics.size() > datumIndex &&
                    statistics[ datumIndex ]) {
                    auto value = statistics[ datumIndex ].value();
                    auto unit = QString::fromStdString(
                        measurement->reader->sensors()[ datumIndex ].unit);
                    return QVariant(util::format_number(value, unit));
                }
                return QVariant("");
            }
            row += measurement->reader->sensors().size();
        }
    }

    return QVariant();
}

Qt::ItemFlags StatisticTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
}

QVariant StatisticTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            QString header;
            if (section == 0) {
                QTextStream(&header) << QObject::tr("Sensor");
            }
            if (section == 1) {
                QTextStream(&header) << QObject::tr("Min");
            }
            if (section == 2) {
                QTextStream(&header) << QObject::tr("Max");
            }
            if (section == 3) {
                QTextStream(&header) << QObject::tr("AVG");
            }
            if (section == 4) {
                QTextStream(&header) << QObject::tr("Median");
            }
            if (section == 5) {
                QTextStream(&header) << QObject::tr("Var");
            }
            return QVariant(header);
        }
    }
    return QVariant();
}

int StatisticTableModel::rowCount(const QModelIndex& parent) const
{
    int row = 0;
    for (auto& measurement : this->_project->measurements) {
        row += measurement->reader->sensors().size();
    }
    return row;
}

int StatisticTableModel::columnCount(const QModelIndex& parent) const
{
    return 7;
}
