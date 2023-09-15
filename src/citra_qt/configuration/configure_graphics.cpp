// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/configure_graphics.h"

#include <vector>
#include <QColorDialog>
#include <QPushButton>
#include <QWidget>
#include "citra_qt/configuration/configuration_shared.h"
#include "citra_qt/configuration/shared_widget.h"
#include "common/common_types.h"
#include "common/settings.h"
#include "common/settings_common.h"
#include "common/settings_enums.h"
#include "core/core.h"
#include "ui_configure_graphics.h"
#include "video_core/renderer_vulkan/vk_instance.h"

ConfigureGraphics::ConfigureGraphics(const ConfigurationShared::Builder& builder_,
                                     std::span<const QString> physical_devices,
                                     const Core::System& system_, QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureGraphics>()), system{system_},
      builder{builder_} {
    ui->setupUi(this);

    Setup();

    for (const QString& name : physical_devices) {
        physical_device_combo->addItem(name);
    }

    if (physical_devices.empty()) {
        const auto& translations = builder.ComboboxTranslations();
        const auto& api_translations =
            translations.at(Settings::EnumMetadata<Settings::GraphicsAPI>::Index());
        const u32 index = [&]() {
            for (int i = 0; i < api_translations.size(); i++) {
                if (api_translations.at(i).first ==
                    static_cast<u32>(Settings::GraphicsAPI::Vulkan)) {
                    return i;
                }
            }
            return -1;
        }();
        graphics_api_combo->removeItem(index);
        ui->deviceWidget->setVisible(false);
    }

    connect(graphics_api_combo, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                const auto& pair = builder.ComboboxTranslations()
                                       .at(Settings::EnumMetadata<Settings::GraphicsAPI>::Index())
                                       .at(index);
                const auto graphics_api = static_cast<Settings::GraphicsAPI>(pair.first);
                const bool is_software = graphics_api == Settings::GraphicsAPI::Software;

                toggle_hw_shader->setEnabled(!is_software);
                toggle_shaders_accurate_mul->setEnabled(!is_software);
                toggle_disk_shader_cache->setEnabled(!is_software && toggle_hw_shader->isChecked());
            });

    connect(toggle_hw_shader, &QCheckBox::toggled, this, [this] {
        const bool checked = toggle_hw_shader->isChecked();
        toggle_shaders_accurate_mul->setEnabled(checked);
        toggle_disk_shader_cache->setEnabled(checked);
    });

    connect(graphics_api_combo, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &ConfigureGraphics::SetPhysicalDeviceComboVisibility);
    SetPhysicalDeviceComboVisibility(graphics_api_combo->currentIndex());

    SetConfiguration();

    const auto api_widget_enable = [this, &builder](int index) {
        const auto& pair = builder.ComboboxTranslations()
                               .at(Settings::EnumMetadata<Settings::GraphicsAPI>::Index())
                               .at(index);
        const auto graphics_api = static_cast<Settings::GraphicsAPI>(pair.first);
        const bool is_software = graphics_api == Settings::GraphicsAPI::Software;

        toggle_hw_shader->setEnabled(!is_software);
        toggle_shaders_accurate_mul->setEnabled(!is_software);
        toggle_disk_shader_cache->setEnabled(!is_software && toggle_hw_shader->isChecked());
    };

    const auto hw_shader_widget_enable = [this] {
        const bool checked = toggle_hw_shader->isChecked();
        toggle_shaders_accurate_mul->setEnabled(checked);
        toggle_disk_shader_cache->setEnabled(checked);
    };

    connect(graphics_api_combo, qOverload<int>(&QComboBox::currentIndexChanged), this,
            api_widget_enable);

    connect(toggle_hw_shader, &QCheckBox::toggled, this, hw_shader_widget_enable);
    hw_shader_widget_enable();
    api_widget_enable(graphics_api_combo->currentIndex());
}

ConfigureGraphics::~ConfigureGraphics() = default;

void ConfigureGraphics::SetConfiguration() {}

void ConfigureGraphics::ApplyConfiguration() {
    const bool powered_on = system.IsPoweredOn();
    for (const auto& func : apply_funcs) {
        func(powered_on);
    }
}

void ConfigureGraphics::RetranslateUI() {
    ui->retranslateUi(this);
}

void ConfigureGraphics::Setup() {
    std::vector<Settings::BasicSetting*> settings;
    const std::vector<Settings::Category> categories = {
        Settings::Category::Renderer, Settings::Category::RendererAdvanced,
        Settings::Category::RendererApi, Settings::Category::RendererDevice};
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
        case Settings::Category::Renderer:
            ui->rendererBox->layout()->addWidget(widget);
            break;
        case Settings::Category::RendererAdvanced:
            ui->advancedBox->layout()->addWidget(widget);
            break;
        case Settings::Category::RendererApi:
            ui->apiLayout->addWidget(widget);
            break;
        case Settings::Category::RendererDevice:
            ui->deviceWidget->layout()->addWidget(widget);
            break;
        default:
            widget->deleteLater();
            continue;
        }

        if (setting->Id() == Settings::values.graphics_api.Id()) {
            graphics_api_combo = widget->combobox;
        } else if (setting->Id() == Settings::values.use_hw_shader.Id()) {
            toggle_hw_shader = widget->checkbox;
        } else if (setting->Id() == Settings::values.shaders_accurate_mul.Id()) {
            toggle_shaders_accurate_mul = widget;
        } else if (setting->Id() == Settings::values.use_disk_shader_cache.Id()) {
            toggle_disk_shader_cache = widget;
        } else if (setting->Id() == Settings::values.physical_device.Id()) {
            physical_device_combo = widget->combobox;
            if (!Settings::IsConfiguringGlobal()) {
                auto restore_global_button = ConfigurationShared::Widget::CreateRestoreGlobalButton(
                    Settings::values.physical_device.UsingGlobal(), widget);
                connect(physical_device_combo, QOverload<int>::of(&QComboBox::activated),
                        [restore_global_button] {
                            restore_global_button->setVisible(true);
                            restore_global_button->setEnabled(true);
                        });
                connect(restore_global_button, &QAbstractButton::clicked,
                        [this, restore_global_button]() {
                            const auto default_index =
                                Settings::values.physical_device.GetValue(true);
                            physical_device_combo->setCurrentIndex(default_index);
                            restore_global_button->setVisible(false);
                            restore_global_button->setEnabled(false);
                        });
                widget->layout()->addWidget(restore_global_button);
            }
        }
    }
}

void ConfigureGraphics::SetPhysicalDeviceComboVisibility(int index) {
    bool is_visible{};

    // When configuring per-game the physical device combo should be
    // shown either when the global api is used and that is Vulkan or
    // Vulkan is set as the per-game api.
    const auto graphics_api = static_cast<Settings::GraphicsAPI>(
        builder.ComboboxTranslations()
            .at(Settings::EnumMetadata<Settings::GraphicsAPI>::Index())
            .at(index)
            .first);
    is_visible = graphics_api == Settings::GraphicsAPI::Vulkan;
    ui->deviceWidget->setVisible(is_visible);
}
