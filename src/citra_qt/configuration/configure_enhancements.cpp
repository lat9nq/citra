// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/configure_enhancements.h"

#include <array>
#include <QColorDialog>
#include "citra_qt/configuration/configuration_shared.h"
#include "citra_qt/configuration/shared_widget.h"
#include "common/settings.h"
#include "common/settings_enums.h"
#include "common/settings_setting.h"
#include "core/core.h"
#include "ui_configure_enhancements.h"
#include "video_core/renderer_opengl/post_processing_opengl.h"

ConfigureEnhancements::ConfigureEnhancements(ConfigurationShared::Builder& builder,
                                             const Core::System& system_, QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureEnhancements>()), system{system_} {
    ui->setupUi(this);

    Setup(builder);
    SetConfiguration();

    ui->layout_group->setEnabled(!Settings::values.custom_layout);

    const auto graphics_api = Settings::values.graphics_api.GetValue();
    const bool res_scale_enabled = graphics_api != Settings::GraphicsAPI::Software;
    resolution_factor_combobox->setEnabled(res_scale_enabled);

    connect(render_3d_combobox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this, &builder](int current_index) {
                const auto& translations = builder.ComboboxTranslations();
                const auto selected_option = static_cast<Settings::StereoRenderOption>(
                    translations.at(Settings::EnumMetadata<Settings::StereoRenderOption>::Index())
                        .at(current_index)
                        .first);
                UpdateShaders(selected_option);
            });

    connect(ui->bg_button, &QPushButton::clicked, this, [this] {
        const QColor new_bg_color = QColorDialog::getColor(bg_color);
        if (!new_bg_color.isValid()) {
            return;
        }
        bg_color = new_bg_color;
        QPixmap pixmap(ui->bg_button->size());
        pixmap.fill(bg_color);
        const QIcon color_icon(pixmap);
        ui->bg_button->setIcon(color_icon);
    });

    toggle_preload_textures->setEnabled(toggle_custom_textures->isChecked());
    toggle_async_custom_loading->setEnabled(toggle_custom_textures->isChecked());
    connect(toggle_custom_textures, &QCheckBox::toggled, this, [this] {
        toggle_preload_textures->setEnabled(toggle_custom_textures->isChecked());
        toggle_async_custom_loading->setEnabled(toggle_custom_textures->isChecked());
        if (!toggle_preload_textures->isEnabled())
            toggle_preload_textures->setChecked(false);
    });
}

ConfigureEnhancements::~ConfigureEnhancements() = default;

void ConfigureEnhancements::SetConfiguration() {
    bg_color =
        QColor::fromRgbF(Settings::values.bg_red.GetValue(), Settings::values.bg_green.GetValue(),
                         Settings::values.bg_blue.GetValue());
    QPixmap pixmap(ui->bg_button->size());
    pixmap.fill(bg_color);
    const QIcon color_icon(pixmap);
    ui->bg_button->setIcon(color_icon);
}

void ConfigureEnhancements::UpdateShaders(Settings::StereoRenderOption stereo_option) {
    shader_combobox->clear();
    shader_combobox->setEnabled(true);

    if (stereo_option == Settings::StereoRenderOption::Interlaced ||
        stereo_option == Settings::StereoRenderOption::ReverseInterlaced) {
        shader_combobox->addItem(QStringLiteral("horizontal (builtin)"));
        shader_combobox->setCurrentIndex(0);
        shader_combobox->setEnabled(false);
        return;
    }

    std::string current_shader;
    if (stereo_option == Settings::StereoRenderOption::Anaglyph) {
        shader_combobox->addItem(QStringLiteral("dubois (builtin)"));
        current_shader = Settings::values.anaglyph_shader_name.GetValue();
    } else {
        shader_combobox->addItem(QStringLiteral("none (builtin)"));
        current_shader = Settings::values.pp_shader_name.GetValue();
    }

    shader_combobox->setCurrentIndex(0);

    for (const auto& shader : OpenGL::GetPostProcessingShaderList(
             stereo_option == Settings::StereoRenderOption::Anaglyph)) {
        shader_combobox->addItem(QString::fromStdString(shader));
        if (current_shader == shader)
            shader_combobox->setCurrentIndex(shader_combobox->count() - 1);
    }
}

void ConfigureEnhancements::RetranslateUI() {
    ui->retranslateUi(this);
}

void ConfigureEnhancements::ApplyConfiguration() {
    const bool powered_on = system.IsPoweredOn();
    for (const auto& func : apply_funcs) {
        func(powered_on);
    }

    if (Settings::IsConfiguringGlobal()) {
        Settings::values.bg_red = static_cast<float>(bg_color.redF());
        Settings::values.bg_green = static_cast<float>(bg_color.greenF());
        Settings::values.bg_blue = static_cast<float>(bg_color.blueF());
    }
}

void ConfigureEnhancements::Setup(ConfigurationShared::Builder& builder) {
    const std::vector<Settings::Category> categories = {
        Settings::Category::RendererEnhancements, Settings::Category::Utility,
        Settings::Category::RendererStereo, Settings::Category::Layout};

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
        case Settings::Category::RendererEnhancements:
            ui->rendererBox->layout()->addWidget(widget);
            break;
        case Settings::Category::Utility:
            ui->utilityBox->layout()->addWidget(widget);
            break;
        case Settings::Category::RendererStereo:
            ui->stereo_group->layout()->addWidget(widget);
            break;
        case Settings::Category::Layout:
            ui->layout_group->layout()->addWidget(widget);
            break;
        default:
            widget->deleteLater();
            continue;
        }

        if (setting->Id() == Settings::values.pp_shader_name.Id()) {
            shader_combobox = widget->combobox;
            UpdateShaders(Settings::values.render_3d.GetValue());
        } else if (setting->Id() == Settings::values.render_3d.Id()) {
            render_3d_combobox = widget->combobox;
        } else if (setting->Id() == Settings::values.resolution_factor.Id()) {
            resolution_factor_combobox = widget->combobox;
        } else if (setting->Id() == Settings::values.async_custom_loading.Id()) {
            toggle_async_custom_loading = widget->checkbox;
        } else if (setting->Id() == Settings::values.preload_textures.Id()) {
            toggle_preload_textures = widget->checkbox;
        } else if (setting->Id() == Settings::values.custom_textures.Id()) {
            toggle_custom_textures = widget->checkbox;
        }
    }

    if (!Settings::IsConfiguringGlobal()) {
        ui->bg_color_group->setVisible(false);
    }
}
