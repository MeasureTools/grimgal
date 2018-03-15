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
#include <QColor>
#include <QLineF>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QVector>

// Own
#include "data/configuration.h"
#include "util/number_format.h"
#include "widget/customqglwidget.h"

// StdLib
#include <chrono>
#include <cmath>
#include <future>
#include <tuple>

// Macros
#define MACRO_PEN_WIDTH() qreal(1.0) / qreal(this->_zoom)

#define MACRO_TIME_TO_X(t) (((t) / this->_time_per_square) * this->_square.x())
#define MACRO_X_TO_TIME(k)                                                     \
    (double(k) / double(this->_square.x()) * this->_time_per_square)
#define MACRO_LEFTWINDOW()                                                     \
    ((-this->size().width() / (2.0 * this->_zoom)) + this->_center.x())
#define MACRO_RIGHTWINDOW()                                                    \
    (this->size().width() / (2.0 * this->_zoom) + this->_center.x())
#define MACRO_UPPERWINDOW()                                                    \
    (this->size().height() / (2.0 * this->_zoom) + this->_center.y())
#define MACRO_LOWERWINDOW()                                                    \
    ((-this->size().height() / (2.0 * this->_zoom)) + this->_center.y())

#define MACRO_LEFTBOUND() (MACRO_LEFTWINDOW() - this->_square.x())
#define MACRO_RIGHTBOUND() (MACRO_RIGHTWINDOW() + this->_square.x())
#define MACRO_UPPERBOUND() (MACRO_UPPERWINDOW() + this->_square.y())
#define MACRO_LOWERBOUND() (MACRO_LOWERWINDOW() - this->_square.y())

#define MACRO_LEFTBOUNDTIME()                                                  \
    (MACRO_LEFTBOUND() / this->_square.x()) * this->_time_per_square
#define MACRO_RIGHTBOUNDTIME()                                                 \
    (MACRO_RIGHTBOUND() / this->_square.x()) * this->_time_per_square

#define MACRO_CONFIG_QPAINTER(p)                                               \
    {                                                                          \
        QMatrix matrix;                                                        \
        {                                                                      \
            matrix.scale(1, -1);                                               \
        }                                                                      \
        p.setMatrix(matrix);                                                   \
        p.setWindow((-this->size().width() / (2 * this->_zoom)) +              \
                        static_cast< int >(this->_center.x()),                 \
            (-this->size().height() / (2 * this->_zoom)) -                     \
                static_cast< int >(this->_center.y()),                         \
            this->size().width() / this->_zoom,                                \
            this->size().height() / this->_zoom);                              \
    }

CustomQGLWidget::CustomQGLWidget(QWidget* parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setMouseTracking(true);
}

void CustomQGLWidget::setProject(std::shared_ptr< Project > project)
{
    this->_project = project;
}

void CustomQGLWidget::setConfiguration(
    std::shared_ptr< Configuration > configuration)
{
    this->_configuration = configuration;
}

double CustomQGLWidget::centerAsTime()
{
    qreal leftBoundTime = MACRO_LEFTBOUNDTIME();
    qreal rightBoundTime = MACRO_RIGHTBOUNDTIME();
    return double(leftBoundTime / 2) + double(rightBoundTime / 2);
}

int_fast32_t CustomQGLWidget::drawResolution()
{
    return static_cast< int_fast32_t >(
               std::ceil(this->_square.x() / this->_time_per_square)) *
           2;
}

void CustomQGLWidget::goTo(double time)
{
    this->_center.rx() = MACRO_TIME_TO_X(time);
    this->update();
}

void CustomQGLWidget::addedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->drawMeasurement(m, MACRO_PEN_WIDTH());
}
void CustomQGLWidget::updatedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->update();
}
void CustomQGLWidget::removedMeasurement(
    std::shared_ptr< Measurement > m, size_t index)
{
    this->update();
}

void CustomQGLWidget::updatedProject()
{
    this->update();
}

void CustomQGLWidget::addedProbe(std::shared_ptr< Probe > p, size_t index)
{
    this->drawProbe(p, "Probe " + QString::number(index), MACRO_PEN_WIDTH());
}
void CustomQGLWidget::updatedProbe(std::shared_ptr< Probe > p, size_t index)
{
    this->update();
}
void CustomQGLWidget::removedProbe(std::shared_ptr< Probe > p, size_t index)
{
    this->update();
}

void CustomQGLWidget::initializeGL()
{
}

void CustomQGLWidget::paintGL()
{
    qreal penWidth = MACRO_PEN_WIDTH();

    // Draw Background
    QPainter painter(this);
    painter.fillRect(0, 0, this->size().width(), this->size().height(),
        this->_configuration->color[ COLOR_CFG::BACKGROUND ]);

    // Draw Grid
    this->drawGrid(penWidth);

    // Draw Probes
    int i = 0;
    for (auto& probe : this->_project->probes) {
        this->drawProbe(
            probe, QObject::tr("Probe ") + QString::number(i++), penWidth);
    }

    // Draw Grid Labels (X/Y)
    this->drawGridLables(penWidth);

    // Draw Values
    for (auto& m : this->_project->measurements) {
        this->drawMeasurement(m, penWidth);
    }
}

void CustomQGLWidget::resizeGL(int w, int h)
{
}

void CustomQGLWidget::wheelEvent(QWheelEvent* event)
{
    if (this->_active_keys.find(Qt::Key_Shift) != this->_active_keys.end() &&
        this->_active_keys.find(Qt::Key_Control) == this->_active_keys.end()) {
        if (event->angleDelta().y() > 0) {
            if (this->_zoom < 4) {
                auto x = this->_center.x();
                double time = MACRO_X_TO_TIME(x);
                this->_zoom *= 2;
                this->_center.rx() = MACRO_TIME_TO_X(time);
                emit this->resolutionChanged(this->drawResolution());
            }
        }
        if (event->angleDelta().y() < 0) {
            if (this->_zoom > 1) {
                auto x = this->_center.x();
                double time = MACRO_X_TO_TIME(x);
                this->_zoom /= 2;
                this->_center.rx() = MACRO_TIME_TO_X(time);
                emit this->resolutionChanged(this->drawResolution());
            }
        }
    }
    else if (this->_active_keys.find(Qt::Key_Control) !=
                 this->_active_keys.end() &&
             this->_active_keys.find(Qt::Key_Tab) != this->_active_keys.end()) {
        if (event->angleDelta().y() > 0) {
            if (this->_time_per_square * qreal(2) > this->_time_per_square) {
                auto x = this->_center.x();
                double time = MACRO_X_TO_TIME(x);
                this->_time_per_square *= qreal(2);
                this->_center.rx() = MACRO_TIME_TO_X(time);
                emit this->resolutionChanged(this->drawResolution());
            }
        }
        if (event->angleDelta().y() < 0) {
            if (this->_time_per_square / qreal(2) > 0) {
                auto x = this->_center.x();
                double time = MACRO_X_TO_TIME(x);
                this->_time_per_square /= qreal(2);
                this->_center.rx() = MACRO_TIME_TO_X(time);
                emit this->resolutionChanged(this->drawResolution());
            }
        }
    }
    else if (this->_active_keys.find(Qt::Key_Shift) ==
                 this->_active_keys.end() &&
             this->_active_keys.find(Qt::Key_Control) !=
                 this->_active_keys.end()) {
        if (event->angleDelta().y() > 0) {
            auto x = this->_center.x();
            double time = MACRO_X_TO_TIME(x);
            this->_square.rx() *= 2;
            this->_square.ry() *= 2;
            this->_center.rx() = MACRO_TIME_TO_X(time);
            emit this->resolutionChanged(this->drawResolution());
        }
        if (event->angleDelta().y() < 0) {
            if (this->_square.x() > 8 && this->_square.y() > 8) {
                auto x = this->_center.x();
                double time = MACRO_X_TO_TIME(x);
                this->_square.rx() /= 2;
                this->_square.ry() /= 2;
                this->_center.rx() = MACRO_TIME_TO_X(time);
                emit this->resolutionChanged(this->drawResolution());
            }
        }
    }
    else if (this->_active_keys.find(Qt::Key_Shift) !=
                 this->_active_keys.end() &&
             this->_active_keys.find(Qt::Key_Control) !=
                 this->_active_keys.end()) {
        if (event->angleDelta().y() > 0) {
            if (this->_value_per_square * qreal(2) > this->_value_per_square) {
                auto x = this->_center.x();
                double time = MACRO_X_TO_TIME(x);
                this->_value_per_square *= qreal(2);
                this->_center.rx() = MACRO_TIME_TO_X(time);
                emit this->resolutionChanged(this->drawResolution());
            }
        }
        if (event->angleDelta().y() < 0) {
            if (this->_value_per_square / qreal(2) > 0) {
                auto x = this->_center.x();
                double time = MACRO_X_TO_TIME(x);
                this->_value_per_square /= qreal(2);
                this->_center.rx() = MACRO_TIME_TO_X(time);
                emit this->resolutionChanged(this->drawResolution());
            }
        }
    }
    else {
        if (event->angleDelta().y() < 0) {
            this->_center.rx() -= this->_square.x() / 2;
        }
        if (event->angleDelta().y() > 0) {
            this->_center.rx() += this->_square.x() / 2;
        }
    }
    this->update();
}

void CustomQGLWidget::keyPressEvent(QKeyEvent* event)
{
    // Add Probe
    if (this->_active_keys.find(Qt::Key_Return) == this->_active_keys.end() &&
        event->key() == Qt::Key_Return) {
        // Add new probe at current center
        auto probe = std::make_shared< Probe >();
        {
            probe->time = this->centerAsTime();
        }
        this->_project->probes.push_back(probe);
        this->_project->addedProbe(probe, this->_project->probes.size() - 1);
    }
    this->_active_keys.insert(event->key());

    bool changed = false;
    if (this->_active_keys.find(Qt::Key_Left) != this->_active_keys.end()) {
        this->_center.rx() -= this->_square.x() / 2;
        changed = true;
    }
    if (this->_active_keys.find(Qt::Key_Right) != this->_active_keys.end()) {
        this->_center.rx() += this->_square.x() / 2;
        changed = true;
    }
    if (this->_active_keys.find(Qt::Key_Up) != this->_active_keys.end()) {
        this->_center.ry() += this->_square.y() / 2;
        changed = true;
    }
    if (this->_active_keys.find(Qt::Key_Down) != this->_active_keys.end()) {
        this->_center.ry() -= this->_square.y() / 2;
        changed = true;
    }
    if (changed) {
        this->update();
    }
}

void CustomQGLWidget::keyReleaseEvent(QKeyEvent* event)
{
    this->_active_keys.remove(event->key());
}

void CustomQGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (this->_mouse_mode == MouseMode::MOVE_PROBE) {
        auto mouseXPos = MACRO_LEFTWINDOW() + event->pos().x();
        this->_selected_probe->time = fmax(0.0, MACRO_X_TO_TIME(mouseXPos));
        for (size_t index = 0; index < this->_project->probes.size(); ++index) {
            if (this->_project->probes[ index ].get() ==
                this->_selected_probe.get()) {
                this->_project->updatedProbe(this->_selected_probe, index);
                break;
            }
        }
    }

    if (this->_mouse_mode == MouseMode::MOVE_COORD) {
        this->_center.rx() -=
            event->pos().x() -
            this->_mouse_button_last_position[ Qt::LeftButton ].x();
        this->_center.ry() +=
            event->pos().y() -
            this->_mouse_button_last_position[ Qt::LeftButton ].y();
        this->_mouse_button_last_position.insert(Qt::LeftButton, event->pos());
    }

    if (this->_mouse_mode == MouseMode::NO_MODE) {
        bool aboveProbe = false;
        auto mouseXPos = MACRO_LEFTWINDOW() + event->pos().x();
        for (auto& probe : this->_project->probes) {
            auto probeTime = probe->time;
            if (fabs(mouseXPos - MACRO_TIME_TO_X(probeTime)) < 3) {
                aboveProbe = true;
                break;
            }
        }

        if (aboveProbe) {
            if (this->cursor().shape() != Qt::SizeHorCursor) {
                QCursor curser;
                curser.setShape(Qt::SizeHorCursor);
                this->setCursor(curser);
            }
        }
        else {
            if (this->cursor().shape() != Qt::ArrowCursor) {
                QCursor curser;
                curser.setShape(Qt::ArrowCursor);
                this->setCursor(curser);
            }
        }
    }

    this->_mouse_last_position.rx() = event->pos().x();
    this->_mouse_last_position.ry() = event->pos().y();
    this->update();
}

void CustomQGLWidget::mousePressEvent(QMouseEvent* event)
{
    this->_mouse_button_last_position.insert(event->button(), event->pos());
    if (event->button() == Qt::LeftButton) {
        auto mouseXPos = MACRO_LEFTWINDOW() + event->pos().x();
        bool aboveProbe = false;
        for (auto& probe : this->_project->probes) {
            auto probeTime = probe->time;
            if (std::fabs(mouseXPos - MACRO_TIME_TO_X(probeTime)) < 3) {
                this->_selected_probe = probe;
                aboveProbe = true;
                break;
            }
        }
        QCursor curser;
        if (aboveProbe) {
            this->_mouse_mode = MouseMode::MOVE_PROBE;
            curser.setShape(Qt::SizeHorCursor);
        }
        else {
            this->_mouse_mode = MouseMode::MOVE_COORD;
            curser.setShape(Qt::OpenHandCursor);
        }
        this->setCursor(curser);
    }
}

void CustomQGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    this->_mouse_mode = MouseMode::NO_MODE;
    this->_mouse_button_last_position.remove(event->button());
    if (event->button() == Qt::LeftButton) {
        QCursor curser;
        curser.setShape(Qt::ArrowCursor);
        this->setCursor(curser);
    }
}

void CustomQGLWidget::drawMeasurement(
    std::shared_ptr< Measurement > m, qreal penWidth, QPointF offset)
{
    bool redrawAgain = false;
    bool anyVisible = false;
    for (const auto visible : m->visible) {
        anyVisible |= visible;
    }
    if (!anyVisible) {
        return;
    }
    double leftBoundTime = MACRO_LEFTBOUNDTIME();
    double rightBoundTime = MACRO_RIGHTBOUNDTIME();
    auto offsetX = std::minmax_element(m->offsetX.begin(), m->offsetX.end());
    double minOffsetX = *offsetX.first;
    double maxOffsetX = *offsetX.second;

    auto mouseXPos = MACRO_LEFTWINDOW() + this->_mouse_last_position.x();
    auto mouseYPos = MACRO_UPPERWINDOW() - this->_mouse_last_position.y();
    double begin = leftBoundTime + minOffsetX;
    double end = rightBoundTime + maxOffsetX;
    if (end < 0) {
        return;
    }
    auto valuePerSquareScale = (qreal(1) / this->_value_per_square);
    int_fast32_t resolution = this->drawResolution();

    std::future< std::vector< rlib::common::sample > > datumFuture =
        std::async(std::launch::async, [&]() {
            // Load Data from Reader
            auto data = m->reader->samples(begin - 1.0, end + 1.0, resolution);
            return data;
        });
    std::future_status datumStatus =
        datumFuture.wait_for(std::chrono::microseconds(10000));
    if (datumStatus == std::future_status::ready) {
        this->_datum_cache = datumFuture.get();
    }
    else {
        // TODO: Draw load information
        redrawAgain = true;
    }
    auto data = this->_datum_cache;

    auto yTimesValuePerScale = this->_square.y() * valuePerSquareScale;

    auto sensors = m->reader->sensors();
    auto sensorsSize = sensors.size();
    for (size_t i = 0; i < sensorsSize; ++i) {
        if (!m->visible.at(i)) {
            continue;
        }
        QVector< QPointF > polyline;
        QPointF circle;
        double circleTime = 0.0;
        double circleValue = 0.0;
        bool drawCircle = false;
        for (auto& datum : data) {
            // HOTFIX: Somewhere in the codebasis is an error in which case
            // datum.values.size() != sensorsSize!!!
            if (datum.values.size() <= i) {
                continue;
            }

            double xTime = datum.time + m->offsetX.at(i);
            double x = MACRO_TIME_TO_X(xTime);
            double y = 0.0; //(datum.values.at(i) + m->offsetY.at(i)) *
                            // yTimesValuePerScale;
            {
                y += datum.values.at(i);
                y += m->offsetY.at(i);
                y *= yTimesValuePerScale;
            }
            if (fabs(x - mouseXPos) < 3 && fabs(y - mouseYPos) < 3) {
                circleTime = xTime;
                circleValue = datum.values.at(i) + m->offsetY.at(i);
                drawCircle = true;
                circle = QPointF(x, y);
            }
            QPointF p(x + offset.x(), y + offset.y());
            polyline.push_back(p);
        }
        if (!polyline.isEmpty()) {
            QPainter painter(this);
            MACRO_CONFIG_QPAINTER(painter);
            QPen pen(m->color.at(i), penWidth);
            if (m->line_types[ i ] == LINE_TYPE::DASHED) {
                pen.setStyle(Qt::DashLine);
            }
            else if (m->line_types[ i ] == LINE_TYPE::SOLID) {
                pen.setStyle(Qt::SolidLine);
            }
            painter.setPen(pen);
            painter.drawPolyline(polyline.data(), polyline.size());
            if (drawCircle) {
                painter.drawEllipse(circle, qreal(5), qreal(5));

                painter.setFont(QFont("Arial", 9, QFont::Bold));
                painter.setWorldMatrixEnabled(false);
                auto point = circle;
                {
                    point.rx() += 10;
                    point.ry() = -point.y() - 10;
                }
                painter.drawText(point,
                    "Time:" + util::format_time(circleTime) + " Value:" +
                        util::format_number(circleValue,
                            QString::fromStdString(sensors[ i ].unit)));
                painter.setWorldMatrixEnabled(true);
            }
        }
    }

    // Draw Events
    std::future< std::vector< rlib::common::event_data > > eventFuture =
        std::async(std::launch::async, [&]() {
            // Load Data from Reader
            return m->reader->events(begin - 1.0, end + 1.0);
        });
    std::future_status eventStatus =
        eventFuture.wait_for(std::chrono::microseconds(1000));
    if (eventStatus == std::future_status::ready) {
        this->_event_cache = eventFuture.get();
    }
    else {
        // TODO: Draw load information
        redrawAgain = true;
    }
    auto events = this->_event_cache;

    for (auto& event : events) {
        this->drawEvent(m, event, penWidth);
    }
    if (redrawAgain) {
        // this->update();
    }
}

void CustomQGLWidget::drawProbe(
    std::shared_ptr< Probe > p, QString lable, qreal penWidth, QPointF offset)
{
    QPainter painter(this);
    MACRO_CONFIG_QPAINTER(painter);

    qreal leftBound = MACRO_LEFTBOUND();
    qreal rightBound = MACRO_RIGHTBOUND();
    qreal upperBound = MACRO_UPPERBOUND();
    qreal lowerBound = MACRO_LOWERBOUND();

    QPen probePen(this->_configuration->color[ COLOR_CFG::PROBE ], penWidth);
    QPen fontPan(
        this->_configuration->color[ COLOR_CFG::DEFAULT_FONT_COLOR ], penWidth);

    painter.setPen(probePen);
    auto xpos = (p->time / this->_time_per_square) * this->_square.x();
    QPointF p1(xpos, upperBound);
    QPointF p2(xpos, lowerBound);
    painter.drawLine(p1, p2);

    painter.setPen(fontPan);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.setWorldMatrixEnabled(false);
    QPointF point(xpos + 5 + offset.x(),
        -MACRO_UPPERWINDOW() + double(1.8) * this->_square.y() + offset.y());
    painter.drawText(point, lable);
    painter.setWorldMatrixEnabled(true);
}

void CustomQGLWidget::drawEvent(std::shared_ptr< Measurement > m,
    rlib::common::event_data& e, qreal penWidth, QPointF offset)
{
    QPainter painter(this);
    MACRO_CONFIG_QPAINTER(painter);
    auto eventTime = e.time;
    QPen pen(this->_configuration->color[ COLOR_CFG::EVENT ], penWidth);
    if (e.origin >= 0) {
        pen.setColor(m->color.at(static_cast< size_t >(e.origin)));
        eventTime += m->offsetX.at(static_cast< size_t >(e.origin));
    }
    painter.setPen(pen);
    auto xpos = MACRO_TIME_TO_X(eventTime);
    QPointF p1(xpos + offset.x(), MACRO_UPPERBOUND() + offset.y());
    QPointF p2(xpos + offset.x(), MACRO_LOWERBOUND() + offset.y());
    painter.drawLine(p1, p2);
}

void CustomQGLWidget::drawGrid(qreal penWidth, QPointF offset)
{
    QPainter painter(this);
    MACRO_CONFIG_QPAINTER(painter);

    // Build Grid
    QPen gridPen(this->_configuration->color[ COLOR_CFG::GRID ], penWidth);
    QVector< QLineF > lines;
    qreal leftBound = MACRO_LEFTBOUND();
    qreal rightBound = MACRO_RIGHTBOUND();
    qreal upperBound = MACRO_UPPERBOUND();
    qreal lowerBound = MACRO_LOWERBOUND();

    for (int i =
             static_cast< int >(std::ceil(upperBound / this->_square.y())) + 1;
         i * this->_square.y() > lowerBound; --i) {
        QLineF line(leftBound, i * this->_square.y(), rightBound,
            i * this->_square.y());
        lines.push_back(line);
    }

    for (int i =
             static_cast< int >(std::ceil(rightBound / this->_square.x())) + 1;
         i * this->_square.y() > leftBound; --i) {
        QLineF line(i * this->_square.x(), upperBound, i * this->_square.x(),
            lowerBound);
        lines.push_back(line);
    }

    // Draw Grid
    painter.setPen(gridPen);
    painter.drawLines(lines);

    // Draw Middleline (if visible)
    if (lowerBound < 0.0 && upperBound > 0.0) {
        QPen middleLinePen(
            this->_configuration->color[ COLOR_CFG::ZERO_LINE ], penWidth);
        painter.setPen(middleLinePen);
        QPointF p1(leftBound + offset.x(), 0.0 + offset.y());
        QPointF p2(rightBound + offset.x(), 0.0 + offset.y());
        painter.drawLine(p1, p2);
    }
}

void CustomQGLWidget::drawGridLables(qreal penWidth, QPointF offset)
{
    qreal leftBound = MACRO_LEFTBOUND();
    qreal rightBound = MACRO_RIGHTBOUND();
    qreal upperBound = MACRO_UPPERBOUND();
    qreal lowerBound = MACRO_LOWERBOUND();

    QPainter painter(this);
    MACRO_CONFIG_QPAINTER(painter);
    {
        painter.setPen(
            { this->_configuration->color[ COLOR_CFG::DEFAULT_FONT_COLOR ],
                penWidth });
        painter.setFont(this->_configuration->font[ FONT_CFG::DEFAULT_FONT ]);
    }

    QVector< std::tuple< QPointF, QString > > labels;

    // Collect Labels (Y)
    for (int i =
             static_cast< int >(std::ceil(upperBound / this->_square.y())) + 1;
         i * this->_square.y() > lowerBound; --i) {
        // Note! Y-Axis is inverted manualy here to prevent text flip!
        auto point =
            QPointF(leftBound + double(1.1) * this->_square.x() + offset.x(),
                -i * this->_square.y() + offset.y());
        QString string;
        if (i % 2 != 0) {
            string = util::format_number(
                static_cast< double >(i) *
                static_cast< double >(this->_value_per_square));
        }
        labels.push_back({ point, string });
    }
    // Collect Labels (X)
    for (int i =
             static_cast< int >(std::ceil(rightBound / this->_square.x())) + 1;
         i * this->_square.y() > leftBound; --i) {
        // Note! Y-Axis is inverted manualy here to prevent text flip!
        auto point = QPointF(i * this->_square.x() + offset.y(),
            -lowerBound - double(1.1) * this->_square.y() + offset.y());
        double time = (i * this->_square.x() * this->_time_per_square) /
                      this->_square.y();
        QString string;
        if (i % 4 == 0) {
            string = util::format_time(time);
        }
        labels.push_back({ point, string });
    }

    // Draw Labels
    painter.setWorldMatrixEnabled(false);
    for (auto& label : labels) {
        auto point = std::get< QPointF >(label);
        auto string = std::get< QString >(label);
        painter.drawText(point, string);
    }
    painter.setWorldMatrixEnabled(true);
}
