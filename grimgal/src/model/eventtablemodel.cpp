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
#include "data/probe.h"
#include "data/project.h"
#include "model/eventtablemodel.h"
#include "util/number_format.h"
#include <rlib/common/event_data.h>

// StdLib
#include <algorithm>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
#include <tuple>
#include <vector>

EventTableModel::EventTableModel(std::shared_ptr< Configuration > configuration,
    std::shared_ptr< Project > project, QObject* parent)
    : QAbstractTableModel(parent)
{
    this->_configuration = configuration;
    this->_project = project;
    this->rebuildEventCache();
}

void EventTableModel::addMeasurement(std::shared_ptr< Measurement > m)
{
}

void EventTableModel::removeMeasurement(std::shared_ptr< Measurement > m)
{
    this->_event_cache.clear();
    this->projectChanged();
}

QVariant EventTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (static_cast< size_t >(index.row()) >= this->_event_cache.size()) {
            return QVariant();
        }
        auto m = std::get< std::shared_ptr< Measurement > >(
            this->_event_cache[ static_cast< size_t >(index.row()) ]);
        auto e = std::get< rlib::common::event_data >(
            this->_event_cache[ static_cast< size_t >(index.row()) ]);
        if (index.column() == 0) {
            return QVariant(util::format_time(e.time));
        }
        if (index.column() == 1) {
            QString source = m->name;
            QString channel = "[GLOBAL]";
            if (e.origin != -1) {
                channel = m->sensorName[ static_cast< size_t >(e.origin) ];
            }
            QString level = "[UKN]";
            switch (e.event_level) {
                case rlib::common::event_data_level::VERBOS:
                    level = "[VERBOS]";
                    break;
                case rlib::common::event_data_level::DEBUG:
                    level = "[DEBUG]";
                    break;
                case rlib::common::event_data_level::WARNING:
                    level = "[WARNING]";
                    break;
                case rlib::common::event_data_level::ERROR:
                    level = "[ERROR]";
                    break;
            }
            return QVariant(level + " " + source + " " + channel + ": " +
                            QString::fromStdString(e.message));
        }
    }

    return QVariant();
}

Qt::ItemFlags EventTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
}

QVariant EventTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            QString header;
            if (section == 0) {
                QTextStream(&header) << QObject::tr("Time");
            }
            if (section == 1) {
                QTextStream(&header) << QObject::tr("Event");
            }
            return QVariant(header);
        }
    }
    return QVariant();
}

int EventTableModel::rowCount(const QModelIndex& parent) const
{
    return static_cast< int >(this->_event_cache.size());
}

int EventTableModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

void EventTableModel::doubleClicked(const QModelIndex& index)
{
    emit this->goTo(std::get< rlib::common::event_data >(
        this->_event_cache[ static_cast< size_t >(index.row()) ])
                        .time);
}

void EventTableModel::sectionClicked(int section)
{
    emit this->goTo(std::get< rlib::common::event_data >(
        this->_event_cache[ static_cast< size_t >(section) ])
                        .time);
}

void EventTableModel::addedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    auto events = m->reader->events(0.0, -1.0);
    if (!events.empty()) {
        // Add all events to the event cache
        for (auto& e : events) {
            this->_event_cache.push_back({ m, e });
        }

        this->sortEventCache();
    }
    this->projectChanged();
}
void EventTableModel::updatedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    // Remove all events from Measurement m
    this->_event_cache.erase(
        std::remove_if(this->_event_cache.begin(), this->_event_cache.end(),
            [&m](auto t) {
                return std::get< std::shared_ptr< Measurement > >(t).get() ==
                       m.get();
            }),
        this->_event_cache.end());
    // Readd events
    auto events = m->reader->events(0.0, -1.0);
    if (!events.empty()) {
        // Add all events to the event cache
        for (auto& e : events) {
            this->_event_cache.push_back({ m, e });
        }

        this->sortEventCache();
    }
    this->projectChanged();
}
void EventTableModel::removedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    // Remove all events from Measurement m
    this->_event_cache.erase(
        std::remove_if(this->_event_cache.begin(), this->_event_cache.end(),
            [&m](auto t) {
                return std::get< std::shared_ptr< Measurement > >(t).get() ==
                       m.get();
            }),
        this->_event_cache.end());
    this->projectChanged();
}

void EventTableModel::updatedProject()
{
    this->rebuildEventCache();
    this->projectChanged();
}

void EventTableModel::addedProbe(std::shared_ptr< Probe > p, size_t index)
{
}
void EventTableModel::updatedProbe(std::shared_ptr< Probe > p, size_t index)
{
}
void EventTableModel::removedProbe(std::shared_ptr< Probe > p, size_t index)
{
}

void EventTableModel::projectChanged()
{
    emit this->dataChanged(QModelIndex(),
        this->index(this->rowCount() - 1, this->columnCount() - 1));
    emit this->layoutChanged();
}

void EventTableModel::rebuildEventCache()
{
    // Clear cache
    this->_event_cache.clear();

    // Add all events of updated project to cache
    for (auto& m : this->_project->measurements) {
        auto events = m->reader->events(0.0, -1.0);
        if (!events.empty()) {
            // Add all events to cache
            for (auto& e : events) {
                this->_event_cache.push_back({ m, e });
            }
        }
    }

    this->sortEventCache();
}

void EventTableModel::sortEventCache()
{
    // Sort events by time
    auto comp = [](auto t1, auto t2) {
        return std::get< rlib::common::event_data >(t1).time <
               std::get< rlib::common::event_data >(t2).time;
    };
    std::sort(this->_event_cache.begin(), this->_event_cache.end(), comp);
}
