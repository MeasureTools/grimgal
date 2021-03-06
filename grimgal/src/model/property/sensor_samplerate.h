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

#pragma once

// Qt
#include <QObject>
#include <QString>
#include <QVariant>

// Own
#include "data/measurement.h"
#include "model/property/iproperty.h"

// StdLib
#include <experimental/optional>
#include <memory>

namespace model {
    namespace property {
        class sensor_samplerate : public iproperty {
            public:
            virtual std::string name() const
            {
                return QObject::tr("Samplerate").toStdString();
            }

            virtual QVariant get_data(std::shared_ptr< Measurement > m,
                std::experimental::optional< size_t > idx) const
            {
                if (!idx) {
                    return QVariant();
                }
                auto sampling_interval =
                    m->reader->sensors().at(0).sampling_interval;
                if (sampling_interval <= 0.0) {
                    return QVariant(QObject::tr("N/A"));
                }
                return QVariant(1.0 / sampling_interval);
            }
        };
    }
}
