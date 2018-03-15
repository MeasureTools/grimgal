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
        class meta_line_type : public iproperty {
            public:
            virtual std::string name() const
            {
                return QObject::tr("Line type").toStdString();
            }

            virtual bool set_allowed() const
            {
                return true;
            }

            virtual bool set_data(std::shared_ptr< Measurement > m,
                std::experimental::optional< size_t > idx, QVariant const& v)
            {
                if (!idx) {
                    return false;
                }
                if (to_string(LINE_TYPE::SOLID) == v.toString().toStdString()) {
                    m->line_types[ *idx ] = LINE_TYPE::SOLID;
                    return true;
                }
                if (to_string(LINE_TYPE::DASHED) ==
                    v.toString().toStdString()) {
                    m->line_types[ *idx ] = LINE_TYPE::DASHED;
                    return true;
                }
                return false;
            }

            virtual QVariant get_data(std::shared_ptr< Measurement > m,
                std::experimental::optional< size_t > idx) const
            {
                if (!idx) {
                    return {};
                }
                return QString::fromStdString(to_string(m->line_types[ *idx ]));
            }
        };
    }
}
