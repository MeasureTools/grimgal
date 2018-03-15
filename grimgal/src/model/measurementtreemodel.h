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

#ifndef MEASURMENTTREEMODEL_H
#define MEASURMENTTREEMODEL_H

// Qt
#include <QAbstractItemModel>

// Own
#include "data/configuration.h"
#include "data/measurement.h"
#include "data/project.h"

// StdLib
#include <memory>

class MeasurementTreeModel : public QAbstractItemModel {
    Q_OBJECT

    private:
    std::shared_ptr< Project > _project;
    std::shared_ptr< Configuration > _configuration;

    public:
    MeasurementTreeModel(std::shared_ptr< Configuration > configuration,
        std::shared_ptr< Project > project, QObject* parent = 0);
    virtual ~MeasurementTreeModel() = default;

    public: // From QAbstractItemModel
    virtual bool setData(const QModelIndex& index, const QVariant& value,
        int role = Qt::EditRole) override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column,
        const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;
    virtual int rowCount(
        const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(
        const QModelIndex& parent = QModelIndex()) const override;

    public slots:
    void selectedChanged();
    void projectChanged();

    // Project / Measurment / Probes Slots
    void addedMeasurement(std::shared_ptr< Measurement > m, size_t index);
    void updatedMeasurement(std::shared_ptr< Measurement > m, size_t index);
    void removedMeasurement(std::shared_ptr< Measurement > m, size_t index);

    void updatedProject();

    void addedProbe(std::shared_ptr< Probe > p, size_t index);
    void updatedProbe(std::shared_ptr< Probe > p, size_t index);
    void removedProbe(std::shared_ptr< Probe > p, size_t index);
};

#endif // MEASURMENTTREE_H
