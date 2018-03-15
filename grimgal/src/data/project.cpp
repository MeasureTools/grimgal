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

// Own
#include "data/project.h"

// StdLib
#include <algorithm>

void Project::clear()
{
    this->file = "";
    this->measurements.clear();
    this->measurements.shrink_to_fit();
    this->probes.clear();
    this->probes.shrink_to_fit();

    this->updatedProject();
}

bool Project::addMeasurement(std::shared_ptr< Measurement > m)
{
    this->measurements.push_back(m);
    size_t index = this->measurements.size() - 1;
    this->addedMeasurement(m, index);
    return true;
}
bool Project::removeMeasurement(std::shared_ptr< Measurement > m)
{
    for (size_t index = 0; index < this->measurements.size(); ++index) {
        if (this->measurements[ index ].get() == m.get()) {
            return this->removeMeasurement(index);
        }
    }
    return false;
}
bool Project::removeMeasurement(size_t index)
{
    if (index >= this->measurements.size()) {
        return false;
    }
    auto m = this->measurements[ index ];
    this->measurements.erase(this->measurements.begin() + index);
    this->removedMeasurement(m, index);
    return true;
}

bool Project::addProbe(std::shared_ptr< Probe > p)
{
    this->probes.push_back(p);
    size_t index = this->probes.size() - 1;
    this->addedProbe(p, index);
    return true;
}
bool Project::removeProbe(std::shared_ptr< Probe > p)
{
    for (size_t index = 0; index < this->probes.size(); ++index) {
        if (this->probes[ index ].get() == p.get()) {
            return this->removeProbe(index);
        }
    }
    return false;
}
bool Project::removeProbe(size_t index)
{
    if (index >= this->probes.size()) {
        return false;
    }
    auto p = this->probes[ index ];
    this->probes.erase(this->probes.begin() + index);
    this->removedProbe(p, index);
    return true;
}
