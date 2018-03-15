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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

// Qt
#include <QAbstractButton>
#include <QColor>
#include <QDialog>

// Own
#include "data/configuration.h"
#include "model/settings_dialog_color_model.h"

// StdLib
#include <map>
#include <memory>

// Macros

namespace Ui {
    class settings_dialog;
}

class settings_dialog : public QDialog {
    Q_OBJECT

    private:
    Ui::settings_dialog* _ui;
    std::shared_ptr< Configuration > _configuration;
    std::map< COLOR_CFG, QColor > _old_settings;

    // Models
    std::unique_ptr< settings_dialog_color_model > _color_model;

    public:
    explicit settings_dialog(
        std::shared_ptr< Configuration > configuration, QWidget* parent = 0);
    ~settings_dialog();

    public slots:
    virtual void accept() override final;
    virtual void reject() override final;

    virtual void change_color(QModelIndex index);
    virtual void button_clicked(QAbstractButton* button);
};

#endif // SETTINGS_DIALOG_H
