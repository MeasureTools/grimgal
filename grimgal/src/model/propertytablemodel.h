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

#ifndef PROPERTYTABLEMODEL_H
#define PROPERTYTABLEMODEL_H

// Qt
#include <QAbstractItemModel>
#include <QItemSelection>
#include <QString>
#include <QVariant>

// Own
#include "data/configuration.h"
#include "data/measurement.h"
#include "data/project.h"
#include "model/property/iproperty.h"

// StdLib
#include <cstddef>
#include <cstdint>
#include <experimental/optional>
#include <functional>
#include <memory>

class PropertyTableModel : public QAbstractTableModel {
    Q_OBJECT

    private:
    std::vector< std::unique_ptr< model::property::iproperty > >
        _measurment_properties;
    std::vector< std::unique_ptr< model::property::iproperty > >
        _sensor_properties;

    std::shared_ptr< Project > _project;
    std::shared_ptr< Configuration > _configuration;
    void* _selected;
    std::experimental::optional< size_t > _index;

    private:
    std::shared_ptr< Measurement > getSelectedMeasurement() const;

    public:
    PropertyTableModel(std::shared_ptr< Configuration > configuration,
        std::shared_ptr< Project > project, QObject* parent = 0);
    virtual ~PropertyTableModel() = default;

    public: // From QAbstractTableModel
    virtual bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    virtual int rowCount(
        const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(
        const QModelIndex& parent = QModelIndex()) const override;

    public slots:
    void measurementTreeSelectionChanged(
        const QItemSelection& selected, const QItemSelection& deselected);

    // Project / Measurment / Probes Slots
    void addedMeasurement(std::shared_ptr< Measurement > m, size_t index);
    void updatedMeasurement(std::shared_ptr< Measurement > m, size_t index);
    void removedMeasurement(std::shared_ptr< Measurement > m, size_t index);

    void updatedProject();

    void addedProbe(std::shared_ptr< Probe > p, size_t index);
    void updatedProbe(std::shared_ptr< Probe > p, size_t index);
    void removedProbe(std::shared_ptr< Probe > p, size_t index);
};

#endif // PROPERTYTABLEMODEL_H
