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

#ifndef SETTINGS_DIALOG_COLOR_MODEL_H
#define SETTINGS_DIALOG_COLOR_MODEL_H

// Qt
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

// Own
#include "data/configuration.h"

// StdLib
#include <memory>

class settings_dialog_color_model : public QAbstractTableModel {
    Q_OBJECT

    private:
    std::shared_ptr< Configuration > _configuration;

    public:
    settings_dialog_color_model(
        std::shared_ptr< Configuration > configuration, QObject* parent = 0);
    virtual ~settings_dialog_color_model() = default;

    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const override;
    virtual int rowCount(
        const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(
        const QModelIndex& parent = QModelIndex()) const override;
};

#endif // SETTINGS_DIALOG_COLOR_MODEL_H
