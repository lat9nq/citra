// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <QWidget>
#include "common/common_types.h"

namespace Core {
class System;
}

namespace Settings {
enum class StereoRenderOption : u32;
}

namespace ConfigurationShared {
class Builder;
}

namespace Ui {
class ConfigureEnhancements;
}

class QComboBox;
class QCheckBox;

class ConfigureEnhancements : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureEnhancements(ConfigurationShared::Builder& builder,
                                   const Core::System& system, QWidget* parent = nullptr);
    ~ConfigureEnhancements();

    void ApplyConfiguration();
    void RetranslateUI();
    void SetConfiguration();

    void Setup(ConfigurationShared::Builder& builder);

private:
    void UpdateShaders(Settings::StereoRenderOption stereo_option);
    void updateTextureFilter(int index);

    std::unique_ptr<Ui::ConfigureEnhancements> ui;
    QColor bg_color;

    std::vector<std::function<void(bool)>> apply_funcs{};

    const Core::System& system;

    QComboBox* shader_combobox;
    QComboBox* render_3d_combobox;
    QComboBox* resolution_factor_combobox;
    QCheckBox* toggle_preload_textures;
    QCheckBox* toggle_async_custom_loading;
    QCheckBox* toggle_custom_textures;
};
