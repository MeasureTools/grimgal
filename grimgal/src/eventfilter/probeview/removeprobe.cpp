/**
 * Copyright (c) 2017, Daniel "Dadie" Korner
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
#include <QEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QTableView>

// Own
#include "data/configuration.h"
#include "data/project.h"
#include "eventfilter/probeview/removeprobe.h"

// StdLib
#include <memory>

eventfilter::probeview::RemoveProbe::RemoveProbe(
    std::shared_ptr< Configuration > configuration,
    std::shared_ptr< Project > project)
{
    this->_configuration = configuration;
    this->project = project;
}

bool eventfilter::probeview::RemoveProbe::eventFilter(
    QObject* obj, QEvent* event)
{
    if (event->type() != QEvent::KeyPress) {
        return false;
    }
    QKeyEvent* keyEvent = static_cast< QKeyEvent* >(event);
    if (keyEvent->key() != Qt::Key_Delete) {
        return false;
    }
    if (!obj->inherits("QTableView")) {
        return false;
    }
    QTableView* tableView = static_cast< QTableView* >(obj);
    auto cols = tableView->selectionModel()->selectedColumns();
    if (cols.isEmpty()) {
        return false;
    }

    QString probes = "Probe " + QString::number(cols[ 0 ].column());
    for (int i = 1; i < cols.size(); ++i) {
        probes += ", Probe " + QString::number(cols[ i ].column());
    }

    QMessageBox::StandardButton res = QMessageBox::question(nullptr,
        cols.size() > 1 ? QObject::tr("Delete Probes")
                        : QObject::tr("Delete Probe"),
        probes, QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        for (int i = cols.size(); i > 0; --i) {
            size_t index = static_cast< size_t >(cols[ i - 1 ].column());
            this->project->removeProbe(index);
            tableView->selectionModel()->clearSelection();
        }
    }

    return false;
}
