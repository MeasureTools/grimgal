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
#include <QAbstractButton>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>

// Own
#include "data/configuration.h"
#include "form/settings_dialog.h"
#include "model/settings_dialog_color_model.h"
#include "ui_settings_dialog.h"

// StdLib
#include <memory>

#include <iostream>

settings_dialog::settings_dialog(
    std::shared_ptr< Configuration > configuration, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::settings_dialog)
{
    // Initial Ui setup
    this->_ui->setupUi(this);

    // Set members
    this->_configuration = configuration;

    // Set Members :: Set Models
    this->_color_model = std::make_unique< settings_dialog_color_model >(
        this->_configuration, this);

    // Config Ui :: Set Models
    this->_ui->color_table_view->setModel(this->_color_model.get());
}

settings_dialog::~settings_dialog()
{
    delete _ui;
}

void settings_dialog::accept()
{
    std::cout << "accept" << std::endl;
    this->_old_settings.clear();
    QDialog::accept();
}

void settings_dialog::reject()
{
    std::cout << "reject" << std::endl;
    for (auto& color_cfg_pair : this->_old_settings) {
        this->_configuration->color[ color_cfg_pair.first ] =
            color_cfg_pair.second;
    }
    this->parentWidget()->update();
    QDialog::reject();
}

void settings_dialog::change_color(QModelIndex index)
{

    auto old_color = this->_configuration->color[ COLOR_CFG(index.row()) ];
    auto new_color = QColorDialog::getColor(old_color, this);
    if (this->_old_settings.find(COLOR_CFG(index.row())) ==
        this->_old_settings.end()) {
        this->_old_settings[ COLOR_CFG(index.row()) ] = old_color;
    }
    this->_configuration->color[ COLOR_CFG(index.row()) ] = new_color;
    this->parentWidget()->activateWindow();
    this->activateWindow();
}

void settings_dialog::button_clicked(QAbstractButton* button)
{
    if (this->_ui->dialog_buttons->standardButton(button) ==
        QDialogButtonBox::RestoreDefaults) {
        Configuration default_cfg;
        this->_configuration->color = default_cfg.color;
        this->parentWidget()->activateWindow();
        this->activateWindow();
    }
}