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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt
#include <QMainWindow>

// Own
#include "data/configuration.h"
#include "data/project.h"
#include "eventfilter/probeview/removeprobe.h"
#include "form/settings_dialog.h"
#include "model/eventtablemodel.h"
#include "model/measurementtreemodel.h"
#include "model/probetablemodel.h"
#include "model/propertytablemodel.h"
#include "model/statistictablemodel.h"
#include <rlib/common/reader.h>

// StdLib
#include <functional>
#include <map>
#include <memory>

// Macros
#define MACRO_CONNECT_TO_PROJECT(_SIGNAL, _SLOT, _TYPE)                        \
    do {                                                                       \
        QObject::connect(_SIGNAL, &Project::addedMeasurement, _SLOT,           \
            &_TYPE::addedMeasurement);                                         \
        QObject::connect(_SIGNAL, &Project::updatedMeasurement, _SLOT,         \
            &_TYPE::updatedMeasurement);                                       \
        QObject::connect(_SIGNAL, &Project::removedMeasurement, _SLOT,         \
            &_TYPE::removedMeasurement);                                       \
        QObject::connect(                                                      \
            _SIGNAL, &Project::updatedProject, _SLOT, &_TYPE::updatedProject); \
        QObject::connect(                                                      \
            _SIGNAL, &Project::addedProbe, _SLOT, &_TYPE::addedProbe);         \
        QObject::connect(                                                      \
            _SIGNAL, &Project::updatedProbe, _SLOT, &_TYPE::updatedProbe);     \
        QObject::connect(                                                      \
            _SIGNAL, &Project::removedProbe, _SLOT, &_TYPE::removedProbe);     \
    } while (false);

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    private:
    Ui::MainWindow* _ui;

    std::shared_ptr< Project > _project;
    std::shared_ptr< Configuration > _configuration;

    // Models
    std::shared_ptr< MeasurementTreeModel > _measurement_model;
    std::shared_ptr< PropertyTableModel > _property_model;
    std::shared_ptr< ProbeTableModel > _probe_model;
    std::shared_ptr< EventTableModel > _event_model;
    std::shared_ptr< StatisticTableModel > _statistic_model;

    // Event Filter
    std::shared_ptr< eventfilter::probeview::RemoveProbe >
        _event_filter_probe_view_remove_probe;

    std::map< QString,
        std::function< std::shared_ptr< rlib::common::reader >(QString) > >
        _reader;

    // Dialogs
    std::unique_ptr< settings_dialog > _other_settings;

    // Recent File Actions
    std::vector< std::shared_ptr< QAction > > _recent_projects;
    std::vector< std::shared_ptr< QAction > > _recent_measurments;

    public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

    // Opens a project file by filename
    std::experimental::optional< std::shared_ptr< Project > > open_project(
        QString filename);

    // Adds a measurement file by filename
    std::experimental::optional< std::shared_ptr< Measurement > >
        add_measurment(QString filename);

    public slots:
    // menubar->Project
    void newProject();
    void openProject();
    void saveProject();
    void saveAsProject();
    // menbubar->Project->Recent Projects
    void open_recent_project();

    // menubar->Measurement OR Plus-Button in Measurements dock
    void addMeasurement();
    // menbubar->Measurement->Recent Measurements
    void open_recent_measurments();

    // Minus-Button in Measurements dock
    void removeMeasurement();

    // Probe->New
    void addProbe();

    // double click on measurementtreeview
    void changeMeasurementColor(QModelIndex index);

    // Help->About
    void about();

    // Settings->Use cached reader
    void setUseCachedReader(bool use);

    // Settings->Use statistic reader
    void setUseStatisticReader(bool use);

    // Settings->Other Settings
    void showOtherSettings();

    // Extras->Screenshot
    void takeScreenshot();
};

#endif // MAINWINDOW_H
