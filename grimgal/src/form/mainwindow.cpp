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
#include <QColorDialog>
#include <QDomElement>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QObject>
#include <QXmlStreamWriter>

// Own
#include "data/configuration.h"
#include "eventfilter/probeview/removeprobe.h"
#include "form/mainwindow.h"
#include "model/eventtablemodel.h"
#include "model/measurementtreemodel.h"
#include "model/probetablemodel.h"
#include "model/propertytablemodel.h"
#include "model/statistictablemodel.h"
#include "ui_mainwindow.h"
#include <rlib/android/meta_reader.h>
#include <rlib/common/cached_reader.h>
#include <rlib/common/reader.h>
#include <rlib/common/statistic_reader.h>
#include <rlib/csv/csv_reader.h>
#include <rlib/grim/grim_reader.h>
#include <rlib/keysight/dlog_reader.h>
#include <rlib/powerscale/psi_reader.h>
#include <rlib/remote/reader.h>
#include <rlib/xml/xml_reader.h>

// StdLib
#include <iostream>

//--//PUBLIC
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
{
    // Initial Ui setup
    this->_ui->setupUi(this);

    // TabifyDockWidget
    this->tabifyDockWidget(this->_ui->probeDock, this->_ui->eventDock);
    this->tabifyDockWidget(this->_ui->eventDock, this->_ui->statisticDock);

    // Set Members
    this->_project = std::make_shared< Project >();
    this->_configuration = std::make_shared< Configuration >();
    this->_other_settings =
        std::make_unique< settings_dialog >(this->_configuration, this);

    // Set Members :: Set Models
    this->_measurement_model = std::make_shared< MeasurementTreeModel >(
        this->_configuration, this->_project, this);
    this->_property_model = std::make_shared< PropertyTableModel >(
        this->_configuration, this->_project, this);
    this->_probe_model = std::make_shared< ProbeTableModel >(
        this->_configuration, this->_project, this);
    {
        this->_probe_model->setResolution(
            this->_ui->glWidget->drawResolution());
    }
    this->_event_model = std::make_shared< EventTableModel >(
        this->_configuration, this->_project, this);
    this->_statistic_model = std::make_shared< StatisticTableModel >(
        this->_configuration, this->_project, this);

    // Set Members :: Event Filter
    this->_event_filter_probe_view_remove_probe =
        std::make_shared< eventfilter::probeview::RemoveProbe >(
            this->_configuration, this->_project);

    // Config Ui
    this->_ui->glWidget->setProject(this->_project);
    this->_ui->glWidget->setConfiguration(this->_configuration);
    this->_ui->probeTable->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);

    // Config Ui :: Set Models
    this->_ui->measurementTree->setModel(this->_measurement_model.get());
    this->_ui->propertyTable->setModel(this->_property_model.get());
    this->_ui->eventTable->setModel(this->_event_model.get());
    this->_ui->statisticTable->setModel(this->_statistic_model.get());
    this->_ui->probeTable->setModel(this->_probe_model.get());

    // Config Ui :: Add Event Filter
    this->_ui->probeTable->installEventFilter(
        this->_event_filter_probe_view_remove_probe.get());

    // Connect Slots and Signals
    QItemSelectionModel* selectionModel =
        this->_ui->measurementTree->selectionModel();
    QObject::connect(selectionModel, &QItemSelectionModel::selectionChanged,
        this->_property_model.get(),
        &PropertyTableModel::measurementTreeSelectionChanged);
    QObject::connect(this->_ui->glWidget,
        SIGNAL(resolutionChanged(int_fast32_t)), this->_probe_model.get(),
        SLOT(setResolution(int_fast32_t)));
    QObject::connect(this->_ui->probeTable->horizontalHeader(),
        SIGNAL(sectionClicked(int)), this->_probe_model.get(),
        SLOT(sectionClicked(int)));
    QObject::connect(this->_ui->eventTable,
        SIGNAL(doubleClicked(const QModelIndex)), this->_event_model.get(),
        SLOT(doubleClicked(QModelIndex)));
    QObject::connect(this->_probe_model.get(), SIGNAL(goTo(double)),
        this->_ui->glWidget, SLOT(goTo(double)));
    QObject::connect(this->_event_model.get(), SIGNAL(goTo(double)),
        this->_ui->glWidget, SLOT(goTo(double)));

    // Nofity Project / Measurement / Probe add/update/remove
    MACRO_CONNECT_TO_PROJECT(
        this->_project.get(), this->_ui->glWidget, CustomQGLWidget);
    MACRO_CONNECT_TO_PROJECT(
        this->_project.get(), this->_event_model.get(), EventTableModel);
    MACRO_CONNECT_TO_PROJECT(this->_project.get(),
        this->_measurement_model.get(), MeasurementTreeModel);
    MACRO_CONNECT_TO_PROJECT(
        this->_project.get(), this->_property_model.get(), PropertyTableModel);
    MACRO_CONNECT_TO_PROJECT(
        this->_project.get(), this->_probe_model.get(), ProbeTableModel);
    MACRO_CONNECT_TO_PROJECT(this->_project.get(), this->_statistic_model.get(),
        StatisticTableModel);

    // Add Reader
    this->_reader.insert_or_assign("Keysight;.dlog", [](QString file) {
        return std::make_shared< rlib::keysight::dlog_reader >(
            file.toStdString());
    });
    this->_reader.insert_or_assign("Powerscale;.psi", [](QString file) {
        return std::make_shared< rlib::powerscale::psi_reader >(
            file.toStdString());
    });
    this->_reader.insert_or_assign("Android;.meta", [](QString file) {
        return std::make_shared< rlib::android::meta_reader >(
            file.toStdString());
    });
    this->_reader.insert_or_assign("Grimgal;.grim", [](QString file) {
        return std::make_shared< rlib::grim::grim_reader >(file.toStdString());
    });
    this->_reader.insert_or_assign("Grimgal CSV;.csv", [](QString file) {
        return std::make_shared< rlib::csv::csv_reader >(file.toStdString());
    });
    this->_reader.insert_or_assign("Grimgal XML;.xml", [](QString file) {
        return std::make_shared< rlib::xml::xml_reader >(file.toStdString());
    });
    this->_reader.insert_or_assign("Remote;.remote", [](QString file) {
        return std::make_shared< rlib::remote::reader >(
            file.toStdString(), 1, 8888, 1);
    });
}

MainWindow::~MainWindow()
{
    delete _ui;
}

std::experimental::optional< std::shared_ptr< Project > > MainWindow::
    open_project(QString filename)
{
    QFileInfo check_file(filename);
    if (!check_file.exists() || !check_file.isFile()) {
        std::cout << QObject::tr("Could not load file ").toStdString()
                  << filename.toStdString()
                  << QObject::tr(" Reason: File Not Found").toStdString()
                  << std::endl;
        return {};
    }

    std::shared_ptr< QAction > action;

    auto found_action = std::find_if(this->_recent_projects.begin(),
        this->_recent_projects.end(),
        [filename](auto a) { return a->data().toString() == filename; });
    if (found_action != this->_recent_projects.end()) {
        action = *found_action;
        this->_ui->menuRecent_Projects->removeAction(action.get());
    }
    else {
        action = this->_recent_projects.emplace_back(
            std::make_shared< QAction >(filename, this));
        {
            action->setVisible(true);
            action->setData(filename);
        }
        QObject::connect(action.get(), SIGNAL(triggered()), this,
            SLOT(open_recent_project()));
    }
    this->_ui->menuRecent_Projects->addAction(action.get());

    // Rename each action to add a correct number before the filename
    int i = 0;
    for (auto recent_action : this->_ui->menuRecent_Projects->actions()) {
        recent_action->setText(
            "&" + QString::number(i) + " " + recent_action->data().toString());
        ++i;
    }

    this->newProject();
    this->_project->file = filename;

    QDomDocument projectXml;
    {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        projectXml.setContent(&file);
        file.close();
    }
    QDomElement projectElement = projectXml.documentElement();
    QDomElement measurementsElement =
        projectElement.firstChildElement("measurements");
    QDomNodeList measurementsChildNodes = measurementsElement.childNodes();
    for (int i = 0; i < measurementsChildNodes.length(); i++) {

        QDomElement measurementElement =
            measurementsChildNodes.at(i).toElement();

        auto measurementFile =
            measurementElement.firstChildElement("file").text();

        auto opt_measurment = this->add_measurment(measurementFile);
        if (!opt_measurment) {
            continue;
        }
        auto measurement = *(opt_measurment);

        measurement->name = measurementElement.firstChildElement("name").text();

        for (int k = 0; k < measurementElement.firstChildElement("sensorname")
                                .childNodes()
                                .size();
             k++) {
            auto value = measurementElement.firstChildElement("sensorname")
                             .childNodes()
                             .at(k);
            measurement->sensorName[ k ] = (value.toElement().text());
        }
        for (int k = 0; k < measurementElement.firstChildElement("unitfactor")
                                .childNodes()
                                .size();
             k++) {
            auto value = measurementElement.firstChildElement("unitfactor")
                             .childNodes()
                             .at(k);
            measurement->unitFactor[ k ] =
                (value.toElement().text().toLongLong());
        }
        for (int k = 0; k < measurementElement.firstChildElement("offsetx")
                                .childNodes()
                                .size();
             k++) {
            auto value =
                measurementElement.firstChildElement("offsetx").childNodes().at(
                    k);
            measurement->offsetX[ k ] = (value.toElement().text().toDouble());
        }
        for (int k = 0; k < measurementElement.firstChildElement("offsety")
                                .childNodes()
                                .size();
             k++) {
            auto value =
                measurementElement.firstChildElement("offsety").childNodes().at(
                    k);
            measurement->offsetY[ k ] = (value.toElement().text().toDouble());
        }
        for (int k = 0; k < measurementElement.firstChildElement("visible")
                                .childNodes()
                                .size();
             k++) {
            auto value =
                measurementElement.firstChildElement("visible").childNodes().at(
                    k);
            if (value.toElement().text().toInt() == 1) {
                measurement->visible[ k ] = (true);
            }
            else {
                measurement->visible[ k ] = (false);
            }
        }
        for (int k = 0;
             k <
             measurementElement.firstChildElement("color").childNodes().size();
             k++) {
            auto value =
                measurementElement.firstChildElement("color").childNodes().at(
                    k);
            measurement->color[ k ] = (QColor(value.toElement().text()));
        }
        for (int k = 0; k < measurementElement.firstChildElement("comment")
                                .childNodes()
                                .size();
             k++) {
            auto value =
                measurementElement.firstChildElement("comment").childNodes().at(
                    k);
            measurement->comment[ k ] = (value.toElement().text());
        }
    }

    QDomElement probeElement = projectElement.firstChildElement("probes");
    for (int k = 0; k < probeElement.childNodes().size(); k++) {
        auto probeNode = probeElement.childNodes().at(k);
        auto probe = std::make_shared< Probe >();
        {
            probe->time = probeNode.toElement().text().toDouble();
        }
        this->_project->probes.push_back(probe);
    }

    this->_project->updatedProject();
    return this->_project;
}

std::experimental::optional< std::shared_ptr< Measurement > > MainWindow::
    add_measurment(QString filename)
{
    QFileInfo check_file(filename);
    if (!check_file.exists() || !check_file.isFile()) {
        std::cout << QObject::tr("Could not load file ").toStdString()
                  << filename.toStdString()
                  << QObject::tr(" Reason: File Not Found").toStdString()
                  << std::endl;
        return {};
    }

    std::shared_ptr< QAction > action;

    auto found_action = std::find_if(this->_recent_measurments.begin(),
        this->_recent_measurments.end(),
        [filename](auto a) { return a->data().toString() == filename; });
    if (found_action != this->_recent_measurments.end()) {
        action = *found_action;
        this->_ui->menuRecent_Measurments->removeAction(action.get());
    }
    else {
        action = this->_recent_measurments.emplace_back(
            std::make_shared< QAction >(filename, this));
        {
            action->setVisible(true);
            action->setData(filename);
        }
        QObject::connect(action.get(), SIGNAL(triggered()), this,
            SLOT(open_recent_measurments()));
    }
    this->_ui->menuRecent_Measurments->addAction(action.get());

    // Rename each action to add a correct number before the filename
    int i = 0;
    for (auto recent_action : this->_ui->menuRecent_Measurments->actions()) {
        recent_action->setText(
            "&" + QString::number(i) + " " + recent_action->data().toString());
        ++i;
    }

    auto measurement = std::make_shared< Measurement >();

    // Look for a reader object which already reads the same file
    bool foundReader = false;
    for (auto& m : this->_project->measurements) {
        if (QString::fromStdString(m->reader->filename()) == filename) {
            measurement->setReader(m->reader);
            foundReader = true;
            break;
        }
    }
    if (!foundReader) {
        // Look for a reader
        for (auto& tuple : this->_reader) {
            auto ext = std::get< 0 >(tuple).section(';', 1, 1);
            if (filename.endsWith(ext)) {
                auto reader = std::get< 1 >(tuple)(filename);
                std::shared_ptr< rlib::common::reader > measurement_reader =
                    reader;
                if (this->_configuration->_use_statistic_reader) {
                    measurement_reader =
                        std::make_shared< rlib::common::statistic_reader >(
                            measurement_reader);
                }
                if (this->_configuration->_use_cached_reader) {
                    measurement_reader =
                        std::make_shared< rlib::common::cached_reader >(
                            measurement_reader);
                }
                measurement->setReader(measurement_reader);
                foundReader = true;
                break;
            }
        }
    }
    // Skip file without reader
    if (!foundReader) {
        std::cout << QObject::tr("Could not load file ").toStdString()
                  << filename.toStdString()
                  << QObject::tr(" Reason: No reader found").toStdString()
                  << std::endl;
        return {};
    }
    // Add Measurement to current Project
    this->_project->measurements.push_back(measurement);
    this->_project->addedMeasurement(
        measurement, this->_project->measurements.size() - 1);
    return measurement;
}

//--//PUBLIC SLOTS
void MainWindow::newProject()
{
    this->_project->clear();
    this->_project->updatedProject();
}

void MainWindow::openProject()
{
    QFileInfo info(this->_project->file);
    QString filename = QFileDialog::getOpenFileName(
        this, "Open Project", info.canonicalPath(), "Project (*.project)");
    if (filename.isEmpty()) {
        return;
    }
    this->open_project(filename);
}

void MainWindow::saveProject()
{
    if (this->_project->file.isEmpty()) {
        this->saveAsProject();
    }

    if (this->_project->file.isEmpty()) {
        return;
    }
    QFile f(this->_project->file);
    f.open(QIODevice::WriteOnly);

    QXmlStreamWriter writer(&f);
    writer.setAutoFormatting(true);

    writer.writeStartDocument();
    {
        // Project
        writer.writeStartElement("project");
        {
            // Measurements
            writer.writeStartElement("measurements");

            for (auto& measurement : this->_project->measurements) {
                writer.writeStartElement("measurement");
                {
                    writer.writeTextElement(
                        "file", QString::fromStdString(
                                    measurement->reader->filename()));
                    writer.writeTextElement("name", measurement->name);

                    writer.writeStartElement("sensorname");
                    {
                        for (auto& name : measurement->sensorName) {
                            writer.writeTextElement("name", name);
                        }
                    }
                    writer.writeEndElement();

                    writer.writeStartElement("unitfactor");
                    {
                        for (auto& factor : measurement->unitFactor) {
                            writer.writeTextElement(
                                "value", QString::number(factor));
                        }
                    }
                    writer.writeEndElement();

                    writer.writeStartElement("offsetx");
                    {
                        for (auto& offsetX : measurement->offsetX) {
                            writer.writeTextElement(
                                "value", QString::number(offsetX));
                        }
                    }
                    writer.writeEndElement();

                    writer.writeStartElement("offsety");
                    {
                        for (auto& offsetY : measurement->offsetY) {
                            writer.writeTextElement(
                                "value", QString::number(offsetY));
                        }
                    }
                    writer.writeEndElement();

                    writer.writeStartElement("visible");
                    {
                        for (const auto visible : measurement->visible) {
                            if (visible) {
                                writer.writeTextElement("value", "1");
                            }
                            else {
                                writer.writeTextElement("value", "0");
                            }
                        }
                    }
                    writer.writeEndElement();

                    writer.writeStartElement("color");
                    {
                        for (auto& color : measurement->color) {
                            writer.writeTextElement("value", color.name());
                        }
                    }
                    writer.writeEndElement();

                    writer.writeStartElement("comment");
                    {
                        for (auto& comment : measurement->comment) {
                            writer.writeTextElement("value", comment);
                        }
                    }
                    writer.writeEndElement();
                }
                writer.writeEndElement();
            }
            writer.writeEndElement();

            // Probes
            writer.writeStartElement("probes");
            for (auto& probe : this->_project->probes) {
                writer.writeTextElement("probe", QString::number(probe->time));
            }
            writer.writeEndElement();
        }
        writer.writeEndElement();
    }
    writer.writeEndElement();

    f.close();
}

void MainWindow::saveAsProject()
{
    QString filename = QFileDialog::getSaveFileName(
        this, QObject::tr("Save Project As ..."), "", "Project (*.project)");
    if (!filename.isEmpty()) {
        this->_project->file = filename;
        if (!this->_project->file.endsWith(".project")) {
            this->_project->file += ".project";
        }
        this->saveProject();
    }
}

void MainWindow::open_recent_project()
{
    QAction* action = qobject_cast< QAction* >(sender());
    if (action) {
        this->open_project(action->data().toString());
    }
}

void MainWindow::addMeasurement()
{
    QFileInfo info(this->_project->file);
    QString exts = "";
    for (auto& tuple : this->_reader) {
        auto name = std::get< 0 >(tuple).section(';', 0, 0);
        auto ext = std::get< 0 >(tuple).section(';', 1, 1);
        exts += ";;" + name + "(*" + ext + ")";
    }
    auto filenames =
        QFileDialog::getOpenFileNames(this, QObject::tr("Add Measurement"),
            info.canonicalPath(), QObject::tr("All Files (*.*)") + exts);
    for (auto& filename : filenames) {
        this->add_measurment(filename);
    }
}

void MainWindow::removeMeasurement()
{
    auto selectedMeasurmentList =
        this->_ui->measurementTree->selectionModel()->selectedIndexes();
    if (selectedMeasurmentList.size() != 1) {
        return;
    }

    // Get selected item
    QModelIndex selectedMeasurment = selectedMeasurmentList.at(0);

    // Identify index of selected items
    std::experimental::optional< size_t > foundIndex;
    for (size_t index = 0; index < this->_project->measurements.size();
         ++index) {
        if (selectedMeasurment.internalPointer() ==
                this->_project->measurements[ index ].get() ||
            selectedMeasurment.internalPointer() ==
                this->_project->measurements[ index ]->reader.get()) {
            foundIndex = index;
            break;
        }
    }

    if (!foundIndex) {
        return;
    }

    auto measurmentToBeRemoved =
        this->_project->measurements[ foundIndex.value() ];

    // Remove Measurement from current Project
    this->_project->measurements.erase(
        this->_project->measurements.begin() + foundIndex.value());
    this->_project->removedMeasurement(
        measurmentToBeRemoved, foundIndex.value());
}

void MainWindow::open_recent_measurments()
{
    QAction* action = qobject_cast< QAction* >(sender());
    if (action) {
        this->add_measurment(action->data().toString());
    }
}

void MainWindow::addProbe()
{
    auto probe = std::make_shared< Probe >();
    {
        probe->time = this->_ui->glWidget->centerAsTime();
    }
    this->_project->probes.push_back(probe);
    this->_project->addedProbe(probe, this->_project->probes.size() - 1);
}

void MainWindow::changeMeasurementColor(QModelIndex index)
{
    size_t i = 0;
    for (auto& measurement : this->_project->measurements) {
        if (measurement->reader.get() == index.internalPointer()) {
            QColor color = QColorDialog::getColor(
                measurement->color[ static_cast< size_t >(index.row()) ], this);
            if (color.isValid()) {
                measurement->color[ static_cast< size_t >(index.row()) ] =
                    color;
                this->_project->updatedMeasurement(measurement, i);
            }
        }
        ++i;
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, "Grimgal",
        "Grimgal\n\n(c) 2016-2017, Daniel Korner\nWebsite: "
        "http://grimgal.com\nLicence: "
        "BSD 1-Clause with no military use clause");
}

void MainWindow::setUseCachedReader(bool use)
{
    if (use != this->_configuration->_use_cached_reader) {
        if (use) {
            // Wrap each non cached_reader reader in a cached_reader
            for (auto& measurement : this->_project->measurements) {
                auto dyn_cast = dynamic_cast< rlib::common::cached_reader* >(
                    measurement->reader.get());
                if (dyn_cast == nullptr) {
                    measurement->reader =
                        std::make_shared< rlib::common::cached_reader >(
                            measurement->reader);
                }
            }
        }
        else {
            // Unwrap each reader from the cached_reader
            for (auto& measurement : this->_project->measurements) {
                auto dyn_cast = dynamic_cast< rlib::common::cached_reader* >(
                    measurement->reader.get());
                if (dyn_cast != nullptr) {
                    measurement->reader = dyn_cast->_reader;
                }
            }
        }
        this->_project->updatedProject();
        this->_configuration->_use_cached_reader = use;
    }
}

void MainWindow::setUseStatisticReader(bool use)
{
    if (use != this->_configuration->_use_statistic_reader) {
        if (use) {
            for (auto& measurement : this->_project->measurements) {
                auto dyn_cast_statistic_reader =
                    dynamic_cast< rlib::common::statistic_reader* >(
                        measurement->reader.get());
                if (dyn_cast_statistic_reader != nullptr) {
                    continue;
                }
                auto dyn_cast_cached_reader =
                    dynamic_cast< rlib::common::cached_reader* >(
                        measurement->reader.get());
                if (dyn_cast_cached_reader != nullptr) {
                    auto dyn_cast_inner_reader =
                        dynamic_cast< rlib::common::statistic_reader* >(
                            dyn_cast_cached_reader->_reader.get());
                    if (dyn_cast_inner_reader != nullptr) {
                        continue;
                    }
                    dyn_cast_cached_reader->_reader =
                        std::make_shared< rlib::common::statistic_reader >(
                            dyn_cast_cached_reader->_reader);
                    dyn_cast_cached_reader->reset();
                }
                else {
                    measurement->reader =
                        std::make_shared< rlib::common::statistic_reader >(
                            measurement->reader);
                }
            }
        }
        else {
            // Unwrap each reader
            for (auto& measurement : this->_project->measurements) {
                auto dyn_cast_statistic_reader =
                    dynamic_cast< rlib::common::statistic_reader* >(
                        measurement->reader.get());
                if (dyn_cast_statistic_reader != nullptr) {
                    measurement->reader = dyn_cast_statistic_reader->_reader;
                    continue;
                }
                auto dyn_cast_cached_reader =
                    dynamic_cast< rlib::common::cached_reader* >(
                        measurement->reader.get());
                if (dyn_cast_cached_reader != nullptr) {
                    auto dyn_cast_inner_reader =
                        dynamic_cast< rlib::common::statistic_reader* >(
                            dyn_cast_cached_reader->_reader.get());
                    if (dyn_cast_inner_reader != nullptr) {
                        dyn_cast_cached_reader->_reader =
                            dyn_cast_inner_reader->_reader;
                        dyn_cast_cached_reader->reset();
                        continue;
                    }
                }
            }
        }
        this->_project->updatedProject();
        this->_configuration->_use_statistic_reader = use;
    }
}

void MainWindow::showOtherSettings()
{
    this->_other_settings->show();
}

void MainWindow::takeScreenshot()
{
    this->_ui->glWidget->repaint();
    auto frame_buffer = this->_ui->glWidget->grabFramebuffer();
    QString filename = QFileDialog::getSaveFileName(
        this, QObject::tr("Save Screenshot"), "", "Image (*.png)");
    frame_buffer.save(filename, "PNG");
}
