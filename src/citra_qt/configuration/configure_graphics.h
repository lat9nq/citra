// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include <memory>
#include <span>
#include <vector>
#include <QString>
#include <QWidget>

namespace Core {
class System;
}

namespace Ui {
class ConfigureGraphics;
}

namespace ConfigurationShared {
class Builder;
} // namespace ConfigurationShared

class QComboBox;
class QCheckBox;

class ConfigureGraphics : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureGraphics(const ConfigurationShared::Builder& builder,
                               std::span<const QString> physical_devices,
                               const Core::System& system, QWidget* parent = nullptr);
    ~ConfigureGraphics() override;

    void ApplyConfiguration();
    void RetranslateUI();
    void SetConfiguration();

    void UpdateBackgroundColorButton(const QColor& color);

private:
    void Setup();
    void SetPhysicalDeviceComboVisibility(int index);

    std::unique_ptr<Ui::ConfigureGraphics> ui;
    QColor bg_color;

    std::vector<std::function<void(bool)>> apply_funcs;
    const Core::System& system;
    const ConfigurationShared::Builder& builder;

    QComboBox* graphics_api_combo;
    QComboBox* physical_device_combo;
    QCheckBox* toggle_hw_shader;
    QWidget* toggle_shaders_accurate_mul;
    QWidget* toggle_disk_shader_cache;
};
