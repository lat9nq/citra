// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <string_view>
#include <utility>
#include "common/file_util.h"
#include "common/settings.h"
#include "core/hle/service/cam/cam.h"

namespace Settings {

Values values = {};
static bool configuring_global = true;

const char* TranslateCategory(Category category) {
    switch (category) {
    case Category::Audio:
        return "Audio";
    case Category::Core:
        return "Core";
    case Category::Layout:
        return "Layout";
    case Category::Renderer:
    case Category::RendererAdvanced:
    case Category::RendererDebug:
    case Category::RendererEnhancements:
    case Category::RendererApi:
    case Category::RendererStereo:
    case Category::RendererOther:
        return "Renderer";
    case Category::System:
        return "System";
    case Category::Utility:
        return "Utility";
    case Category::DataStorage:
        return "Data Storage";
    case Category::Debugging:
    case Category::DebuggingGraphics:
        return "Debugging";
    case Category::Miscellaneous:
        return "Miscellaneous";
    case Category::VideoDumping:
        return "VideoDumping";
    case Category::WebService:
        return "WebService";
    case Category::Controls:
        return "Controls";
    case Category::Ui:
    case Category::UiGeneral:
        return "UI";
    case Category::UiLayout:
        return "UILayout";
    case Category::UiGameList:
        return "GameList";
    case Category::UiUpdater:
        return "Updater";
    case Category::Shortcuts:
        return "Shortcuts";
    case Category::Multiplayer:
        return "Multiplayer";
    case Category::Services:
        return "Services";
    case Category::Screenshots:
    case Category::Paths:
        return "Paths";
    case Category::MaxEnum:
        break;
    }
    return "Miscellaneous";
}

void LogSettings() {
    const auto log_setting = [](std::string_view name, const auto& value) {
        LOG_INFO(Config, "{}: {}", name, value);
    };

    LOG_INFO(Config, "Citra Configuration:");

    for (const auto& [category, settings] : values.linkage.by_category) {
        const char* const category_str = TranslateCategory(category);
        for (const auto* setting : settings) {
            const char modified = setting->ToString() == setting->DefaultToString() ? '-' : 'M';
            const char custom = setting->UsingGlobal() ? '-' : 'C';
            LOG_INFO(Config, "{}{} {}.{}: {}", modified, custom, category_str, setting->GetLabel(),
                     setting->Canonicalize());
        }
    }

    using namespace Service::CAM;
    log_setting("Camera_OuterRightName", values.camera_name[OuterRightCamera]);
    log_setting("Camera_OuterRightConfig", values.camera_config[OuterRightCamera]);
    log_setting("Camera_OuterRightFlip", values.camera_flip[OuterRightCamera]);
    log_setting("Camera_InnerName", values.camera_name[InnerCamera]);
    log_setting("Camera_InnerConfig", values.camera_config[InnerCamera]);
    log_setting("Camera_InnerFlip", values.camera_flip[InnerCamera]);
    log_setting("Camera_OuterLeftName", values.camera_name[OuterLeftCamera]);
    log_setting("Camera_OuterLeftConfig", values.camera_config[OuterLeftCamera]);
    log_setting("Camera_OuterLeftFlip", values.camera_flip[OuterLeftCamera]);
    if (values.use_custom_storage) {
        log_setting("DataStorage_SdmcDir", FileUtil::GetUserPath(FileUtil::UserPath::SDMCDir));
        log_setting("DataStorage_NandDir", FileUtil::GetUserPath(FileUtil::UserPath::NANDDir));
    }
}

bool IsConfiguringGlobal() {
    return configuring_global;
}

void SetConfiguringGlobal(bool is_global) {
    configuring_global = is_global;
}

float Volume() {
    if (values.audio_muted) {
        return 0.0f;
    }
    return values.volume.GetValue();
}

void RestoreGlobalState(bool is_powered_on) {
    // If a game is running, DO NOT restore the global settings state
    if (is_powered_on) {
        return;
    }

    for (const auto& func : values.linkage.restore_functions) {
        func();
    }
}

void LoadProfile(int index) {
    Settings::values.current_input_profile = Settings::values.input_profiles[index];
    Settings::values.current_input_profile_index = index;
}

void SaveProfile(int index) {
    Settings::values.input_profiles[index] = Settings::values.current_input_profile;
}

void CreateProfile(std::string name) {
    Settings::InputProfile profile = values.current_input_profile;
    profile.name = std::move(name);
    Settings::values.input_profiles.push_back(std::move(profile));
    Settings::values.current_input_profile_index =
        static_cast<int>(Settings::values.input_profiles.size()) - 1;
    Settings::LoadProfile(Settings::values.current_input_profile_index);
}

void DeleteProfile(int index) {
    Settings::values.input_profiles.erase(Settings::values.input_profiles.begin() + index);
    Settings::LoadProfile(0);
}

void RenameCurrentProfile(std::string new_name) {
    Settings::values.current_input_profile.name = std::move(new_name);
}

} // namespace Settings
