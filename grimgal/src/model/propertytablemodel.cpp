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
#include <QModelIndex>
#include <QObject>
#include <QVariant>

// Own
#include "data/configuration.h"
#include "data/measurement.h"
#include "data/project.h"
#include "model/property/measurment_file.h"
#include "model/property/measurment_name.h"
#include "model/property/meta_comment.h"
#include "model/property/meta_line_color.h"
#include "model/property/meta_line_type.h"
#include "model/property/sensor_name.h"
#include "model/property/sensor_offset_x.h"
#include "model/property/sensor_offset_y.h"
#include "model/property/sensor_samplerate.h"
#include "model/property/sensor_unit.h"
#include "model/propertytablemodel.h"

// StdLib
#include <iostream>
#include <memory>

PropertyTableModel::PropertyTableModel(
    std::shared_ptr< Configuration > configuration,
    std::shared_ptr< Project > project, QObject* parent)
    : QAbstractTableModel(parent)
{
    this->_configuration = configuration;
    this->_project = project;
    this->_selected = nullptr;

    // Measurment Properties
    this->_measurment_properties.push_back(
        std::make_unique< model::property::measurment_file >());
    this->_measurment_properties.push_back(
        std::make_unique< model::property::measurment_name >());

    // Sensor Properties
    this->_sensor_properties.push_back(
        std::make_unique< model::property::measurment_file >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::measurment_name >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::sensor_name >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::sensor_samplerate >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::sensor_unit >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::sensor_offset_x >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::sensor_offset_y >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::meta_line_color >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::meta_line_type >());
    this->_sensor_properties.push_back(
        std::make_unique< model::property::meta_comment >());
}

bool PropertyTableModel::setData(
    const QModelIndex& index, const QVariant& value, int role)
{

    if (role != Qt::EditRole && role != Qt::DisplayRole) {
        return false;
    }
    if (index.column() != 1 || index.row() < 0) {
        return false;
    }

    auto measurement = this->getSelectedMeasurement();
    // On non set index only a measurment is selected
    if (!this->_index) {
        if (index.row() >= this->_measurment_properties.size()) {
            return false;
        }
        if (!this->_measurment_properties.at(index.row())->set_allowed()) {
            return false;
        }
        if (!this->_measurment_properties.at(index.row())
                 ->set_data(measurement, this->_index, value)) {
            return false;
        }
    }
    else {
        if (index.row() >= this->_sensor_properties.size()) {
            return false;
        }
        if (!this->_sensor_properties.at(index.row())->set_allowed()) {
            return false;
        }
        if (!this->_sensor_properties.at(index.row())
                 ->set_data(measurement, this->_index, value)) {
            return false;
        }
    }

    emit this->dataChanged(QModelIndex(), index, QVector< int >(role));
    for (size_t pos = 0; pos < this->_project->measurements.size(); ++pos) {
        if (this->_project->measurements[ pos ].get() == measurement.get()) {
            this->_project->updatedMeasurement(
                this->_project->measurements[ pos ], pos);
            break;
        }
    }
    return true;
}

QVariant PropertyTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    if (index.column() < 0 || index.column() > 1 || index.row() < 0) {
        return QVariant();
    }

    auto measurement = this->getSelectedMeasurement();
    // On non set index only a measurment is selected
    if (!this->_index) {
        if (index.row() >= this->_measurment_properties.size()) {
            return QVariant();
        }
        if (index.column() == 0) {
            return QString::fromStdString(
                this->_measurment_properties.at(index.row())->name());
        }
        if (index.column() == 1) {
            return this->_measurment_properties.at(index.row())
                ->get_data(measurement, this->_index);
        }
    }
    else {
        if (index.row() >= this->_sensor_properties.size()) {
            return QVariant();
        }
        if (index.column() == 0) {
            return QString::fromStdString(
                this->_sensor_properties.at(index.row())->name());
        }
        if (index.column() == 1) {
            return this->_sensor_properties.at(index.row())
                ->get_data(measurement, this->_index);
        }
    }

    std::cout << "END?!_data({col:" << index.column() << ", row:" << index.row()
              << "}, role: " << role << ")" << std::endl;

    return QVariant();
}

Qt::ItemFlags PropertyTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (index.column() != 1 || index.row() < 0) {
        return QAbstractTableModel::flags(index);
    }

    // On non set index only a measurment is selected
    if (!this->_index) {
        if (index.row() < this->_measurment_properties.size()) {
            if (this->_measurment_properties.at(index.row())->set_allowed()) {
                return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
            }
            return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
        }
    }
    else {
        if (index.row() < this->_sensor_properties.size()) {
            if (this->_sensor_properties.at(index.row())->set_allowed()) {
                return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
            }
            return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
        }
    }

    return QAbstractTableModel::flags(index);
}

QVariant PropertyTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return QVariant(QObject::tr("Name"));
        }
        if (section == 1) {
            return QVariant(QObject::tr("Value"));
        }
    }
    return QVariant();
}

int PropertyTableModel::rowCount(const QModelIndex& parent) const
{
    if (this->_selected == nullptr) {
        return 0;
    }
    if (!this->_index) {
        return this->_measurment_properties.size();
    }
    return this->_sensor_properties.size();
}

int PropertyTableModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

void PropertyTableModel::measurementTreeSelectionChanged(
    const QItemSelection& selected, const QItemSelection& deselected)
{
    emit this->beginResetModel();
    if (selected.indexes().isEmpty()) {
        this->_selected = nullptr;
    }
    else {
        this->_selected = selected.indexes().at(0).internalPointer();
        for (int i = 0; i < this->_project->measurements.size(); ++i) {
            auto measurement = this->_project->measurements.at(i);
            if (measurement.get() == this->_selected) {
                // FOUND it!
                this->_index = {};
                break;
            }
            if (measurement->reader.get() == this->_selected) {
                // FOUND it!
                this->_index = { static_cast< size_t >(
                    selected.indexes().at(0).row()) };
                break;
            }
        }
    }
    emit this->endResetModel();
}

void PropertyTableModel::addedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
}
void PropertyTableModel::updatedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    if (this->_selected == m.get() || this->_selected == m->reader.get()) {
        emit this->beginResetModel();
        emit this->endResetModel();
    }
}
void PropertyTableModel::removedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    if (this->_selected == m.get() || this->_selected == m->reader.get()) {
        this->_selected = nullptr;
        this->_index = {};
        emit this->beginResetModel();
        emit this->endResetModel();
    }
}

void PropertyTableModel::updatedProject()
{
    this->_selected = nullptr;
    this->_index = {};
    emit this->beginResetModel();
    emit this->endResetModel();
}

void PropertyTableModel::addedProbe(std::shared_ptr< Probe > p, size_t index)
{
}
void PropertyTableModel::updatedProbe(std::shared_ptr< Probe > p, size_t index)
{
}
void PropertyTableModel::removedProbe(std::shared_ptr< Probe > p, size_t index)
{
}

std::shared_ptr< Measurement > PropertyTableModel::getSelectedMeasurement()
    const
{
    // Search for selected Object
    for (auto& measurement : this->_project->measurements) {
        if (measurement.get() == this->_selected) {
            // FOUND it!
            return measurement;
        }
        if (measurement->reader.get() == this->_selected) {
            // FOUND it!
            return measurement;
        }
    }
    return std::shared_ptr< Measurement >();
}
