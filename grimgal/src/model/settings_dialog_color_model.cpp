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
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

// Own
#include "data/configuration.h"
#include "model/settings_dialog_color_model.h"

// StdLib
#include <memory>

settings_dialog_color_model::settings_dialog_color_model(
    std::shared_ptr< Configuration > configuration, QObject* parent)
    : QAbstractTableModel(parent)
{
    this->_configuration = configuration;
}

QVariant settings_dialog_color_model::data(
    const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    // TODO
    if (role == Qt::DisplayRole) {
        return this->_configuration->color[ COLOR_CFG(index.row()) ].name();
    }
    return QVariant();
}

Qt::ItemFlags settings_dialog_color_model::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
}

QVariant settings_dialog_color_model::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return QObject::tr("Color");
        }
        else {
            return QString::fromStdString(to_string(COLOR_CFG(section)));
        }
    }
    return QVariant();
}

int settings_dialog_color_model::rowCount(const QModelIndex& parent) const
{
    return COLOR_CFG::__COLOR_CFG_COUNT;
}

int settings_dialog_color_model::columnCount(const QModelIndex& parent) const
{
    return 1;
}
