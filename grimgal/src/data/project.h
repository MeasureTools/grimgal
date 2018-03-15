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

#ifndef PROJECT_H
#define PROJECT_H

// Qt
#include <QObject>

// Own
#include "data/measurement.h"
#include "data/probe.h"

// StdLib
#include <memory>
#include <vector>

class Project : public QObject {
    Q_OBJECT

    public:
    QString file;
    std::vector< std::shared_ptr< Measurement > > measurements;
    std::vector< std::shared_ptr< Probe > > probes;

    signals:
    void addedMeasurement(std::shared_ptr< Measurement > m, size_t index);
    void updatedMeasurement(std::shared_ptr< Measurement > m, size_t index);
    void removedMeasurement(std::shared_ptr< Measurement > m, size_t index);

    void updatedProject();

    void addedProbe(std::shared_ptr< Probe > p, size_t index);
    void updatedProbe(std::shared_ptr< Probe > p, size_t index);
    void removedProbe(std::shared_ptr< Probe > p, size_t index);

    public:
    bool addMeasurement(std::shared_ptr< Measurement > m);
    bool removeMeasurement(std::shared_ptr< Measurement > m);
    bool removeMeasurement(size_t index);

    bool addProbe(std::shared_ptr< Probe > p);
    bool removeProbe(std::shared_ptr< Probe > p);
    bool removeProbe(size_t index);

    void clear();
};

#endif // PROJECT_H
