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

// Own
#include "util/number_format.h"

// StdLib

#include <iostream>

QString util::format_number(double d, QString unit)
{
    int e = 0;
    if (d != 0 && std::fabs(d) < 1.0) {
        while (std::fabs(d) < 1.0) {
            e -= 3;
            d *= 1000.0;
        }
    }
    else {
        while (std::fabs(d) > 1000.0) {
            e += 3;
            d /= 1000.0;
        }
    }
    return QString::number(d) + metric_prefix(e) + unit;
}

QString util::format_time(double time)
{
    QString string = "";
    double factor;

    // factor: day
    factor = 60.0 * 60.0 * 24.0;
    if (std::fabs(time) > factor) {
        string += QString::number(std::floor(time / factor)) + "d";
        time -= std::floor(time / factor) * factor;
    }

    // factor : hours
    factor = 60.0 * 60.0;
    if (std::fabs(time) > factor) {
        string += QString::number(std::floor(time / factor)) + "h";
        time -= std::floor(time / factor) * factor;
    }
    else if (!string.isEmpty()) {
        string += "0h";
    }

    // factor : minutes
    factor = 60.0;
    if (std::fabs(time) > factor) {
        string += QString::number(std::floor(time / factor)) + "m";
        time -= std::floor(time / factor) * factor;
    }
    else if (!string.isEmpty()) {
        string += "0m";
    }

    return string + QString::number(time) + "s";
}

QString util::metric_prefix(int e)
{
    switch (e) {
        case 18:
            return "E";
        case 15:
            return "P";
        case 12:
            return "T";
        case 9:
            return "G";
        case 6:
            return "M";
        case 3:
            return "k";
        case 2:
            return "h";
        case 1:
            return "da";
        case 0:
            return "";
        case -1:
            return "d";
        case -2:
            return "c";
        case -3:
            return "m";
        case -6:
            return "µ";
        case -9:
            return "n";
        case -12:
            return "p";
        case -15:
            return "f";
        case -18:
            return "a";
        default:
            return "e+" + QString::number(e);
    }
}
