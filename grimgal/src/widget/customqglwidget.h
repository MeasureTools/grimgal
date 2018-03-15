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

#ifndef CUSTOMQGLWIDGET_H
#define CUSTOMQGLWIDGET_H

// Qt
#include <QKeyEvent>
#include <QMap>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPair>
#include <QPen>
#include <QPoint>
#include <QSet>
#include <QWheelEvent>

// Own
#include "data/configuration.h"
#include "data/measurement.h"
#include "data/probe.h"
#include "data/project.h"
#include <rlib/common/event_data.h>
#include <rlib/common/sample.h>

// StdLib
#include <iostream>
#include <memory>

enum MouseMode { NO_MODE, MOVE_PROBE, MOVE_COORD };

class CustomQGLWidget : public QOpenGLWidget {
    Q_OBJECT

    private:
    std::shared_ptr< Project > _project;
    std::shared_ptr< Configuration > _configuration;

    QMap< Qt::MouseButton, QPoint > _mouse_button_last_position;
    QPoint _mouse_last_position;
    QSet< int > _active_keys;

    QPointF _center = QPointF(0.0, 0.0);
    QPointF _square = QPointF(64.0, 64.0);
    qreal _time_per_square = 0.5;
    int _zoom = 1;
    qreal _value_per_square = 1.0;
    MouseMode _mouse_mode = MouseMode::NO_MODE;
    std::shared_ptr< Probe > _selected_probe;

    std::vector< rlib::common::sample > _datum_cache;
    std::vector< rlib::common::event_data > _event_cache;

    private:
    void drawMeasurement(std::shared_ptr< Measurement > m, qreal penWidth,
        QPointF offset = QPointF(0.0, 0.0));
    void drawProbe(std::shared_ptr< Probe > p, QString lable, qreal penWidth,
        QPointF offset = QPointF(0.0, 0.0));
    void drawEvent(std::shared_ptr< Measurement > m,
        rlib::common::event_data& e, qreal penWidth,
        QPointF offset = QPointF(0.0, 0.0));
    void drawGrid(qreal penWidth, QPointF offset = QPointF(0.0, 0.0));
    void drawGridLables(qreal penWidth, QPointF offset = QPointF(0.0, 0.0));

    protected:
    virtual void initializeGL() override final;
    virtual void paintGL() override final;
    virtual void resizeGL(int w, int h) override final;
    virtual void wheelEvent(QWheelEvent* event) override final;
    virtual void keyPressEvent(QKeyEvent* event) override final;
    virtual void keyReleaseEvent(QKeyEvent* event) override final;
    virtual void mouseMoveEvent(QMouseEvent* event) override final;
    virtual void mousePressEvent(QMouseEvent* event) override final;
    virtual void mouseReleaseEvent(QMouseEvent* event) override final;

    public:
    CustomQGLWidget(
        QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~CustomQGLWidget() = default;

    void setProject(std::shared_ptr< Project > project);
    void setConfiguration(std::shared_ptr< Configuration > configuration);

    double centerAsTime();
    int_fast32_t drawResolution();

    signals:
    void resolutionChanged(int_fast32_t newResolution);

    public slots:
    void goTo(double time);

    // Project / Measurment / Probes Slots
    void addedMeasurement(std::shared_ptr< Measurement > m, size_t index);
    void updatedMeasurement(std::shared_ptr< Measurement > m, size_t index);
    void removedMeasurement(std::shared_ptr< Measurement > m, size_t index);

    void updatedProject();

    void addedProbe(std::shared_ptr< Probe > p, size_t index);
    void updatedProbe(std::shared_ptr< Probe > p, size_t index);
    void removedProbe(std::shared_ptr< Probe > p, size_t index);
};

#endif // CUSTOMQGLWIDGET_H
