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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Qt
#include <QColor>
#include <QFont>
#include <QObject>

// StdLib
#include <map>
#include <string>

enum COLOR_CFG {
    BACKGROUND,
    GRID,
    ZERO_LINE,
    PROBE,
    EVENT,
    DEFAULT_FONT_COLOR,

    __COLOR_CFG_COUNT
};

enum FONT_CFG {
    DEFAULT_FONT,

    __FONT_CFG_COUNT
};

inline std::string to_string(COLOR_CFG c)
{
    switch (c) {
        case BACKGROUND:
            return QObject::tr("Background").toStdString();
        case GRID:
            return QObject::tr("Grid").toStdString();
        case ZERO_LINE:
            return QObject::tr("Zero line").toStdString();
        case PROBE:
            return QObject::tr("Probe").toStdString();
        case EVENT:
            return QObject::tr("Event").toStdString();
        case DEFAULT_FONT_COLOR:
            return QObject::tr("Default Font").toStdString();
    }
    return "";
}

inline std::string to_string(FONT_CFG c)
{
    switch (c) {
        case DEFAULT_FONT:
            return QObject::tr("Default Font").toStdString();
    }
    return "";
}

class Configuration {
    public:
    std::map< COLOR_CFG, QColor > color = {
        { BACKGROUND, QColor(0, 30, 30) }, { GRID, QColor(0, 60, 60) },
        { ZERO_LINE, Qt::yellow }, { PROBE, Qt::white }, { EVENT, Qt::red },
        { DEFAULT_FONT_COLOR, Qt::white },
    };

    std::map< FONT_CFG, QFont > font = {
        { DEFAULT_FONT, QFont("Arial", 9, QFont::Bold) },
    };

    // Other
    bool _use_cached_reader = true;
    bool _use_statistic_reader = false;
};

#endif // CONFIGURATION_H
