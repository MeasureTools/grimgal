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

#ifndef MEASURMENT_H
#define MEASURMENT_H

// Qt
#include <QColor>
#include <QObject>
#include <QString>
#include <QtGlobal>

// StdLib
#include <memory>
#include <vector>

// Own
#include <rlib/common/reader.h>

enum class LINE_TYPE : int {
    SOLID,
    DASHED,
};

inline std::string to_string(LINE_TYPE c)
{
    switch (c) {
        case LINE_TYPE::SOLID:
            return QObject::tr("Solid").toStdString();
        case LINE_TYPE::DASHED:
            return QObject::tr("Dashed").toStdString();
    }
    return "";
}

class Measurement {
    public:
    std::shared_ptr< rlib::common::reader > reader;
    // PROPERTIES
    QString name;
    std::vector< QString > sensorName;
    std::vector< qint64 > unitFactor;
    std::vector< double > offsetX;
    std::vector< double > offsetY;
    std::vector< bool > visible;
    std::vector< QColor > color;
    std::vector< QString > comment;
    std::vector< LINE_TYPE > line_types;

    public:
    void setReader(std::shared_ptr< rlib::common::reader > reader);
};

#endif // MEASURMENT_H
