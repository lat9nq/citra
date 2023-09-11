// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <vector>
#include <QWidget>

namespace Ui {
class ConfigureAudio;
}

namespace ConfigurationShared {
class Builder;
}

namespace Core {
class System;
}

class QComboBox;

class ConfigureAudio : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureAudio(ConfigurationShared::Builder& builder, const Core::System& system,
                            QWidget* parent = nullptr);
    ~ConfigureAudio() override;

    void ApplyConfiguration();
    void RetranslateUI();
    void SetConfiguration();

private:
    void UpdateAudioOutputDevices(int sink_index);
    void UpdateAudioInputDevices(int index);

    void SetOutputTypeFromSinkType();
    void SetOutputDeviceFromDeviceID();
    void SetInputTypeFromInputType();
    void SetInputDeviceFromDeviceID();

    void Setup(ConfigurationShared::Builder& builder);

    std::unique_ptr<Ui::ConfigureAudio> ui;
    QComboBox* output_device_combo_box;
    QComboBox* input_device_combo_box;
    QComboBox* output_type_combo_box;
    QComboBox* input_type_combo_box;

    std::vector<std::function<void(bool)>> apply_funcs{};

    const Core::System& system;
};
