// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/configure_audio.h"

#include <memory>
#include <QtGlobal>
#include "audio_core/input_details.h"
#include "audio_core/sink.h"
#include "audio_core/sink_details.h"
#include "citra_qt/configuration/configuration_shared.h"
#include "citra_qt/configuration/shared_widget.h"
#include "common/settings.h"
#include "common/settings_common.h"
#include "common/settings_enums.h"
#include "core/core.h"
#include "ui_configure_audio.h"

#if defined(__APPLE__)
#include "common/apple_authorization.h"
#endif

ConfigureAudio::ConfigureAudio(ConfigurationShared::Builder& builder, const Core::System& system_,
                               QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureAudio>()), system{system_} {
    ui->setupUi(this);

    Setup(builder);

    if (!Settings::IsConfiguringGlobal()) {
        ui->input_group->setVisible(false);
        return;
    }

    output_type_combo_box->clear();
    for (u32 type = 0; type < static_cast<u32>(AudioCore::SinkType::NumSinkTypes); type++) {
        output_type_combo_box->addItem(QString::fromUtf8(
            AudioCore::GetSinkName(static_cast<AudioCore::SinkType>(type)).data()));
    }

    input_type_combo_box->clear();
    for (u32 type = 0; type < static_cast<u32>(AudioCore::InputType::NumInputTypes); type++) {
        input_type_combo_box->addItem(QString::fromUtf8(
            AudioCore::GetInputName(static_cast<AudioCore::InputType>(type)).data()));
    }

    SetConfiguration();

    connect(output_type_combo_box, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &ConfigureAudio::UpdateAudioOutputDevices);
    connect(input_type_combo_box, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &ConfigureAudio::UpdateAudioInputDevices);
}

ConfigureAudio::~ConfigureAudio() {}

void ConfigureAudio::SetConfiguration() {
    SetOutputTypeFromSinkType();
    SetInputTypeFromInputType();

    // The device list cannot be pre-populated (nor listed) until the output sink is known.
    UpdateAudioOutputDevices(output_type_combo_box->currentIndex());
    UpdateAudioInputDevices(input_type_combo_box->currentIndex());
    SetOutputDeviceFromDeviceID();
    SetInputDeviceFromDeviceID();
}

void ConfigureAudio::SetOutputTypeFromSinkType() {
    output_type_combo_box->setCurrentIndex(
        static_cast<int>(Settings::values.output_type.GetValue()));
}

void ConfigureAudio::SetOutputDeviceFromDeviceID() {
    int new_device_index = -1;

    const QString device_id = QString::fromStdString(Settings::values.output_device.GetValue());
    for (int index = 0; index < output_device_combo_box->count(); index++) {
        if (output_device_combo_box->itemText(index) == device_id) {
            new_device_index = index;
            break;
        }
    }

    output_device_combo_box->setCurrentIndex(new_device_index);
}

void ConfigureAudio::SetInputTypeFromInputType() {
    input_type_combo_box->setCurrentIndex(static_cast<int>(Settings::values.input_type.GetValue()));
}

void ConfigureAudio::SetInputDeviceFromDeviceID() {
    int new_device_index = -1;

    const QString device_id = QString::fromStdString(Settings::values.input_device.GetValue());
    for (int index = 0; index < input_device_combo_box->count(); index++) {
        if (input_device_combo_box->itemText(index) == device_id) {
            new_device_index = index;
            break;
        }
    }

    input_device_combo_box->setCurrentIndex(new_device_index);
}

void ConfigureAudio::ApplyConfiguration() {
    const bool is_powered_on = system.IsPoweredOn();
    for (const auto& func : apply_funcs) {
        func(is_powered_on);
    }

    if (Settings::IsConfiguringGlobal()) {
        Settings::values.output_device = output_device_combo_box->currentText().toStdString();
        Settings::values.input_device = input_device_combo_box->currentText().toStdString();
    }
}

void ConfigureAudio::UpdateAudioOutputDevices(int sink_index) {
    auto sink_type = static_cast<AudioCore::SinkType>(sink_index);

    output_device_combo_box->clear();
    output_device_combo_box->addItem(QString::fromUtf8(AudioCore::auto_device_name));

    for (const auto& device : AudioCore::GetDeviceListForSink(sink_type)) {
        output_device_combo_box->addItem(QString::fromStdString(device));
    }
}

void ConfigureAudio::UpdateAudioInputDevices(int input_index) {
    auto input_type = static_cast<AudioCore::InputType>(input_index);

#if defined(__APPLE__)
    if (input_type != AudioCore::InputType::Null && input_type != AudioCore::InputType::Static) {
        AppleAuthorization::CheckAuthorizationForMicrophone();
    }
#endif

    input_device_combo_box->clear();
    input_device_combo_box->addItem(QString::fromUtf8(AudioCore::auto_device_name));

    for (const auto& device : AudioCore::GetDeviceListForInput(input_type)) {
        input_device_combo_box->addItem(QString::fromStdString(device));
    }
}

void ConfigureAudio::RetranslateUI() {
    ui->retranslateUi(this);
}

void ConfigureAudio::Setup(ConfigurationShared::Builder& builder) {
    const std::vector<Settings::Category> categories{Settings::Category::Audio,
                                                     Settings::Category::AudioInput};
    std::vector<Settings::BasicSetting*> settings;
    ConfigurationShared::GroupSettings(settings, categories);

    for (auto* setting : settings) {
        ConfigurationShared::Widget* widget = builder.BuildWidget(setting, apply_funcs);

        if (widget == nullptr) {
            continue;
        } else if (!widget->Valid()) {
            widget->deleteLater();
            continue;
        }

        switch (setting->GetCategory()) {
        case Settings::Category::Audio:
            ui->output_group->layout()->addWidget(widget);
            break;
        case Settings::Category::AudioInput:
            ui->input_group->layout()->addWidget(widget);
            break;
        default:
            widget->deleteLater();
            continue;
        }

        if (setting->Id() == Settings::values.output_device.Id()) {
            output_device_combo_box = widget->combobox;
        } else if (setting->Id() == Settings::values.input_device.Id()) {
            input_device_combo_box = widget->combobox;
        } else if (setting->Id() == Settings::values.output_type.Id()) {
            output_type_combo_box = widget->combobox;
        } else if (setting->Id() == Settings::values.input_type.Id()) {
            input_type_combo_box = widget->combobox;
        }
    }
}
